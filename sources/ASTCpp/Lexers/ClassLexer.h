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

#include "AST/Lexers/BaseLexer.h"

#include <unordered_set>

namespace Ast
{
    class FileReader;
} // namespace Ast

namespace Ast::Cpp
{

    class ClassLexer final : public BaseLexer
    {
    public:
        enum class InheritanceType
        {
            Public,
            Private,
            Protected
        };

        struct Parent
        {
            InheritanceType type = InheritanceType::Private;
            String name;
        };

        inline static const auto typeName = "class"_atom;

        explicit ClassLexer(const Ast::FileReader& fileReader);
        ~ClassLexer() override = default;

    protected:
        bool DoValidate(LogCollector& logCollector) override;
        bool DoValidateScope(LogCollector& logCollector) override;

    private:
        void RecognizeFields(LogCollector& logCollector);

    private:
        bool _hasFinal = false;
        std::vector<Parent> _parents;

        std::unordered_set<String> _publicFieldNames;
        std::unordered_set<String> _protectedFieldNames;
        std::unordered_set<String> _privateFieldNames;

    };

} // namespace Ast::Cpp