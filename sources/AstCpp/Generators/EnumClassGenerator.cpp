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

    String EnumClassGenerator::OnGenerate(const BaseLexer* lexer) const
    {
        String out;

        out += GenerateName(lexer) + Code::Endl() + Code::Endl();

        return out;
    }

    String EnumClassGenerator::PreGenerate(const BaseLexer* lexer) const
    {
        return String("namespace Reflect::") + namespaceName + Code::Endl() + "{" + Code::Endl();
    }
    String EnumClassGenerator::PostGenerate(const BaseLexer* lexer) const
    {
        return String("} // namespace Reflect::") + namespaceName + Code::Endl();
    }

    String EnumClassGenerator::GenerateName(const BaseLexer* lexer) const
    {
        auto l = lexer->CastTo<EnumClassLexer>();
        String out = R"(template<class T>
[[nodiscard]] std::enable_if_t<std::is_same_v<T, REPLACE_WITH_NAME>, const Ast::String&> Name()
{
    static const auto returnValue = "REPLACE_WITH_NAME"_atom;
    return returnValue;
})";
        out.ReplaceAll("REPLACE_WITH_NAME", l->GetLexerName());
        return out;
    }

} // namespace Ast::Cpp