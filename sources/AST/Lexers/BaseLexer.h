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

namespace Ast
{
    class FileReader;
    class LogCollector;

    class BaseLexer : public Utils::CopyableAndMoveable
    {
    public:
        struct LineToken final
        {
            const String::CharT* string = nullptr;
            std::size_t line = 0;

            [[nodiscard]] bool IsValid() const noexcept { return string && line != 0; }
        };

    public:
        ~BaseLexer() override = default;

        void SetToken(const TokenReader& token);
        virtual void Validate(LogCollector& logCollector) = 0;
        virtual void ValidateScope(LogCollector& logCollector){};

    protected:
        BaseLexer(const FileReader& reader, const String& type);

    protected:
        TokenReader _token;
        const FileReader* _reader = nullptr;

        std::optional<LineToken> _openScope;
        std::optional<LineToken> _closeScope;

        const String _type;
    };

}// namespace Ast