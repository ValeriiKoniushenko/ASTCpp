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

#include "RegexTokenReaderImpl.h"

#include "Reader.h"
#include "BaseTokenReader.h"

namespace Ast
{

    std::optional<TokenReader> RegexTokenReaderImpl::FindNextToken() const
    {
        if (!Verify(_baseTokenReader))
        {
            return std::nullopt;
        }

        if (!Verify(_baseTokenReader->GetReader()))
        {
            return std::nullopt;
        }

        const auto& data = _baseTokenReader->GetReader()->Data();

        auto tempToken = _baseTokenReader->GetLastToken();

        if (!tempToken.IsValid())
        {
            tempToken.beginData = data.c_str();
            tempToken.endData = data.c_str() + data.Size() - 1ull;
        }

        if (!Verify(tempToken.IsValid()))
        {
            return std::nullopt;
        }

        std::size_t offset = 0;
        if (tempToken.endData != data.c_str() + data.Size() - 1ull)
        {
            offset = static_cast<std::size_t>(tempToken.endData - data.c_str());
        }
        bool wasFoundAtLeastOneToken = false;

        data.IterateRegex(
            _regexExpr,
            [&](const String::StdRegexMatchResults& match)
            {
                wasFoundAtLeastOneToken = true;
                auto s = match[0];

                tempToken.beginData = data.c_str() + (match[0].first - data.begin());
                while (String::Toolset::IsSpace(*tempToken.beginData)) ++tempToken.beginData;

                tempToken.endData = data.c_str() + (match[0].second - data.begin());

                tempToken.startLine = String::GetLinesCountInText(data, tempToken.beginData);
                tempToken.endLine = String::GetLinesCountInText(data, tempToken.endData) - 1; // 1 - to ignore the last '\n'

                return false;
            },
            offset);

        if (!wasFoundAtLeastOneToken)
        {
            return std::nullopt;
        }

        _baseTokenReader->SetLastToken(tempToken);

        return std::make_optional(tempToken);
    }

} // namespace Ast