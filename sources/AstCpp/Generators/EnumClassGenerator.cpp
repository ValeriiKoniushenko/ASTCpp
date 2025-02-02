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

    String EnumClassGeneratorDecl::OnGenerate(const BaseLexer* lexer) const
    {
        auto out = GeneratorUnitDecl::OnGenerate(lexer);

        out += String(R"(   enum class REPLACE_WITH_ENUM_NAME : REPLACE_WITH_ENUM_TYPE;
    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, const Ast::String&> Name();

    [[nodiscard]] inline const Ast::String& ToString(const REPLACE_WITH_ENUM_NAME value);

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_ENUM_NAME>, std::optional<REPLACE_WITH_ENUM_NAME>> FromString(const Ast::String& value);

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

        out.ReplaceAll("REPLACE_WITH_ENUM_NAME", enumLexer->GetLexerName());
        out.ReplaceAll("REPLACE_WITH_ENUM_TYPE", enumLexer->GetType());

        return out;
    }

} // namespace Ast::Cpp