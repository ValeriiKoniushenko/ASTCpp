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

#include "EnumClassLexer.h"

#include "Ast/Lexers/FileLexer.h"
#include "spdlog/spdlog.h"
#include "Ast/Readers/ContentStream.h"
#include "Ast/Utils/Scopes.h"
#include "Ast/Utils/String.h"
#include "NamespaceLexer.h"

namespace Ast::Cpp
{

    String EnumClassLexer::Constant::GetTextSource()
    {
        String source = name;
        if (!value.isEmpty())
        {
            source += " = " + value;
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

    EnumClassLexer::Constant EnumClassLexer::GetConstantByValue(const String& value) const
    {
        auto found = std::find_if(_constants.cbegin(), _constants.cend(),
                                  [&value](const Constant& a)
                                  {
                                      return a.value == value;
                                  });

        if (found != _constants.cend())
        {
            return *found;
        }

        return {};
    }

    bool EnumClassLexer::AddConstant(const String& name, String value)
    {
        if (!Verify(std::find_if(_constants.cbegin(), _constants.cend(), [&name](const Constant& a)
            {
                return a.name == name;
            }) == _constants.cend(), "Impossible to add new enum class constant, because such name of the constant already exists"))
        {
            return false;
        }

        _constants.emplace_back(name, std::move(value));
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
        enumSource.trimEnd('\r').trimEnd('\n').trimEnd('\r').trimEnd(',');

        enumSource += Code::Endl() + "};" + Code::Endl();

        source.source.insert(pos, enumSource.c_str());
        source.carets["write-point"_atom] = source.source.size();

        return true;
    }

    EnumClassLexer::EnumClassLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool EnumClassLexer::DoParse()
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            spdlog::error("EnumClassLexer: Impossible to work with an invalid token");
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.regexReplace(R"(\n|\r|(enum class)|\{)", " ");
        string.trim(' ');
        if (string.isEmpty())
        {
            spdlog::error(("Impossible to parse enum class token at {}"_f << _token.startLine).toStdStringView());
            return false;
        }

        auto match = string.regexFind("^\\w+");
        if (Verify(!!match, "Impossible to define an enum class name"))
        {
            _lexerName = match.convertBasedOn(string);
            _lexerName.shrink_to_fit();
        }
        else
        {
            spdlog::error(("Impossible to parse enum class token at {}"_f << _token.startLine).toStdStringView());
            return false;
        }

        if (string.regexReplace(R"(^\w+\s*:)", "", 1))
        {
            _type = string.trim(' ');
        }

        return true;
    }

    bool EnumClassLexer::DoScopeParse()
    {
        if (!BaseLexer::DoScopeParse())
        {
            return false;
        }

        const auto* openedBracket = _token.endData;
        while (String::Toolset::IsSpace(*openedBracket))
        {
            ++openedBracket;
        }

        // Looks like it just forward declaration
        if (*openedBracket != '{')
        {
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data().c_str(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data().c_str(), closedBracket) };

        if (!RecognizeConstants())
        {
            return false;
        }

        return true;
    }

    bool EnumClassLexer::DoMarkingParse()
    {
        if (!BaseLexer::DoMarkingParse())
        {
            return false;
        }

        auto limits = GetReaderLimits();

        const char* begin = _token.beginData;
        // trying to find closed bracket
        if (Verify(begin) && *begin != ')')
        {
            do
            {
                --begin;
            } while (begin > limits.first && (String::IsSpace(*begin) || *begin == ';'));
        }


        // Corresponding to AstCpp/Markers.h -> #define ENUM_CLASS
        if ((begin = Ast::Utils::SkipBracketsR(this, begin, '(', ')', limits.first)))
        {
            while (begin > limits.first && String::IsSpace(*begin))
            {
                --begin;
            }

            begin -= marker.size();
            if (begin > limits.first)
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

    void EnumClassLexer::ValidateMark()
    {
        if (_marking)
        {
            if (_parentLexer && !_parentLexer->IsTypeOf<FileLexer>() && !_parentLexer->IsTypeOf<NamespaceLexer>())
            {
                _marking = std::nullopt;

                spdlog::error(( "Marking of the lexer '{}' of type '{}' is impossible. Becuase marking of this lexer available only in a file or namespace scope. It can't be marked inside '{}': '{}'"_f
                    << _lexerName << _lexerType << _parentLexer->GetLexerType() << _parentLexer->GetLexerName()).toStdStringView());
            }
        }
    }

    bool EnumClassLexer::RecognizeConstants()
    {
        if (!Verify(_openScope.has_value() && _openScope->IsValid() && _closeScope.has_value() && _closeScope->IsValid()))
        {
            spdlog::error(("Impossible to get a enum class scope '{}'"_f << _lexerName.c_str()).toStdStringView());
            return false;
        }

        String buffer(_openScope->string, _closeScope->string - _openScope->string);
        buffer.trim('{').trim('}').regexReplaceAll(R"(\s)", "", 1);

        bool canParseValues = true;

        auto tokens = buffer.split(","_atom);
        _constants.clear();
        _constants.reserve(tokens.size());

        String* lastValue = nullptr;

        for (auto& constant : tokens)
        {
            auto splitted = constant.split("=");

            if (1 > splitted.size() || splitted.size() > 2)
            {
                String file;
                if (_reader)
                {
                    file = "File '{}'"_f << _reader->GetFilePath();
                }

                errorLog("The error occurred while parsing constants of the enum class: '{}' in file: {}"_f << _lexerName << file);
                Assert();
                return false;
            }

            Constant tmp;
            tmp.name = std::move(splitted.front());
            tmp.name.shrink_to_fit();

            if (splitted.size() == 2)
            {
                tmp.value = std::move(splitted.back());
                tmp.value.shrink_to_fit();
            }
            else
            {
                if (lastValue)
                {
                    tmp.value = *lastValue + " + 1";
                }
                else
                {
                    tmp.value = "0";
                }
            }
            _constants.push_back(std::move(tmp));
            lastValue = &_constants.back().value;
        }

        return true;
    }

} // namespace Ast::Cpp