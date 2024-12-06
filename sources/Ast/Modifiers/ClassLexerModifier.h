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

#include "AstCpp/Lexers/ClassLexer.h"
#include "BaseLexerModifier.h"

namespace Ast::Cpp
{

    template<IsClassLexerBase Lexer = ClassLexer>
    class ClassLexerModifier : public BaseLexerModifier<Lexer>
    {
    public:
        AST_CLASS(ClassLexerModifier<Lexer>);

        ClassLexerModifier() = default;
        explicit ClassLexerModifier(const typename Lexer::Ptr& object) : BaseLexerModifier<Lexer>(object)
        {
        }

        bool AddField(typename Lexer::Field field)
        {
            if (!Verify(this->IsValid()))
            {
                return false;
            }

            this->GetLexer()->_fields.push_back(std::move(field));

            return true;
        }

        bool AddParent(typename Lexer::ParentUnit parent)
        {
            if (!Verify(this->IsValid()))
            {
                return false;
            }

            this->GetLexer()->_parents.push_back(std::move(parent));

            return true;
        }

        bool AddMethod(typename Lexer::Method method)
        {
            if (!Verify(this->IsValid()))
            {
                return false;
            }

            this->GetLexer()->_methods.push_back(std::move(method));

            return true;
        }
    };

} // namespace Ast::Cpp
