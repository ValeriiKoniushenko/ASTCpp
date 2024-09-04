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

#include "String.h"

#include "../Lexers/BaseLexer.h"

namespace Ast::Utils
{
    const String::CharT* SkipBracketsR(const BaseLexer* lexer, const String::CharT* str, String::CharT openBracket, String::CharT closedBracket)
    {
        if (!Verify(lexer->GetTokenReader().IsValid()) || *str != closedBracket || !Verify(str))
        {
            return nullptr;
        }

        if (*str == closedBracket)
        {
            --str; // to skip closedBracket
        }
        else
        {
            return nullptr;
        }

        int bracketsCount = -1;
        while(str >= lexer->GetReader()->Data().c_str() && bracketsCount != 0)
        {
            if (*str == openBracket)
            {
                ++bracketsCount;
            }
            else if (*str == closedBracket)
            {
                --bracketsCount;
            }

            --str;
        }

        return ++str;
    }

} // namespace Ast::Utils