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

#include "Ast/CommonTypes.h"
#include "../Lexers/BaseLexer.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace Ast
{

    template<IsLexerOrBase Lexer>
    class ILexerModifier : public ::Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<ILexerModifier<Lexer>>
    {
    public:
        AST_CLASS(ILexerModifier<Lexer>);

        [[nodiscard]] virtual bool IsValid() const noexcept
        {
            return !!_object;
        }

        void AttachTo(const Lexer::Ptr& object)
        {
            _object = object;
            _object->_modifierParams.wasModified = true;
        }

    protected:
        ILexerModifier() = default;
        ILexerModifier(const Lexer::Ptr& object)
        {
            AttachTo(object);
        }

    protected:
        typename Lexer::Ptr _object;
    };

} // namespace Ast
