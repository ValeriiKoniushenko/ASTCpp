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
#include "GeneratorUnit.h"

namespace Ast::Cpp
{

    class EnumClassGeneratorDecl : public GeneratorUnitDecl
    {
    public:
        AST_CLASS(EnumClassGeneratorDecl)

        using Code = ITextSourceReader::Code;

        inline static const char* namespaceName = "Enum";

        EnumClassGeneratorDecl()
            : GeneratorUnitDecl(GeneratorUnitDecl::Create<EnumClassLexer>(namespaceName))
        {
            AddLocalInclude("Ast/CommonTypes.h");
            AddGlobalInclude("type_traits");
            AddGlobalInclude("vector");
            AddGlobalInclude("unordered_map");
            AddGlobalInclude("unordered_set");
        }

        ~EnumClassGeneratorDecl() override = default;

        [[nodiscard]] static Ptr Create() { return new EnumClassGeneratorDecl(); }

        [[nodiscard]] String OnFinishGenerateNeededStartOfFile(const BaseLexer* lexer) const override;
        [[nodiscard]] String OnGenerate(const BaseLexer* lexer) const override;
    };

    class EnumClassGeneratorImpl : public GeneratorUnitImpl
    {
    public:
        AST_CLASS(EnumClassGeneratorImpl)

        using Code = ITextSourceReader::Code;

        inline static const char* namespaceName = "Enum";

        EnumClassGeneratorImpl()
            : GeneratorUnitImpl(GeneratorUnitImpl::Create<EnumClassLexer>(namespaceName))
        {
        }
        ~EnumClassGeneratorImpl() override = default;

        [[nodiscard]] static Ptr Create() { return new EnumClassGeneratorImpl(); }

        [[nodiscard]] String OnGenerate(const BaseLexer* lexer) const override;

    private:
        [[nodiscard]] String GenerateNameImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateToStringImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateFromStringImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateSizeImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateToVectorImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateToSetImpl(const EnumClassLexer* lexer) const;
        [[nodiscard]] String GenerateToMapImpl(const EnumClassLexer* lexer) const;
    };

} // namespace Ast::Cpp
