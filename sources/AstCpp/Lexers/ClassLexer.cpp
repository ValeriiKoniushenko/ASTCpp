//  MIT License
//
//  Copyright (c) 2019-2025 Valerii Koniushenko
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include "ClassLexer.h"

#include "Ast/Readers/ContentStream.h"
#include "Ast/Utils/Scopes.h"
#include "Ast/Utils/String.h"
#include "AstCpp/Utils/CheckForTemplateLexer.h"
#include "spdlog/spdlog.h"

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
        source += value.isEmpty() ? "{}" : value;
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
            tmp.trimEnd(' ').trimEnd(',');
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
            classSource.trimEnd('\n').trimEnd(',');
        }

        // =========== class body ==============
        classSource += Code::Endl();
        classSource += "{" + Code::Endl();

        // public
        classSource += "public:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Public,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });

        // protected
        classSource += Code::Endl();
        classSource += "protected:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Protected,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });

        // private
        classSource += Code::Endl();
        classSource += "private:" + Code::Endl();
        IterateOverChilds(AccessSpecifier::Private,
                          [&classSource, &source](ITextSourceReader& unit)
                          {
                              source.carets["write-point"_atom] = source.source.size();
                              classSource += Code::Tab() + unit.GetTextSource() + Code::Endl();
                          });

        classSource += "};" + Code::Endl();

        source.source.insert(pos, classSource.c_str());
        source.carets["write-point"_atom] = source.source.size();

        return true;
    }

    ClassLexer::ClassLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool ClassLexer::DoParse()
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            spdlog::error("ClassLexer: Impossible to work with an invalid token");
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.regexReplaceAll(R"(\n|\r|(class)|\{)", "", 1, 0, 0, PCRE2_MULTILINE);
        string.trim(' ');
        string.trimEnd('{');

        if (string.isEmpty())
        {
            spdlog::error(("Impossible to parse the class token at {}"_f << _token.startLine).toStdStringView());
            return false;
        }

        if (string.regexReplace("\\final", "", 1))
        {
            _hasFinal = true;
        }

        auto match = string.regexFind("^\\w+");
        if (Verify(!!match, "Impossible to define a class name"))
        {
            _lexerName = match.convertBasedOn(string);
            _lexerName.shrink_to_fit();
        }
        else
        {
            spdlog::warn(( "Impossible to parse class token at {}"_f << _token.startLine).toStdStringView());
            return false;
        }

        if (string.regexReplace(R"(^\w+\s*:)", "", 1))
        {
            std::vector<String> parents;
            int bracketsCount = 0;
            String tmp;
            for (int i = 0; i < string.size(); ++i)
            {
                if (string[i] == '<')
                {
                    ++bracketsCount;
                }
                else if (string[i] == '>')
                {
                    --bracketsCount;
                }
                tmp.push_back(string[i]);

                if (bracketsCount == 0)
                {
                    if (string[i] == ',')
                    {
                        tmp.trim(',').trim(' ');
                        parents.push_back(std::move(tmp));
                    }
                }
            }
            parents.push_back(std::move(tmp));

            for (auto&& parentStr : parents)
            {
                InheritanceType type = InheritanceType::Private;
                if (parentStr.regexReplace(R"(\s*public\s*)", "", 1))
                {
                    type = InheritanceType::Public;
                }
                else if (parentStr.regexReplace(R"(\s*protected\s*)", "", 1))
                {
                    type = InheritanceType::Protected;
                }
                else
                {
                    parentStr.regexReplace(R"(\s*private\s*)", "", 1);
                    type = InheritanceType::Private;
                }

                parentStr.regexReplace(R"(\s*$)", "", 1);
                parentStr.shrink_to_fit();

                ParentUnit parent;
                parent.name = std::move(parentStr);
                parent.type = type;
                _parents.push_back(std::move(parent));
            }
        }

        return true;
    }

    bool ClassLexer::DoScopeParse()
    {
        if (!BaseLexer::DoScopeParse())
        {
            return false;
        }

        const auto* openedBracket = _token.endData - 1; // -1 - to back to the '{' correspoinding to regex expr
        if (!Verify(*openedBracket == '{', "Impossible to define an class scope."))
        {
            spdlog::error(("Impossible to define an class scope '{}'"_f << _lexerName.c_str()).toStdStringView());
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data().c_str(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data().c_str(), closedBracket) };

        Assert(!!_closeScope->string);
        Assert(!!_openScope->string);

        closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        return true;
    }

    bool ClassLexer::DoMarkingParse()
    {
        if (!BaseLexer::DoMarkingParse())
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

            begin -= marker.size();
            if (begin >= _reader->Data().c_str())
            {
                if (String(begin, marker.size()).regexMatch(marker))
                {
                    Marker marker;

                    while (*begin != '(')
                    {
                        marker.rule.push_back(*begin);
                        ++begin;
                    }

                    const auto* end = Utils::FindClosedBracket(begin, ')', '(');
                    for (auto param : String(begin, end - begin).split(","))
                    {
                        param.trim(' ').trim('(').trim(')');
                        marker.params.push_back(std::move(param));
                    }

                    _marking = std::move(marker);
                }
            }
        }

        return true;
    }

    bool ClassLexer::DoPostParse()
    {
        if (!BaseLexer::DoPostParse())
        {
            return false;
        }

        TryToFindTemplate();

        RecognizeFields();

        return true;
    }

    void ClassLexer::TryToFindTemplate()
    {
        auto* begin = _token.beginData;

        if (auto string = Cpp::TryToExtrudeTemplate(this))
        {
            _isTemplate = true;

            string.trim('<').trim('>');
            int bracketsCount = 0;
            String tmp;
            for (int i = 0; i < string.size(); ++i)
            {
                if (string[i] == '<')
                {
                    ++bracketsCount;
                }
                else if (string[i] == '>')
                {
                    --bracketsCount;
                }
                tmp.push_back(string[i]);

                if (bracketsCount == 0)
                {
                    if (string[i] == ',')
                    {
                        tmp.trim(',').trim(' ');
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

    void ClassLexer::RecognizeFields()
    {
        Assert(!!_closeScope->string);
        Assert(!!_openScope->string);
        String body(_openScope->string, _closeScope->string ? _closeScope->string - _openScope->string : String::Settings::invalidSize);
        body.trim('{').trim('}');

        RemoveNestedScopes(body);

        const auto publics = body.regexFindAll(R"(^\s*public\s*\:)", 0, 0,0, PCRE2_MULTILINE);
        const auto protecteds = body.regexFindAll(R"(^\s*protected\s*\:)", 0, 0,0, PCRE2_MULTILINE);
        const auto privates = body.regexFindAll(R"(^\s*private\s*\:)", 0, 0,0, PCRE2_MULTILINE);

        body.regexIterate(
            R"(^\s*((static\s+)|(constexpr\s+)|(const\s+)|(constinit\s+))*[\w:]+(\<.*\>)?\s+\w+(((\s*=).*)|(;)))",
            [&](const Core::RegexMatch::MatchedData& field)
            {
                auto str = field.convertBasedOn(body);
                str.regexReplace(R"([\s;]*$)", "", 1, 0, 0, PCRE2_MULTILINE);
                str.regexReplace(R"(^\s*)", "", 1, 0, 0, PCRE2_MULTILINE);

                Field tempField;

                if (str.regexFind(R"(^\s*(using|namespace|enum))"))
                {
                    return true;
                }

                if (str.regexFind(R"(static\s+)"))
                {
                    tempField.isStatic = true;
                    str.regexReplace(R"(static\s+)", "", 1);
                }
                if (str.regexFind(R"(const\s+)"))
                {
                    tempField.isConst = true;
                    str.regexReplace(R"(const\s+)", "", 1);
                }
                if (str.regexFind(R"(constexpr\s+)"))
                {
                    tempField.isConstexpr = true;
                    str.regexReplace(R"(constexpr\s+)", "", 1);
                }
                if (str.regexFind(R"(constinit\s+)"))
                {
                    tempField.isConstinit = true;
                    str.regexReplace(R"(constinit\s+)", "", 1);
                }

                const char* typeRegexExpr = R"(^\s*[\w:\*\&]+(\<[\[\]\(\)\w ,\<\>:\*\&\.\+\-]*\>)?)";
                if (auto matchType = str.regexFind(typeRegexExpr))
                {
                    tempField.type = matchType.convertBasedOn(str);
                    tempField.type.shrink_to_fit();
                    str.regexReplace(typeRegexExpr, "", 1);
                    std::cerr << tempField.type;
                    str.trimStart(' ');
                }
                else
                {
                    Assert();
                    spdlog::error(("Impossible to define a class's field type. Class: '{}'"_f << _lexerName.c_str()).toStdStringView());
                    return true;
                }

                str.regexReplace(R"(^\s*)", "", 1, 0, 0, PCRE2_MULTILINE);

                if (auto matchName = str.regexFind(R"(^\w+)"))
                {
                    tempField.name = matchName.convertBasedOn(str);
                    tempField.name.shrink_to_fit();
                    str.regexReplace(R"(^\w+)", "", 1);
                    str.trimStart(' ');

                    std::cerr << std::setw(50) << tempField.name << std::endl;
                }
                else
                {
                    Assert();
                    spdlog::error(("Impossible to define a class's field name. Class: '{}'"_f << _lexerName.c_str()).toStdStringView());
                    return true;
                }

                int64_t minDistance = (std::numeric_limits<int64_t>::max)();
                AccessSpecifier accessSpecifier = AccessSpecifier::Private;
                for (auto&& token : publics)
                {
                    const auto distance = static_cast<int64_t>(field.offset) - static_cast<int64_t>(token.offset);
                    if (distance >= 0 && distance < minDistance)
                    {
                        minDistance = distance;
                        accessSpecifier = AccessSpecifier::Public;
                    }
                }
                for (auto&& token : protecteds)
                {
                    const auto distance = static_cast<int64_t>(field.offset) - static_cast<int64_t>(token.offset);
                    if (distance >= 0 && distance < minDistance)
                    {
                        minDistance = distance;
                        accessSpecifier = AccessSpecifier::Protected;
                    }
                }
                for (auto&& token : privates)
                {
                    const auto distance = static_cast<int64_t>(field.offset) - static_cast<int64_t>(token.offset);
                    if (distance >= 0 && distance < minDistance)
                    {
                        minDistance = distance;
                        accessSpecifier = AccessSpecifier::Private;
                    }
                }
                tempField.accessSpecifier = accessSpecifier;

                _fields.push_back(std::move(tempField));

                return true;
            },
            0, 0, 0, PCRE2_MULTILINE);
    }

    void ClassLexer::RemoveNestedScopes(String& body)
    {
        const String::CharT* opened = nullptr;
        while ((opened = Utils::FindFirstBracket(body.c_str(), '{')))
        {
            if (const auto* closed = Utils::FindClosedBracket(opened, '}', '{'))
            {
                body.erase(opened - body.c_str(), closed - body.c_str());
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
