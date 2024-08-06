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

#include "ClassReader.h"

#include "FileReader.h"
#include "RegexTokenReaderImpl.h"

namespace Ast
{

    /*std::optional<TokenReader> ClassReader::FindNextToken()
    {
        if (!Verify(_fileReader))
        {
            return {};
        }

        auto tempToken = _lastToken;

        if (!tempToken.IsValid())
        {
            tempToken.beginData = _fileReader->Data().c_str();
            tempToken.endData = _fileReader->Data().c_str() + _fileReader->Data().Size() - 1ull;
        }

        if (!Verify(tempToken.IsValid()))
        {
            return {};
        }

        auto offset = 0;
        if (tempToken.endData != _fileReader->Data().c_str() + _fileReader->Data().Size() - 1ull)
        {
            offset = static_cast<int>(tempToken.endData - _fileReader->Data().c_str());
        }
        bool wasFoundAtLeastOneToken = false;

        _fileReader->Data().IterateRegex(regexNamespace, offset, [&](const Core::StringAtom::StdRegexMatchResults& match)
        {
            wasFoundAtLeastOneToken = true;
            tempToken.beginData = _fileReader->Data().c_str() + match.position();
            tempToken.endData = _fileReader->Data().c_str() + match.position() + match.length();
            return false;
        });

        if (!wasFoundAtLeastOneToken)
        {
            return std::nullopt;
        }

        return std::make_optional(_lastToken = tempToken);
    }*/

} // namespace Ast