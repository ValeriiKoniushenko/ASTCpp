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

        struct Constant
        {
            String name;
            std::optional<unsigned long long> value;
        };

    public:
        inline static const auto typeName = "enum class"_atom;

        [[nodiscard]] static Ptr Create(const ContentStream::Ptr& fileReader)
        {
            return { new EnumClassLexer(fileReader) };
        }

        ~EnumClassLexer() override = default;

        [[nodiscard]] const String& GetType() const noexcept { return _type; }
        [[nodiscard]] const std::vector<Constant>& GetConstants() const noexcept { return _constants; }

    protected:
        explicit EnumClassLexer(const ContentStream::Ptr& fileReader);

        bool DoValidate(LogCollector& logCollector) override;
        bool DoValidateScope(LogCollector& logCollector) override;

    private:
        bool RecognizeConstants(LogCollector& logCollector);

    private:
        String _type = "int"_atom;
        std::vector<Constant> _constants;
    };

} // namespace Ast::Cpp