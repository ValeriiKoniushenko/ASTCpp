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
#include "Token.h"
#include "BaseTokenReaderImpl.h"
#include "boost/smart_ptr/intrusive_ptr.hpp"

#include <type_traits>

namespace Ast
{
    class FileReader;

    template<class T>
    concept IsLexer = std::is_base_of_v<BaseLexer, T>;

    class BaseTokenReader : public Utils::CopyableAndMoveable
    {
    public:
        BaseTokenReader(const FileReader& fileReader, boost::intrusive_ptr<BaseTokenReaderImpl> tokenReaderImpl)
            : _fileReader{ &fileReader },
              _tokenReaderImpl{ std::move(tokenReaderImpl) }
        {
        }

        ~BaseTokenReader() override = default;

        class Iterator final : public Core::IForwardIterator<TokenReader, Iterator, Utils::CopyableAndMoveable, true>
        {
        public:
            Iterator() = default;
            ~Iterator() override = default;

            [[nodiscard]] bool operator==(const Iterator& other) const noexcept override;

            [[nodiscard]] bool operator!=(const Iterator& other) const noexcept override { return !(*this == other); }

            [[nodiscard]] const TokenReader operator*() const noexcept override { return _token; }

            [[nodiscard]] const TokenReader operator->() const override { return _token; }

            void Swap(Iterator& other) override;

            [[nodiscard]] TokenReader operator*() noexcept override { return _token; }

            [[nodiscard]] TokenReader operator->() noexcept override { return _token; }

            Iterator& operator++() noexcept override;

            Iterator operator++(int) noexcept override;

        protected:
            explicit Iterator(BaseTokenReader& baseTokenReader)
                : _baseTokenReader{ &baseTokenReader }
            {
            }

        protected:
            TokenReader _token;

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

        [[nodiscard]] const FileReader* GetFileReader() const noexcept { return _fileReader; }
        [[nodiscard]] const TokenReader& GetLastToken() const noexcept { return _lastToken; }
        void SetLastToken(const TokenReader& lastToken) noexcept { _lastToken = lastToken; }

    protected:
        [[nodiscard]] virtual std::optional<TokenReader> FindNextToken() const;

    protected:
        const FileReader* _fileReader = nullptr;
        TokenReader _lastToken;
        boost::intrusive_ptr<BaseTokenReaderImpl> _tokenReaderImpl = nullptr;

        friend class Iterator;
    };

    template<class T>
    concept IsReader = std::is_base_of_v<BaseTokenReader, T>;

} // namespace Ast