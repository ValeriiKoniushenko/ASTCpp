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

#include "CommonRules.h"

#include "Ast/Lexers/BaseLexer.h"
#include "AstCpp/Lexers/ClassLexer.h"

namespace Ast::Cpp
{

    LineCountRule::LineCountRule(std::size_t max)
    {
        SetMaxLineCount(max);
    }

    bool LineCountRule::IsCorrespondingTheRules(const BaseLexer* lexer, LogCollector& logCollector, const char* additionalMessage) const
    {
        if (!Verify(lexer))
        {
            return false;
        }

        if (lexer->GetOpenScope() && lexer->GetCloseScope())
        {
            if (lexer->GetCloseScope()->line - lexer->GetOpenScope()->line <= _max)
            {
                return true;
            }
        }

        logCollector.AddLog(
            { String::Format("ClassRule: invalid class name. Additional message: '{}'", additionalMessage ? additionalMessage : "none"),
              GetLogType() });

        return false;
    }
} // namespace Ast::Cpp
