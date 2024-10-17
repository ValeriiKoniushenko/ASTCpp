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


#include "BaseLexerModifier.h"
#include "Ast/Lexers/FileLexer.h"

namespace Ast
{

    template<IsLexerOrBase Lexer = FileLexer>
    class FileLexerModifier : public BaseLexerModifier<Lexer>
    {
    public:
        AST_CLASS(FileLexerModifier<Lexer>);

        FileLexerModifier() = default;
        FileLexerModifier(const Lexer::Ptr& object) : BaseLexerModifier<Lexer>(object)
        {
        }

        void SetFileName(const String& fileName)
        {
            if (!Verify(IsValid()))
            {
                return;
            }
            SetLexerName(fileName);
        }

        void SetPragmaOnce(bool has = true) noexcept
        {
            if (!Verify(IsValid()))
            {
                return;
            }

            _object->_hasPragmaOnce = has;
        }
    };

} // namespace Ast
