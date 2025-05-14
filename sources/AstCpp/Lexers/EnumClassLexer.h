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

#pragma once

#include "Ast/Lexers/BaseLexer.h"

namespace Ast
{
    class ContentStream;
} // namespace Ast

namespace Ast::Cpp
{
    class EnumClassLexer final : public BaseLexer
    {
    public:
        AST_CLASS(EnumClassLexer)

        inline static const String marker = "ENUM_CLASS";

        struct Constant : public ITextSourceReader
        {
            [[nodiscard]] String GetTextSource() override;

            Constant() = default;
            Constant(String name, String value)
                : name{ std::move(name) },
                  value{ value }
            {
            }

            String name;
            String value;
        };

        inline static const auto typeName = "enum class"_atom;

    public:
        [[nodiscard]] static Ptr Create(const ContentStream::Ptr& fileReader) { return { new EnumClassLexer(fileReader) }; }

        ~EnumClassLexer() override = default;

        [[nodiscard]] const String& GetType() const noexcept { return _type; }
        [[nodiscard]] const std::vector<Constant>& GetConstants() const noexcept { return _constants; }
        [[nodiscard]] Constant GetConstant(const String& name) const;
        [[nodiscard]] Constant GetConstantByValue(const String& value) const;

        bool AddConstant(const String& name, String value = {});

        bool GenerateTextSource(TextSourceT& source) override;

    protected:
        explicit EnumClassLexer(const ContentStream::Ptr& fileReader);

        bool DoParse() override;
        bool DoScopeParse() override;
        bool DoMarkingParse() override;
        void ValidateMark() override;

    private:
        bool RecognizeConstants();

    private:
        String _type = "int"_atom;
        // TODO: change std::vector to another data structure
        std::vector<Constant> _constants;
    };

} // namespace Ast::Cpp