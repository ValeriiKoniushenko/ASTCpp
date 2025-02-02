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

#pragma once

#include "Ast/Generators/GeneratorUnit.h"

namespace Ast::Cpp
{
    class GeneratorUnit : public Ast::GeneratorUnit
    {
    public:
        using Code = ITextSourceReader::Code;

        inline static const char* namespaceName = "Reflect";

    protected:
        template<IsLexer Lexer>
        [[nodiscard]] static GeneratorUnit Create(const String& nestedNamespace)
        {
            return GeneratorUnit(Lexer::typeName, nestedNamespace);
        }

        [[nodiscard]] String PreGenerate(const BaseLexer* lexer) const override;
        [[nodiscard]] String PostGenerate(const BaseLexer* lexer) const override;

        GeneratorUnit(const String& type, const String& nestedNamespace) :
            Ast::GeneratorUnit(type),
            _nestedNamespace{ nestedNamespace }
        {
        }

    protected:
        const String _nestedNamespace;
    };
} // namespace Ast::Cpp