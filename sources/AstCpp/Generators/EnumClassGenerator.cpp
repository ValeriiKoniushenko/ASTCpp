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

        out += String(R"(template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, const Ast::String&> Name();

    [[nodiscard]] inline const Ast::String& ToString(const ExampleEnum value);

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::optional<ExampleEnum>> FromString(const Ast::String& value);

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, uint32_t> Size() noexcept;

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::vector<ExampleEnum>> ToVector();

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_set<ExampleEnum>> ToSet();

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_map<ExampleEnum, Ast::String>> ToMap();)") +
               Code::Endl() + Code::Endl();

        return out;
    }

} // namespace Ast::Cpp