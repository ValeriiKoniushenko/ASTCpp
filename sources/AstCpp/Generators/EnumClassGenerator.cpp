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

#include "EnumClassGenerator.h"

namespace Ast::Cpp
{
#if 0
    /*String EnumClassGeneratorDecl::OnFinishGenerateNeededStartOfFile(const BaseLexer* lexer) const
    {
        const auto* realLexer = dynamic_cast<const EnumClassLexer*>(lexer);
        if (!Verify(realLexer, "Was met not expected lexer"))
        {
            return {};
        }

        return "enum class {} : {};{}"_f << realLexer->GetLexerName() << realLexer->GetType() << Code::Endl();
    }

    String EnumClassGeneratorDecl::OnGenerate(const BaseLexer* lexer) const
    {
        auto out = GeneratorUnitDecl::OnGenerate(lexer);

        out += String(R"(    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, const Ast::String&> Name();

    [[nodiscard]] inline const Ast::String& ToString(const REPLACE_WITH_ENUM_NAME value);

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::optional<REPLACE_WITH_ENUM_NAME>> FromString(const Ast::String&
    value);

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, uint32_t> Size() noexcept;

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::vector<REPLACE_WITH_ENUM_NAME>> ToVector();

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::unordered_set<REPLACE_WITH_ENUM_NAME>> ToSet();

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::unordered_map<REPLACE_WITH_ENUM_NAME, Ast::String>> ToMap();)") +
               Code::Endl() + Code::Endl();

        auto enumLexer = lexer->CastTo<EnumClassLexer>();

        out.replaceAll("REPLACE_WITH_ENUM_NAME", enumLexer->GetLexerName());
        out.replaceAll("REPLACE_WITH_ENUM_TYPE", enumLexer->GetType());

        return out;
    }

    String EnumClassGeneratorImpl::OnGenerate(const BaseLexer* lexer) const
    {
        auto out = GeneratorUnitImpl::OnGenerate(lexer);

        const auto* realLexer = dynamic_cast<const EnumClassLexer*>(lexer);
        if (!Verify(realLexer, "Was met not expected lexer"))
        {
            return {};
        }

        out += GenerateNameImpl(realLexer) + Code::Endl();
        out += GenerateToStringImpl(realLexer) + Code::Endl();
        out += GenerateFromStringImpl(realLexer) + Code::Endl();
        out += GenerateSizeImpl(realLexer) + Code::Endl();
        out += GenerateToVectorImpl(realLexer) + Code::Endl();
        out += GenerateToSetImpl(realLexer) + Code::Endl();
        out += GenerateToMapImpl(realLexer) + Code::Endl();

        return out;
    }*/

    String EnumClassGeneratorImpl::GenerateNameImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, const Ast::String&> Name()
    {
        static const auto returnValue = "REPLACE_WITH_ENUM_NAME"_atom;
        return returnValue;
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        return out;
    }

    String EnumClassGeneratorImpl::GenerateToStringImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    [[nodiscard]] inline const Ast::String& ToString(const REPLACE_WITH_ENUM_NAME value)
    {
        REPLACE_WITH_IFS

        static const auto returnValue = ""_atom;
        return returnValue;
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        String ifs;
        for (const auto& c : lexer->GetConstants())
        {
            ifs += R"(  if (value == {}::{})
    {
        static const auto returnValue = "{}"_atom;
        return returnValue;
    }{})"_f << lexer->GetLexerName()
            << c.name << c.name << Code::Endl();
        }

        out.replaceAll("REPLACE_WITH_IFS", ifs);

        return out;
    }

    String EnumClassGeneratorImpl::GenerateFromStringImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::optional<REPLACE_WITH_ENUM_NAME>> FromString(const Ast::String& value)
    {
        REPLACE_WITH_IFS

        return std::nullopt;
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        String ifs;
        for (const auto& c : lexer->GetConstants())
        {
            ifs += R"(  if (value == "{}"_atom)
        {
            return {}::{};
        }{})"_f << c.name
                << lexer->GetLexerName() << c.name << Code::Endl();
        }

        out.replaceAll("REPLACE_WITH_IFS", ifs);

        return out;
    }

    String EnumClassGeneratorImpl::GenerateSizeImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, uint32_t> Size() noexcept
    {
        return REPLACE_WITH_ENUM_SIZE;
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());
        out.replaceAll("REPLACE_WITH_ENUM_SIZE", String::MakeFrom(lexer->GetConstants().size()));

        return out;
    }

    String EnumClassGeneratorImpl::GenerateToVectorImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::vector<REPLACE_WITH_ENUM_NAME>> ToVector()
    {
        return { REPLACE_WITH_ENUM_VALUE_LIST };
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        String values;
        for (const auto& c : lexer->GetConstants())
        {
            values += "{}::{},{}"_f << lexer->GetLexerName() << c.name << Code::Endl();
        }
        values.trimEnd('\n');
        values.trimEnd('\r');
        values.trimEnd('\n');
        values.trimEnd('\r');
        values.trimEnd(',');
        out.replaceAll("REPLACE_WITH_ENUM_VALUE_LIST", values);

        return out;
    }

    String EnumClassGeneratorImpl::GenerateToSetImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::unordered_set<REPLACE_WITH_ENUM_NAME>> ToSet()
    {
        return { REPLACE_WITH_ENUM_VALUE_LIST };
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        String values;
        for (const auto& c : lexer->GetConstants())
        {
            values += "{}::{},{}"_f << lexer->GetLexerName() << c.name << Code::Endl();
        }
        values.trimEnd('\n');
        values.trimEnd('\r');
        values.trimEnd('\n');
        values.trimEnd('\r');
        values.trimEnd(',');
        out.replaceAll("REPLACE_WITH_ENUM_VALUE_LIST", values);

        return out;
    }

    String EnumClassGeneratorImpl::GenerateToMapImpl(const EnumClassLexer* lexer) const
    {
        String out = R"(    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::unordered_map<REPLACE_WITH_ENUM_NAME, Ast::String>> ToMap()
    {
        return { REPLACE_WITH_ENUM_TOKENS };
    })";
        out.replaceAll("REPLACE_WITH_ENUM_NAME", lexer->GetLexerName());

        String tokens;
        for (const auto& c : lexer->GetConstants())
        {
            String token = R"({ LEXER_NAME::CONST_NAME, "CONST_NAME"_atom }, )";
            token.replaceAll("LEXER_NAME", lexer->GetLexerName());
            token.replaceAll("CONST_NAME", c.name);
            tokens += std::move(token);
        }

        tokens.trimEnd(' ');
        tokens.trimEnd(',');

        out.replaceAll("REPLACE_WITH_ENUM_TOKENS", tokens);

        return out;
    }
#endif
} // namespace Ast::Cpp