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
#include "../Readers/Token.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace Ast
{
    class FileReader;
    class LogCollector;
    class BaseLexer;

    template<class T>
    concept IsLexer = (std::derived_from<T, BaseLexer> && requires(T){ { T::typeName}; }) || std::is_void_v<T>;

    class BaseLexer : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<BaseLexer>
    {
    public:
        using Ptr = boost::intrusive_ptr<BaseLexer>;

        struct LineToken final
        {
            const String::CharT* string = nullptr;
            std::size_t line = 0;

            [[nodiscard]] bool IsValid() const noexcept { return string && line != 0; }
            [[nodiscard]] bool operator<(const LineToken& other) const noexcept { return Verify(other.IsValid() && IsValid()) ? string < other.string : false; }
        };

    public:
        ~BaseLexer() override = default;

        [[nodiscard]] bool operator==(const BaseLexer&) const;

        void SetToken(const TokenReader& token);
        virtual bool Validate(LogCollector& logCollector);

        template<IsLexer Lexer>
        [[nodiscard]] bool IsTypeOf() const noexcept
        {
            // TODO: check for static string
            return _lexerType == Lexer::typeName;
        }

        [[nodiscard]] const String& GetName() const noexcept { return _name; }
        [[nodiscard]] const String& GetLexerType() const noexcept { return _lexerType; }

        [[nodiscard]] bool HasTheSameParent(boost::intrusive_ptr<BaseLexer> parent) const;
        [[nodiscard]] bool HasParent() const noexcept { return !!_parentLexer; }
        [[nodiscard]] const boost::intrusive_ptr<BaseLexer> GetParentLexer() const { return _parentLexer; }
        [[nodiscard]] const std::vector<boost::intrusive_ptr<BaseLexer>>& GetChildLexers() const { return _childLexers; }
        [[nodiscard]] bool HasChildLexers() const noexcept { return !_childLexers.empty(); }

        void TryToSetAsChild(boost::intrusive_ptr<BaseLexer> child);
        void ForceSetAsChild(boost::intrusive_ptr<BaseLexer> child);
        [[nodiscard]] bool IsContainLexer(const BaseLexer* other, bool isInItsScope = false) const;
        [[nodiscard]] std::optional<LineToken> GetOpenScope() const noexcept { return _openScope; }
        [[nodiscard]] std::optional<LineToken> GetCloseScope() const noexcept { return _closeScope; }
        [[nodiscard]] long long GetDistanceToLexer(const BaseLexer* lexer) const noexcept { return Verify(lexer && lexer->GetOpenScope() && _closeScope) ? lexer->_openScope->string - _closeScope->string : 0; }

        void Clear();
        [[nodiscard]] const FileReader* GetFileReader() const { return _reader; }

    protected:
        virtual bool DoValidate(LogCollector& logCollector) = 0;
        virtual bool DoValidateScope(LogCollector& logCollector) { return true; }
        virtual bool DoPostValidate(LogCollector& logCollector) { return true; }

        BaseLexer(const FileReader& reader, const String& type);

    protected:
        TokenReader _token;
        const FileReader* _reader = nullptr;

        std::optional<LineToken> _openScope;
        std::optional<LineToken> _closeScope;

        const String _lexerType;
        String _name;
        boost::intrusive_ptr<BaseLexer> _parentLexer;
        std::vector<boost::intrusive_ptr<BaseLexer>> _childLexers;
    };

}// namespace Ast