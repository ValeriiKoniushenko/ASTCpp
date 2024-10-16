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

#include "Ast/LogCollector.h"
#include "Ast/Readers/ContentStream.h"
#include "Ast/Utils/Scopes.h"
#include "Ast/Utils/String.h"
#include "AstCpp/TemplateLexer/CheckForTemplateLexer.h"

namespace Ast::Cpp
{

    ClassLexer::ClassLexer(const ContentStream::Ptr& fileReader)
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
            logCollector.AddLog({ String::Format("Impossible to parse the class token at {}", _token.startLine), LogCollector::LogType::Error });
            return false;
        }

        if (string.RegexReplace("\\sfinal", ""))
        {
            _hasFinal = true;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define a class name"))
        {
            _lexerName = match.str();
            _lexerName.ShrinkToFit();
        }
        else
        {
            logCollector.AddLog({ "Impossible to parse class token at {line}" });
            return false;
        }

        if (string.RegexReplace(R"(^\w+\s*:)", ""))
        {
            std::vector<String> parents;
            int bracketsCount = 0;
            String tmp;
            for (int i = 0; i < string.Size(); ++i)
            {
                if (string[i] == '<')
                {
                    ++bracketsCount;
                }
                else if (string[i] == '>')
                {
                    --bracketsCount;
                }
                tmp.PushBack(string[i]);

                if (bracketsCount == 0)
                {
                    if (string[i] == ',')
                    {
                        tmp.Trim(',').Trim(' ');
                        parents.push_back(std::move(tmp));
                    }
                }
            }
            parents.push_back(std::move(tmp));

            for (auto&& parentStr : parents)
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
            logCollector.AddLog({ String::Format("Impossible to define an class scope '{}'", _lexerName.c_str()), LogCollector::LogType::Error });
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data(), closedBracket) };

        return true;
    }

    bool ClassLexer::DoMarkingValidate(LogCollector& logCollector)
    {
        if (!BaseLexer::DoMarkingValidate(logCollector))
        {
            return false;
        }

        const auto* begin = Ast::Cpp::TryToFindTemplateBegin(this);
        if (begin == nullptr)
        {
            begin = _token.beginData;
        }
        if (!Verify(begin))
        {
            return false;
        }

        --begin;

        while (String::IsSpace(*begin))
        {
            --begin;
        }

        // Corresponding to AstCpp/Markers.h -> #define CLASS
        if ((begin = Ast::Utils::SkipBracketsR(this, begin, '(', ')')))
        {
            while (String::IsSpace(*begin))
            {
                --begin;
            }
            const auto marker = "CLASS"_atom;
            begin -= marker.Size();
            if (begin >= _reader->Data().c_str())
            {
                if (String(begin, marker.Size()).RegexMatch(marker))
                {
                    Marker marker;

                    while (*begin != '(')
                    {
                        marker.rule.push_back(*begin);
                        ++begin;
                    }

                    const auto* end = Utils::FindClosedBracket(begin, ')', '(');
                    for (auto param : String(begin, end - begin).Split(","))
                    {
                        param.Trim(' ').Trim('(').Trim(')');
                        marker.params.push_back(std::move(param));
                    }

                    _marking = std::move(marker);
                }
            }
        }

        return true;
    }

    bool ClassLexer::DoPostValidate(LogCollector& logCollector)
    {
        if (!BaseLexer::DoPostValidate(logCollector))
        {
            return false;
        }

        TryToFindTemplate(logCollector);

        RecognizeFields(logCollector);

        return true;
    }

    void ClassLexer::TryToFindTemplate(LogCollector& logCollector)
    {
        auto* begin = _token.beginData;

        if (auto string = Cpp::TryToExtrudeTemplate(this))
        {
            _isTemplate = true;

            string.Trim('<').Trim('>');
            int bracketsCount = 0;
            String tmp;
            for (int i = 0; i < string.Size(); ++i)
            {
                if (string[i] == '<')
                {
                    ++bracketsCount;
                }
                else if (string[i] == '>')
                {
                    --bracketsCount;
                }
                tmp.PushBack(string[i]);

                if (bracketsCount == 0)
                {
                    if (string[i] == ',')
                    {
                        tmp.Trim(',').Trim(' ');
                        _templateUnits.push_back({ std::move(tmp) });
                    }
                }
            }
            _templateUnits.push_back({ std::move(tmp) });
        }
    }

    void ClassLexer::RecognizeFields(LogCollector& logCollector)
    {
        String body(_openScope->string, _closeScope->string - _openScope->string);
        body.Trim('{').Trim('}');

        RemoveNestedScopes(body);

        const auto publics = body.FindRegex(R"(^\s*public\s*\:)");
        const auto protecteds = body.FindRegex(R"(^\s*protected\s*\:)");
        const auto privates = body.FindRegex(R"(^\s*private\s*\:)");

        body.IterateRegex(R"(^\s*((static\s+)|(constexpr\s+)|(const\s+)|(constinit\s+))*[\w:]+(\<.*\>)?\s+\w+(((\s*=).*)|(;)))",
                          [&](const String::StdRegexMatchResults& field)
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
                                  logCollector.AddLog({ String::Format("Impossible to define a class's field type. Class: '{}'", _lexerName.c_str()),
                                                        LogCollector::LogType::Error });
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
                                  logCollector.AddLog({ String::Format("Impossible to define a class's field name. Class: '{}'", _lexerName.c_str()),
                                                        LogCollector::LogType::Error });
                                  return true;
                              }

                              long long minDistance = std::numeric_limits<long long>::max();
                              AccessSpecifier accessSpecifier = AccessSpecifier::Private;
                              for (auto&& token : publics)
                              {
                                  const auto distance = std::distance(token.first, field.begin()->first);
                                  if (distance >= 0 && distance < minDistance)
                                  {
                                      minDistance = distance;
                                      accessSpecifier = AccessSpecifier::Public;
                                  }
                              }
                              for (auto&& token : protecteds)
                              {
                                  const auto distance = std::distance(token.first, field.begin()->first);
                                  if (distance >= 0 && distance < minDistance)
                                  {
                                      minDistance = distance;
                                      accessSpecifier = AccessSpecifier::Protected;
                                  }
                              }
                              for (auto&& token : privates)
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