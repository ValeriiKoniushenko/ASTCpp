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

#include "../Lexers/BaseLexer.h"
#include "Core/AbstractIterators.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <type_traits>

namespace Ast
{
    class FileReader;

    template<class T>
    concept IsLexer = std::is_base_of_v<BaseLexer, T>;

    template<IsLexer LexerType>
    class BaseTokenReader : public Utils::CopyableAndMoveable
    {
    public:
        using LexerT = LexerType;

    public:
        explicit BaseTokenReader(const FileReader& fileReader)
            : _fileReader{ &fileReader }
        {
        }

        ~BaseTokenReader() override = default;

        struct Token : Utils::CopyableAndMoveable
        {
            const String::CharT* beginData = nullptr;
            const String::CharT* endData = nullptr;

            [[nodiscard]] bool IsValid() const noexcept { return beginData != nullptr && endData != nullptr; }
        };

        class Iterator final : public Core::IForwardIterator<Token, Iterator, Utils::CopyableAndMoveable, true>
        {
        public:
            Iterator() = default;
            ~Iterator() override = default;

            [[nodiscard]] bool operator==(const Iterator& other) const noexcept override
            {
                return _baseTokenReader == other._baseTokenReader && _token.beginData == other._token.beginData;
            }

            [[nodiscard]] bool operator!=(const Iterator& other) const noexcept override { return !(*this == other); }

            [[nodiscard]] const Token operator*() const noexcept override { return _token; }

            [[nodiscard]] const Token operator->() const override { return _token; }

            void Swap(Iterator& other) override
            {
                auto temp = *this;
                _token = other._token;
                _baseTokenReader = other._baseTokenReader;

                other._token = temp._token;
                other._baseTokenReader = temp._baseTokenReader;
            }

            [[nodiscard]] Token operator*() noexcept override { return _token; }

            [[nodiscard]] Token operator->() noexcept override { return _token; }

            Iterator& operator++() noexcept override
            {
                if (Verify(_baseTokenReader))
                {
                    if (auto token = _baseTokenReader->FindNextToken())
                    {
                        _token = *token;
                    }
                    else
                    {
                        *this = Iterator();
                    }
                }

                return *this;
            }

            Iterator operator++(int) noexcept override
            {
                auto temp = *this;
                ++temp;
                return *this;
            }

        protected:
            explicit Iterator(BaseTokenReader& baseTokenReader)
                : _baseTokenReader{ &baseTokenReader }
            {
            }

        protected:
            Token _token;

        private:
            BaseTokenReader* _baseTokenReader = nullptr;

        private:
            friend class BaseTokenReader;
        };

        Iterator begin()
        {
            Iterator temp{ *this };
            ++temp;
            return temp;
        }

        Iterator end()
        {
            return Iterator{};
        }

    protected:
        [[nodiscard]] virtual std::optional<Token> FindNextToken() = 0;

    protected:
        const FileReader* _fileReader = nullptr;
        Token _token;

        friend class Iterator;
    };

} // namespace Ast