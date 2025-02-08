// Copyright (c) 2024 Valerii Koniushenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "Ast/ProjectTree.h"
#include "AstCpp/Generators/GeneratorUnit.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <fstream>

namespace Ast::Cpp
{

    template<bool IsConst>
    class BaseUnitBridge : public Utils::CopyableAndMoveable
    {
    public:
        using Unit = Ast::ProjectTree::Unit;
        using UnitPtr = Ast::ProjectTree::Unit::AdaptiveRawPtr<IsConst>;

    public:
        explicit BaseUnitBridge(UnitPtr unit)
            : _unit(unit)
        {
        }

        [[nodiscard]] bool IsValid() const { return _unit; }

        [[nodiscard]] std::filesystem::path GetGeneratedImplFilePath() const
        {
            if (!Verify(IsValid()))
            {
                return {};
            }

            auto path = _unit->GetGeneratedSiblingFilePath();
            const auto mainExt = path.extension();
            path.replace_extension("");
            if (!path.extension().empty() && path.extension().string() == ProjectTree::Unit::generatedSuffixDecl)
            {
                String finalExt = AbstractGeneratorUnit<>::generatedSuffixImpl;
                finalExt += ProjectTree::Unit::generatedSuffixDecl;
                finalExt += mainExt.string();

                path.replace_extension(finalExt.c_str());
                return path;
            }

            return {};
        }

        void TryToAddNeededIncludes() const
        {
            if (!Verify(IsValid()))
            {
                return;
            }

            const bool hasDeclInclude = HasDeclInclude();
            const bool hasImplInclude = HasImplInclude();

            if (!hasDeclInclude || !hasImplInclude)
            {
                const auto& stream = _unit->GetFileContentStream();
                if (!Verify(!!stream))
                {
                    return;
                }

                const auto& data = stream->Data();
                if (!Verify(!data.IsEmpty()))
                {
                    return;
                }

                std::vector<std::pair<uint64_t, String>> shouldBeInserted;

                if (!hasDeclInclude)
                {
                    const auto line = GetInsertLineOfDeclInclude(data);
                    const auto path = GetGeneratedDeclFilePath();
                    if (Verify(!path.empty() && path.has_filename()))
                    {
                        auto include = R"({}// Next include must be below of all your includes{}#include "{}")"_f << Unit::Code::Endl() << Unit::Code::Endl() << String::MakeFrom(path.filename());
                        shouldBeInserted.emplace_back(line, std::move(include));
                    }
                }

                if (!hasImplInclude)
                {
                    const auto line = GetInsertLineOfImplInclude(data);
                    const auto path = GetGeneratedImplFilePath();
                    if (Verify(!path.empty() && path.has_filename()))
                    {
                        auto include = R"(// Next include must be in the end of this file{}#include "{}")"_f << Unit::Code::Endl() << String::MakeFrom(path.filename());
                        shouldBeInserted.emplace_back(line, std::move(include));
                    }
                }

                const auto linesInFile = String::GetLinesCountInText(data);

                InsertAtLineOfFile(_unit->GetPath(), std::move(shouldBeInserted));
            }
        }

        [[nodiscard]] std::filesystem::path GetGeneratedDeclFilePath() const
        {
            if (!Verify(IsValid()))
            {
                return {};
            }

            return _unit->GetGeneratedSiblingFilePath();
        }

    protected:
        [[nodiscard]] bool HasDeclInclude() const
        {
            if (!Verify(IsValid()))
            {
                return false;
            }

            auto stream = _unit->GetFileContentStream();
            if (!Verify(!!stream))
            {
                return false;
            }

            const auto& data = stream->Data();
            if (!Verify(!data.IsEmpty()))
            {
                return false;
            }

            const String declExpr = R"(^\s*#include.*{}{}{})"_f << _unit->GetPath().stem().string() << Unit::generatedSuffixDecl
                                                                << _unit->GetPath().extension().string();

            return !data.FindRegex(declExpr).empty();
        }

        [[nodiscard]] bool HasImplInclude() const
        {
            if (!Verify(IsValid()))
            {
                return false;
            }

            auto stream = _unit->GetFileContentStream();
            if (!Verify(!!stream))
            {
                return false;
            }

            const auto& data = stream->Data();
            if (!Verify(!data.IsEmpty()))
            {
                return false;
            }

            const String implExpr = R"(^\s*#include\s*"{}{}{}{}")"_f << _unit->GetPath().stem().string()
                                                                     << AbstractGeneratorUnit<>::generatedSuffixImpl << Unit::generatedSuffixDecl
                                                                     << _unit->GetPath().extension().string();

            return !data.FindRegex(implExpr).empty();
        }

    protected:
        UnitPtr _unit;

    private:
        [[nodiscard]] uint64_t GetInsertLineOfDeclInclude(const String& data) const
        {
            if (data.IsEmpty())
            {
                return ~0ull;
            }
            const auto* begin = data.c_str();
            const auto* end = data.c_str() + data.Size();

            uint64_t validLine = 0;

            uint64_t line = 0;
            const auto* i = begin;

            while (i && i < end)
            {
                // just skip a blank line
                if (std::regex_match(i, end, std::regex(R"(^\s*$)")))
                {
                    ++line;
                    i = String::FindNextLine(i);
                    continue;
                }

                // trying to find #include
                if (std::regex_match(i, end, std::regex(R"(^\s#include)")))
                {
                    validLine = line + 1;
                }
                else
                {
                    break;
                }

                i = String::FindNextLine(i);
                if (i)
                {
                    ++line;
                }
            }

            return validLine;
        }

        [[nodiscard]] uint64_t GetInsertLineOfImplInclude(const String& data) const
        {
            if (data.IsEmpty())
            {
                return ~0ull;
            }

            const auto* begin = data.c_str();
            const auto* end = data.c_str() + data.Size();

            const auto* i = String::FindPrevLine(begin);
            if (std::regex_match(i, end, std::regex(R"(^\s*$)")))
            {
                return String::GetLinesCountInText(data) - 1;
            }

            return ~0ull;
        }


        void InsertAtLineOfFile(const std::filesystem::path& path, std::vector<std::pair<uint64_t, String>> data) const
        {
            std::ifstream readFile(path.string());
            if (!readFile.is_open())
            {
                Assert();
                return;
            }

            std::vector<std::string> lines;
            std::string str;
            while (std::getline(readFile, str))
            {
                lines.push_back(std::move(str));
            }
            readFile.close();

            uint64_t offset = 0;
            for (const auto& [line, str] : data)
            {
                if (line >= lines.size())
                {
                    Assert("Invalid line number. File doesn't have such line. File: {}"_f << path.string());
                    continue;
                }

                lines.insert(lines.begin() + line + offset++, str.ToStdString());
            }

            std::ofstream writeFile(path.string());
            if (!writeFile.is_open())
            {
                Assert();
                return;
            }

            for (const auto& line : lines)
            {
                writeFile << line;
            }
        }
    };

    using UnitBridge = BaseUnitBridge<false>;
    using ConstUnitBridge = const BaseUnitBridge<true>;

} // namespace Ast::Cpp
