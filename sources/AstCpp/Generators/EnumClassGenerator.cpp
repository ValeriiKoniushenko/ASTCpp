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

    EnumClassGenerator::EnumClassGenerator()
    {
        addToGlobalHead(R"(#include <string>)");
        addToGlobalHead(R"(#include <cstring>)");
        addToGlobalHead(R"(#include <type_traits>)");
        addToGlobalHead(R"(#include <vector>)");
        addToGlobalHead(R"(#include <optional>)");
    }

    String EnumClassGenerator::generateLocalHead()
    {
        auto* lexer = getLexer();

        String forwardDecl = "enum class " + lexer->GetLexerName();
        if (lexer->GetType() != "int")
        {
            forwardDecl += " : " + lexer->GetType();
        }
        forwardDecl += ";";

        {
            if (auto fullPath = getNamespacePath())
            {
                auto finalNamespaceStr = "namespace " + fullPath;
                forwardDecl = finalNamespaceStr + "{" + forwardDecl + "}";
            }
        }
        forwardDecl += ITextSourceReader::Code::Endl();

        String out = getLexerDeco() + forwardDecl + getNamespaceStr() + "{";

        return out;
    }

    String EnumClassGenerator::generateBody()
    {
        auto* lexer = getLexer();

        String out = R"(template<class T>
[[nodiscard]] consteval std::enable_if_t<std::is_same_v<T, CHANGEME_absolute_name_just>, const char*> Name() noexcept
{
    return "CHANGEME_name";
}

template<class T>
[[nodiscard]] consteval std::enable_if_t<std::is_same_v<T, CHANGEME_absolute_name_just>, const char*> AbsoluteName() noexcept
{
    return "CHANGEME_absolute_name_just";
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, CHANGEME_absolute_name_just>, const std::vector<std::string>&> AbsoluteNameAsVector() noexcept
{
    static const std::vector<std::string> data = { CHANGEME_absolute_name_as_vector };
    return data;
}

template<class T>
[[nodiscard]] consteval std::enable_if_t<std::is_same_v<T, CHANGEME_absolute_name_just>, int> Count() noexcept
{
    return CHANGEME_count;
}

[[nodiscard]] constexpr const char* ToString(const CHANGEME_absolute_name_just value) noexcept
{
    const auto casted = static_cast<std::underlying_type_t<CHANGEME_absolute_name_just>>(value);
    CHANGEME_to_string_func

    return nullptr;
}

template<class T>
[[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, CHANGEME_absolute_name_just>, std::optional<T>> FromString(const char* value) noexcept
{
    CHANGEME_from_string_func

    return std::nullopt;
})";

        // name
        {
            out.replaceAll("CHANGEME_name", lexer->GetLexerName());
        }

        auto absolute = lexer->GetFullPath();
        {
            // absolute path as string
            out.replaceAll("CHANGEME_absolute_name_just", absolute.first);

            // absolute path as vector
            String str(128);
            for (auto i : absolute.first.split("::"_atom))
            {
                i = "\"" + i + "\", ";
                str.push_back(std::move(i));
            }
            str.trimEnd(' ');
            str.trimEnd(',');
            out.replaceAll("CHANGEME_absolute_name_as_vector", str);
        }

        // count
        {
            out.replaceAll("CHANGEME_count", String::MakeFrom(lexer->GetConstants().size()));
        }

        // to string func
        {
            String func(512);
            for (auto& constant : lexer->GetConstants())
            {
                String piece = R"(if (casted == VALUE)
{
    return "KEY";
})";
                piece.replaceAll("VALUE", constant.value);
                piece.replaceAll("KEY", constant.name);

                func += std::move(piece);
                func += ITextSourceReader::Code::Endl();
            }

            out.replaceAll("CHANGEME_to_string_func", func);
        }

        // from string func
        {
            String func(512);
            for (auto& constant : lexer->GetConstants())
            {
                String piece = R"(if (strcmp("KEY", value) == 0)
{
    return static_cast<T>(VALUE);
})";
                piece.replaceAll("VALUE", constant.value);
                piece.replaceAll("KEY", constant.name);

                func += std::move(piece);
                func += ITextSourceReader::Code::Endl();
            }

            out.replaceAll("CHANGEME_from_string_func", func);
        }

        return out;
    }

    String EnumClassGenerator::generateLocalTail()
    {
        return "} // " + getNamespaceStr();
    }

    EnumClassLexer* EnumClassGenerator::getLexer()
    {
        return dynamic_cast<EnumClassLexer*>(_lexer);
    }

    String EnumClassGenerator::getNamespacePath() const
    {
        auto fullPath = _lexer->GetFullPath().first;
        int finalSize = fullPath.size() - _lexer->GetLexerName().size() - 2; // 2 == "::"
        fullPath.resize(finalSize < 0 ? 0 : finalSize);

        return fullPath;
    }

    const String& EnumClassGenerator::getNamespaceStr() const
    {
        static const String out = "namespace Reflect::Enum" + ITextSourceReader::Code::Endl();
        return out;
    }
    String EnumClassGenerator::getLexerDeco() const
    {
        String out(512);

        String name = _lexer->GetLexerName();

        out += "// ╔═══════════════════════";
        for (int i = 0; i < name.size(); ++i)
        {
            out += "═";
        }
        out += "═══════════════════════╗";
        out += ITextSourceReader::Code::Endl();

        out += "// ║                       ";
        out += name;
        out += "                       ║";
        out += ITextSourceReader::Code::Endl();

        out += "// ╚═══════════════════════";
        for (int i = 0; i < name.size(); ++i)
        {
            out += "═";
        }
        out += "═══════════════════════╝";

        out += ITextSourceReader::Code::Endl();

        return out;
    }
} // namespace Ast::Cpp