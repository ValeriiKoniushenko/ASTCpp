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

    String ClassLexer::Field::GetTextSource()
    {
        String source;
        if (isInline)
        {
            source += "inline ";
        }
        if (isStatic)
        {
            source += "static ";
        }
        if (isConstinit)
        {
            source += "constinit ";
        }
        else if (isConstexpr)
        {
            source += "constexpr ";
        }
        else if (isConst)
        {
            source += "const ";
        }

        source += type;
        source += " "_atom;
        source += name;
        source += " = ";
        source += value.IsEmpty() ? "{}" : value;
        source += ";";

        return source;
    }

    String ClassLexer::Method::GetTextSource()
    {
        return comment + Code::Endl() + header + Code::Endl() + "{" + Code::Endl() + body + Code::Endl() + "}";
    }

    bool ClassLexer::AddTemplate(TemplateUnit template_)
    {
        if (!Verify(IsValid()))
        {
            return false;
        }

        _templateUnits.push_back(std::move(template_));

        return true;
    }

    bool ClassLexer::AddClassParents(ParentUnit parent)
    {
        if (!Verify(IsValid()))
        {
            return false;
        }

        _parents.push_back(std::move(parent));

        return true;
    }

    bool ClassLexer::AddField(Field field)
    {
        if (!Verify(IsValid()))
        {
            return false;
        }

        _fields.push_back(std::move(field));

        return true;
    }

    bool ClassLexer::AddMethod(Method method)
    {
        if (!Verify(IsValid()))
        {
            return false;
        }

        _methods.push_back(std::move(method));

        return true;
    }

    bool ClassLexer::GenerateTextSource(TextSourceT& source)
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

        // =========== template ==============
        String classSource;
        if (_isTemplate)
        {
            String tmp;
            for (const auto& unit : _templateUnits)
            {
                tmp += unit.expression;
                tmp += ", ";
            }
            tmp.TrimEnd(' ').TrimEnd(',');
            classSource += "template<" + tmp + ">" + Code::Endl();
        }

        // =========== class head ==============
        classSource += "class " + _lexerName;
        if (_hasFinal)
        {
            classSource += " final";
        }

        // parents
        if (!_parents.empty())
        {
            classSource += " : " + Code::Endl();
            for (auto& p : _parents)
            {
                classSource += Code::Tab(3) + p.GetTextSource() + "," + Code::Endl();
            }
            classSource.TrimEnd('\n').TrimEnd(',');
        }

        // =========== class body ==============
        classSource += Code::Endl();
        classSource += "{" + Code::Endl();

        // public
        classSource += "public:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Public,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.Size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });

        // protected
        classSource += Code::Endl();
        classSource += "protected:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Protected,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.Size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });

        // private
        classSource += Code::Endl();
        classSource += "private:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Private,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.Size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });


        classSource += "};" + Code::Endl();

        source.source.Insert(pos, classSource.c_str());
        source.carets["write-point"_atom] = source.source.Size();

        return true;
    }

    ClassLexer::ClassLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool ClassLexer::DoParse(LogCollector& logCollector)
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

        if (string.RegexReplace("\\final", ""))
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
            logCollector.AddLog({ "Impossible to parse class token at {}"_f << _token.startLine, LogCollector::LogType::Warning });
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

                ParentUnit parent;
                parent.name = std::move(parentStr);
                parent.type = type;
                _parents.push_back(std::move(parent));
            }
        }

        return true;
    }

    bool ClassLexer::DoScopeParse(LogCollector& logCollector)
    {
        if (!BaseLexer::DoScopeParse(logCollector))
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

    bool ClassLexer::DoMarkingParse(LogCollector& logCollector)
    {
        if (!BaseLexer::DoMarkingParse(logCollector))
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

        auto limits = GetReaderLimits();

        while (begin > limits.first && (String::IsSpace(*begin) || *begin == ';'))
        {
            --begin;
        }

        // Corresponding to AstCpp/Markers.h -> #define CLASS
        if (begin = Ast::Utils::SkipBracketsR(this, begin, '(', ')', limits.first); begin && begin > limits.first)
        {
            while (String::IsSpace(*begin))
            {
                --begin;
            }

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

    bool ClassLexer::DoPostParse(LogCollector& logCollector)
    {
        if (!BaseLexer::DoPostParse(logCollector))
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
                        TemplateUnit templateUnit;
                        templateUnit.expression = std::move(tmp);
                        _templateUnits.push_back(std::move(templateUnit));
                    }
                }
            }
            TemplateUnit templateUnit;
            templateUnit.expression = std::move(tmp);
            _templateUnits.push_back(std::move(templateUnit));
        }
    }

    void ClassLexer::RecognizeFields(LogCollector& logCollector)
    {
        String body(_openScope->string, _closeScope->string - _openScope->string);
        body.Trim('{').Trim('}');

        RemoveNestedScopes(body);

        const auto publics = body.FindRegex(R"(^\s*public\s*\:)", 0, std::regex_constants::match_default);
        const auto protecteds = body.FindRegex(R"(^\s*protected\s*\:)", 0, std::regex_constants::match_default);
        const auto privates = body.FindRegex(R"(^\s*private\s*\:)", 0, std::regex_constants::match_default);

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
                                  str.RegexReplace(R"(static\s+)", "", std::regex_constants::format_first_only);
                              }
                              if (auto match = str.FindRegex(R"(const\s+)"); !match.empty())
                              {
                                  tempField.isConst = true;
                                  str.RegexReplace(R"(const\s+)", "", std::regex_constants::format_first_only);
                              }
                              if (auto match = str.FindRegex(R"(constexpr\s+)"); !match.empty())
                              {
                                  tempField.isConstexpr = true;
                                  str.RegexReplace(R"(constexpr\s+)", "", std::regex_constants::format_first_only);
                              }
                              if (auto match = str.FindRegex(R"(constinit\s+)"); !match.empty())
                              {
                                  tempField.isConstinit = true;
                                  str.RegexReplace(R"(constinit\s+)", "", std::regex_constants::format_first_only);
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

                              long long minDistance = (std::numeric_limits<long long>::max)();
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
                          }, 0, std::regex_constants::match_default);
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

    void ClassLexer::IterateOverChilds(AccessSpecifier accessSpecifier, std::function<void(ITextSourceReader&)>&& callback)
    {
        if (!callback)
        {
            return;
        }

        std::vector<ITextSourceReader*> children;
        for (auto& unit : _fields)
        {
            children.push_back(&unit);
        }

        for (auto& unit : _childLexers)
        {
            children.push_back(unit.get());
        }

        for (auto* unit : children)
        {
            callback(*unit);
        }
    }

} // namespace Ast::Cpp
