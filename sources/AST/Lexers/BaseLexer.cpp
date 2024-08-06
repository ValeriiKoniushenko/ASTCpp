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

#include "BaseLexer.h"

#include "../Readers/FileReader.h"
#include "Core/Assert.h"

namespace Ast
{

    void LexerLogCollector::AddLog(const LogLine& logLine)
    {
        if (Verify(logLine.type != LogType::None, "Was passed LogType::None but expected NOT LogType::None") &&
            Verify(!logLine.message.IsEmpty(), "Was passed an empty message to the log"))
        {
            _logs.emplace_back(logLine);
        }
    }

    void BaseLexer::SetToken(const TokenReader& token)
    {
        _token = token;
    }

    BaseLexer::BaseLexer(const FileReader& reader, const String& type)
        : _reader{ &reader },
          _type{ type }
    {
        Assert(_reader);
        Assert(!_type.IsEmpty());
    }

} // namespace Ast