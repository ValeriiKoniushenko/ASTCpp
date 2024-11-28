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

#include "BaseLexer.h"
#include "Core/Math.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#ifdef Ast_LexerIterator_TODO

namespace Ast
{
    template<IsLexerOrBase Lexer = BaseLexer>
    class LexerIterator : public Core::IRandomAccessIterator<Lexer, LexerIterator<Lexer>, Utils::CopyableAndMoveable>
    {
    public:
        using DataRefT = typename Core::IRandomAccessIterator<Lexer, LexerIterator<Lexer>, Utils::CopyableAndMoveable>::DataRefT;

    public:
        explicit LexerIterator(Lexer& lexer, const int32_t maxDeep = 0)
            : _lexer{ lexer },
              _maxDeep{ maxDeep }
        {
        }

        [[nodiscard]] bool operator==(const LexerIterator& other) const noexcept override { return _lexer.get() == other._lexer.get(); }

        [[nodiscard]] bool operator!=(const LexerIterator& other) const noexcept override { return !operator==(other); }

        [[nodiscard]] const DataRefT operator*() const noexcept override
        {
            if (IsValid() && _lexer->HasChildLexers() && _currentLexer < _lexer->GetChildLexers().size())
            {
                return _lexer->GetChildLexers()->at(_currentLexer);
            }
            return *this;
        }

        [[nodiscard]] const DataRefT operator->() const override { return operator*(); }

        [[nodiscard]] DataRefT operator*() noexcept override
        {
            if (IsValid() && _lexer->HasChildLexers() && _currentLexer < _lexer->GetChildLexers().size())
            {
                return _lexer->GetChildLexers()->at(_currentLexer);
            }
            return {};
        }

        [[nodiscard]] DataRefT operator->() noexcept override { return operator*(); }

        LexerIterator& operator++() noexcept override
        {
            if (IsValid())
            {
                ++_currentLexer;
            }
            return *this;
        }

        LexerIterator operator++(int) noexcept override
        {
            auto tmp = *this;
            if (IsValid())
            {
                ++_currentLexer;
            }
            return tmp;
        }

        LexerIterator& operator--() noexcept override
        {
            if (IsValid())
            {
                --_currentLexer;
            }
            return *this;
        }

        LexerIterator operator--(int) noexcept override
        {
            auto tmp = *this;
            if (IsValid())
            {
                --_currentLexer;
            }
            return tmp;
        }

        LexerIterator& operator+=(int step) noexcept override
        {
            if (!IsValid())
            {
                return *this;
            }

            if (_currentLexer + step >= _lexer->GetChildLexers().size())
            {
                const auto maxSize = IsValid() ? _lexer->GetChildLexers().size() : -1;
                const String lexerName = IsValid() && _lexer->IsValid() ? _lexer->GetLexerName() : "";
                Assert(
                    false,
                    String::Format("Impossible to move the LexerIterator's index for {}. Current index: {}; max size: {}. Lexer name: '{}'", step, _currentLexer, maxSize).c_str(), lexerName);
            }
            _currentLexer += step;

            return *this;
        }

        LexerIterator& operator-=(int step) noexcept override
        {
            if (!IsValid())
            {
                return *this;
            }

            step = Math::Abs(step) * -1;

            if (_currentLexer + step < 0)
            {
                const auto maxSize = IsValid() ? _lexer->GetChildLexers().size() : -1;
                const String lexerName = IsValid() && _lexer->IsValid() ? _lexer->GetLexerName() : "";
                Assert(
                    false,
                    String::Format("Impossible to move the LexerIterator's index for {}. Current index: {}; max size: {}. Lexer name: '{}'", step, _currentLexer, maxSize).c_str(), lexerName);
            }
            _currentLexer += step;

            return *this;
        }

        LexerIterator operator+(int step) const noexcept override
        {
            auto tmp = *this;
            tmp += step;
            return tmp;
        }

        LexerIterator operator-(int step) const noexcept override
        {
            auto tmp = *this;
            tmp -= step;
            return tmp;
        }

        [[nodiscard]] bool operator>(const LexerIterator& other) const noexcept override
        {
            return _currentLexer > other._currentLexer;
        }

        [[nodiscard]] bool operator>=(const LexerIterator& other) const noexcept override
        {
            return _currentLexer >= other._currentLexer;
        }

        [[nodiscard]] bool operator<(const LexerIterator& other) const noexcept override
        {
            return _currentLexer < other._currentLexer;
        }

        [[nodiscard]] bool operator<=(const LexerIterator& other) const noexcept override
        {
            return _currentLexer <= other._currentLexer;
        }

        void Swap(LexerIterator& other) override
        {
            std::swap(*this, other);
        }

        [[nodiscard]] bool IsValid() const noexcept { return _lexer; }

    private:
        typename Lexer::Ptr _lexer;
        int32_t _maxDeep = 0;
        int32_t _currentLexer = 0;
    };

} // namespace Ast

#endif