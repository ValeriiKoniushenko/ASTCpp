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

#include "ClassLexer.h"

#include "AST/LogCollector.h"
#include "AST/Readers/FileReader.h"
#include "AST/Utils/Scopes.h"

namespace Ast::Cpp
{

    ClassLexer::ClassLexer(const FileReader& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool ClassLexer::DoValidate(LogCollector& logCollector)
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            logCollector.AddLog({ "ClassLexer: Impossible to work with an invalid token", LogCollector::LogType::Error });
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(class)|\{)", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({ String::Format("Impossible to parse the class token at {}", 999), LogCollector::LogType::Error });
            return false;
        }

        if (string.RegexReplace("\\sfinal", ""))
        {
            _hasFinal = true;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define a class name"))
        {
            _name = match.str();
            _name.ShrinkToFit();
        }
        else
        {
            logCollector.AddLog({ "Impossible to parse class token at {line}" });
            return false;
        }

        if (string.RegexReplace(R"(^\w+\s*:)", ""))
        {
            for (auto& parentStr : string.Split(","_atom))
            {
                InheritanceType type = InheritanceType::Private;
                if (parentStr.RegexReplace(R"(\s*public\s*)", ""))
                {
                    type = InheritanceType::Public;
                }
                else if (parentStr.RegexReplace(R"(\s*protected\s*)", ""))
                {
                    type = InheritanceType::Protected;
                }
                else
                {
                    parentStr.RegexReplace(R"(\s*private\s*)", "");
                    type = InheritanceType::Private;
                }

                parentStr.Trim(' ');
                parentStr.ShrinkToFit();
                _parents.emplace_back(type, std::move(parentStr));
            }
        }

        return true;
    }

    bool ClassLexer::DoValidateScope(LogCollector& logCollector)
    {
        if (!BaseLexer::DoValidateScope(logCollector))
        {
            return false;
        }

        const auto* openedBracket = _token.endData - 1; // -1 - to back to the '{' correspoinding to regex expr
        if (!Verify(*openedBracket == '{', "Impossible to define an class scope."))
        {
            logCollector.AddLog({ String::Format("Impossible to define an class scope '{}'", _name.c_str()), LogCollector::LogType::Error });
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data(), closedBracket) };

        return true;
    }
    bool ClassLexer::DoPostValidate(LogCollector& logCollector)
    {
        if (!BaseLexer::DoPostValidate(logCollector))
        {
            return false;
        }

        RecognizeFields(logCollector);

        return true;
    }

    void ClassLexer::RecognizeFields(LogCollector& logCollector)
    {
        String body(_openScope->string, _closeScope->string - _openScope->string);
        body.Trim('{').Trim('}');

        RemoveNestedScopes(body);

        const auto publics = body.FindRegex(R"(^\s*public\s*\:)");
        const auto protecteds = body.FindRegex(R"(^\s*protected\s*\:)");
        const auto privates = body.FindRegex(R"(^\s*private\s*\:)");

        body.IterateRegex(R"(^\s*((static\s+)|(constexpr\s+)|(const\s+)|(constinit\s+))*[\w:]+(\<.*\>)?\s+\w+(((\s*=).*)|(;)))",[&](const String::StdRegexMatchResults& field)
        {
            auto str = String(field.str());
            str.RegexReplace(R"([\s;]*$)", "");
            str.RegexReplace(R"(^\s*)", "");

            Field tempField;

            if (auto match = str.FindRegex(R"(static\s+)"); !match.empty())
            {
                tempField.isStatic = true;
                str.RegexReplace(R"(static\s+)", "", std::regex_constants::match_flag_type::format_first_only);
            }
            if (auto match = str.FindRegex(R"(const\s+)"); !match.empty())
            {
                tempField.isConst = true;
                str.RegexReplace(R"(const\s+)", "", std::regex_constants::match_flag_type::format_first_only);
            }
            if (auto match = str.FindRegex(R"(constexpr\s+)"); !match.empty())
            {
                tempField.isConstexpr = true;
                str.RegexReplace(R"(constexpr\s+)", "", std::regex_constants::match_flag_type::format_first_only);
            }
            if (auto match = str.FindRegex(R"(constinit\s+)"); !match.empty())
            {
                tempField.isConstinit = true;
                str.RegexReplace(R"(constinit\s+)", "", std::regex_constants::match_flag_type::format_first_only);
            }

            if (auto matchType = str.FindRegex(R"(^[\w:]+(\<.*\>)?)"); Verify(!matchType.empty()))
            {
                tempField.type = matchType.str();
                tempField.type.ShrinkToFit();
                str.RegexReplace(R"(^[\w:]+(\<.*\>)?)", "");
                str.TrimStart(' ');
            }
            else
            {
                logCollector.AddLog({ String::Format("Impossible to define a class's field type. Class: '{}'", _name.c_str()), LogCollector::LogType::Error });
                return true;
            }

            if (auto matchName = str.FindRegex(R"(^\w+)"); Verify(!matchName.empty()))
            {
                tempField.name = matchName.str();
                tempField.name.ShrinkToFit();
                str.RegexReplace(R"(^\w+)", "");
                str.TrimStart(' ');
            }
            else
            {
                logCollector.AddLog({ String::Format("Impossible to define a class's field name. Class: '{}'", _name.c_str()), LogCollector::LogType::Error });
                return true;
            }

            long long minDistance = std::numeric_limits<long long>::max();
            AccessSpecifier accessSpecifier = AccessSpecifier::Private;
            for(auto&& token : publics)
            {
                const auto distance = std::distance(token.first, field.begin()->first);
                if (distance >= 0 && distance < minDistance)
                {
                    minDistance = distance;
                    accessSpecifier = AccessSpecifier::Public;
                }
            }
            for(auto&& token : protecteds)
            {
                const auto distance = std::distance(token.first, field.begin()->first);
                if (distance >= 0 && distance < minDistance)
                {
                    minDistance = distance;
                    accessSpecifier = AccessSpecifier::Protected;
                }
            }
            for(auto&& token : privates)
            {
                const auto distance = std::distance(token.first, field.begin()->first);
                if (distance >= 0 && distance < minDistance)
                {
                    minDistance = distance;
                    accessSpecifier = AccessSpecifier::Private;
                }
            }
            tempField.accessSpecifier = accessSpecifier;

            _fields.push_back(std::move(tempField));

            return true;
        });
    }

    void ClassLexer::RemoveNestedScopes(String& body)
    {
        const String::CharT* opened = nullptr;
        while ((opened = Utils::FindFirstBracket(body.c_str(), '{')))
        {
            if (const auto* closed = Utils::FindClosedBracket(opened, '}', '{'))
            {
                body.Erase(opened - body.c_str(), closed - body.c_str());
            }
        }
    }

} // namespace Ast::Cpp