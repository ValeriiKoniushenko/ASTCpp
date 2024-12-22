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

#include "EnumClassLexer.h"

#include "Ast/LogCollector.h"
#include "Ast/Readers/ContentStream.h"
#include "Ast/Utils/Scopes.h"

namespace Ast::Cpp
{

    String EnumClassLexer::Constant::GetTextSource()
    {
        String source = name;
        if (value)
        {
            source += " = " + String::MakeFrom(value.value());
        }

        return source;
    }

    EnumClassLexer::Constant EnumClassLexer::GetConstant(const String& name) const
    {
        auto found = std::find_if(_constants.cbegin(), _constants.cend(), [&name](const Constant& a)
        {
            return a.name == name;
        });

        if (found != _constants.cend())
        {
            return *found;
        }

        return {};
    }

    EnumClassLexer::Constant EnumClassLexer::GetConstant(unsigned long long value) const
    {
        auto found = std::find_if(_constants.cbegin(), _constants.cend(), [value](const Constant& a)
        {
            return a.value.value_or(~0ull) == value;
        });

        if (found != _constants.cend())
        {
            return *found;
        }

        return {};
    }

    bool EnumClassLexer::AddConstant(const String& name, std::optional<unsigned long long> value)
    {
        if (!value.has_value())
        {
            auto max = std::max_element(_constants.cbegin(), _constants.cend(), [](const Constant& a, const Constant& b)
            {
                return a.value.value_or(0) < b.value.value_or(0);
            });

            if (max != _constants.cend())
            {
                value = max->value.value_or(0) + 1;
            }
            else
            {
                value = 0;
            }
        }

        if (!Verify(std::find_if(_constants.cbegin(), _constants.cend(), [&value](const Constant& a)
            {
                return a.value.has_value() ? a.value.value_or(0) == value.value() : false;
            }) == _constants.cend(), "Impossible to add new enum class constant, because such value already exists"))
        {
            return false;
        }

        if (!Verify(std::find_if(_constants.cbegin(), _constants.cend(), [&name](const Constant& a)
            {
                return a.name == name;
            }) == _constants.cend(), "Impossible to add new enum class constant, because such name of the constant already exists"))
        {
            return false;
        }

        _constants.emplace_back(Constant{name, std::move(value) });
        return true;
    }

    bool EnumClassLexer::GenerateTextSource(TextSourceT& source)
    {
        if (!ITextSourceReader::GenerateTextSource(source))
        {
            return false;
        }

        uint32_t pos = 0;

        if (auto i = source.carets.find("write-point"_atom); i != source.carets.end())
        {
            pos = i->second;
        }

        String enumSource = "enum class " + GetLexerName() + " : " + _type + Code::Endl() + "{" + Code::Endl();
        for (auto& constant : _constants)
        {
            enumSource += Code::Tab() + constant.GetTextSource() + "," + Code::Endl();
        }
        enumSource.TrimEnd('\r').TrimEnd('\n').TrimEnd('\r').TrimEnd(',');

        enumSource += Code::Endl() + "};" + Code::Endl();

        source.source.Insert(pos, enumSource.c_str());
        source.carets["write-point"_atom] = source.source.Size();

        return true;
    }

    EnumClassLexer::EnumClassLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool EnumClassLexer::DoValidate(LogCollector& logCollector)
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            logCollector.AddLog({ "EnumClassLexer: Impossible to work with an invalid token", LogCollector::LogType::Error });
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(enum class)|\{)", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({ String::Format("Impossible to parse enum class token at {}", _token.startLine), LogCollector::LogType::Error });
            return false;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define an enum class name"))
        {
            _lexerName = match.str();
            _lexerName.ShrinkToFit();
        }
        else
        {
            logCollector.AddLog({ String::Format("Impossible to parse enum class token at {}", _token.startLine), LogCollector::LogType::Error });
            return false;
        }

        if (string.RegexReplace(R"(^\w+\s*:)", ""))
        {
            _type = string.Trim(' ');
        }

        return true;
    }

    bool EnumClassLexer::DoValidateScope(LogCollector& logCollector)
    {
        if (!BaseLexer::DoValidateScope(logCollector))
        {
            return false;
        }

        const auto* openedBracket = _token.endData;
        while (String::Toolset::IsSpace(*openedBracket))
        {
            ++openedBracket;
        }
        if (!Verify(*openedBracket == '{', "Impossible to define an enum class scope."))
        {
            logCollector.AddLog(
                { String::Format("Impossible to define an enum class scope '{}'", _lexerName.c_str()), LogCollector::LogType::Error });
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data(), closedBracket) };

        if (!RecognizeConstants(logCollector))
        {
            return false;
        }

        return true;
    }

    bool EnumClassLexer::RecognizeConstants(LogCollector& logCollector)
    {
        if (!Verify(_openScope.has_value() && _openScope->IsValid() && _closeScope.has_value() && _closeScope->IsValid()))
        {
            logCollector.AddLog({ String::Format("Impossible to get an enum class scope '{}'", _lexerName.c_str()), LogCollector::LogType::Error });
            return false;
        }

        String buffer(_openScope->string, _closeScope->string - _openScope->string);
        buffer.Trim('{').Trim('}').RegexReplace(R"(\s)", "");
        for (auto& constant : buffer.Split(","_atom))
        {
            if (auto match = constant.FindRegex(R"(^\w+)"); !match.empty())
            {
                _constants.emplace_back(String(match.str()), std::nullopt);
                _constants.back().name.ShrinkToFit();
            }
        }

        return true;
    }

} // namespace Ast::Cpp