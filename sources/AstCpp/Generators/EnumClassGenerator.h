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

#include "../Lexers/EnumClassLexer.h"
#include "Ast/Generators/GeneratorUnit.h"

namespace Ast::Cpp
{

    class EnumClassGenerator : public GeneratorUnit
    {
    public:
        AST_CLASS(EnumClassGenerator)

        using Code = ITextSourceReader::Code;

        // TODO: fix it. Add new layer Cpp::GeneratorUnit
        inline static const char* namespaceName = "Enum";

        EnumClassGenerator()
            : GeneratorUnit(GeneratorUnit::Create<EnumClassLexer>())
        {
        }
        ~EnumClassGenerator() override = default;

        [[nodiscard]] static Ptr Create() { return new EnumClassGenerator(); }

        [[nodiscard]] String OnGenerate(const BaseLexer* lexer) const override;

    protected:
        // TODO: fix it. Add new layer Cpp::GeneratorUnit
        [[nodiscard]] String PreGenerate(const BaseLexer* lexer) const override;
        // TODO: fix it. Add new layer Cpp::GeneratorUnit
        [[nodiscard]] String PostGenerate(const BaseLexer* lexer) const override;

    private:
        [[nodiscard]] String GenerateName(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateToString(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateFromString(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateSize(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateToVector(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateToSet(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateToMap(const BaseLexer* lexer) const;
        [[nodiscard]] String GenerateToJsonString(const BaseLexer* lexer) const;
    };

} // namespace Ast::Cpp
