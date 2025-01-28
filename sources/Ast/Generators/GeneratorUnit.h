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

#include "../CommonTypes.h"
#include "../Lexers/BaseLexer.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace Ast
{

    class GeneratorUnit : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<GeneratorUnit>
    {
    public:
        AST_CLASS(GeneratorUnit)

        struct Hasher
        {
            size_t operator()(const GeneratorUnit& gen) const { return gen.GetType().MakeHash(); }
        };
        struct HasherPtr
        {
            size_t operator()(const Ptr& gen) const { return gen->GetType().MakeHash(); }
        };

    public:
        ~GeneratorUnit() override = default;

        // Don't call this method directly. Override it & implement needed logic
        virtual String Generate(const BaseLexer* lexer) const { return String(); }
        const String& GetType() const { return _type; }

        [[nodiscard]] bool operator==(const GeneratorUnit& other) const { return _type == other._type; }
        [[nodiscard]] bool operator==(const CPtr& other) const { return _type == other->_type; }

    protected:
        const String _type;

        template<IsLexer Lexer>
        [[nodiscard]] static GeneratorUnit Create()
        {
            return GeneratorUnit(Lexer::typeName);
        }

    private:
        explicit GeneratorUnit(const String& type)
            : _type{ type } {};
    };

    template<class T>
    concept IsGeneratorUnit = std::derived_from<T, GeneratorUnit> && requires(T)
    {
        { T() };
    };

} // namespace Ast
