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

#include "Ast/Rule.h"

namespace Ast::Cpp
{
    class LineCountRule : public Rule, public OverrideRuleLogType
    {
    public:
        AST_CLASS(LineCountRule)

        explicit LineCountRule(std::size_t max = 0);

        void SetMaxLineCount(std::size_t max) noexcept { _max = max; };
        [[nodiscard]] std::size_t GetMaxLineCount() const noexcept { return _max; }
        [[nodiscard]] bool IsCorrespondingTheRules(const BaseLexer* lexer, LogCollector& logCollector,
                                                   const char* additionalMessage = nullptr) const override;

    private:
        std::size_t _max = 0;
    };

    class NameRule : public Rule, public OverrideRuleLogType
    {
    public:
        AST_CLASS(NameRule)

        explicit NameRule(const String& regexNameRule);

        void SetRegexNameRule(const String& regexNameRule);
        [[nodiscard]] bool IsCorrespondingTheRules(const BaseLexer* lexer, LogCollector& logCollector,
                                                   const char* additionalMessage = nullptr) const override;

    private:
        String _regexNameRule;
    };

} // namespace Ast::Cpp