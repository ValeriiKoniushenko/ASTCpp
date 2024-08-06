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

#include "BaseTokenReader.h"

namespace Ast
{

    bool BaseTokenReader::Iterator::operator==(const Iterator& other) const noexcept
    {
        return _baseTokenReader == other._baseTokenReader && _token.beginData == other._token.beginData;
    }

    void BaseTokenReader::Iterator::Swap(Iterator& other)
    {
        auto temp = *this;
        _token = other._token;
        _baseTokenReader = other._baseTokenReader;

        other._token = temp._token;
        other._baseTokenReader = temp._baseTokenReader;
    }

    BaseTokenReader::Iterator& BaseTokenReader::Iterator::operator++() noexcept
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

    BaseTokenReader::Iterator BaseTokenReader::Iterator::operator++(int) noexcept
    {
        auto temp = *this;
        ++temp;
        return *this;
    }

    std::optional<TokenReader> BaseTokenReader::FindNextToken() const
    {
        if (Verify(_tokenReaderImpl))
        {
            return _tokenReaderImpl->FindNextToken();
        }

        return std::nullopt;
    }

} // namespace Ast