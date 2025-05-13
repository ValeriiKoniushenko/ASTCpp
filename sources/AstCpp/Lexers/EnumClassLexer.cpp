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
            spdlog::error(("Impossible to get an enum class scope '{}'"_f << _lexerName.c_str()).toStdStringView());
            return false;
        }

        String buffer(_openScope->string, _closeScope->string - _openScope->string);
        buffer.trim('{').trim('}').regexReplaceAll(R"(\s)", "", 1);
        for (auto& constant : buffer.split(","_atom))
        {
            if (auto match = constant.regexFind(R"(^\w+)"))
            {
                _constants.emplace_back(match.convertBasedOn(constant), std::nullopt);
                _constants.back().name.shrink_to_fit();
            }
        }

        return true;
    }

} // namespace Ast::Cpp