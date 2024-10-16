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
    class NamespaceLexer final : public BaseLexer
    {
    public:
        using Ptr = boost::intrusive_ptr<NamespaceLexer>;

        inline static const auto typeName = "namespace"_atom;

        [[nodiscard]] static Ptr Create(const ContentStream::Ptr& fileReader)
        {
            return { new NamespaceLexer(fileReader) };
        }

        ~NamespaceLexer() override = default;

        [[nodiscard]] const std::vector<String>& GetNameList() { return _nameList; }

    protected:
        explicit NamespaceLexer(const ContentStream::Ptr& fileReader);

        bool DoValidate(LogCollector& logCollector) override;
        bool DoValidateScope(LogCollector& logCollector) override;

    private:
        std::vector<String> _nameList; // e.g: namespace A::B -> { "A", "B" }
    };

} // namespace Ast::Cpp