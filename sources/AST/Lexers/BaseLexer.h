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
#include "../Reader/Token.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

namespace Ast
{
    class FileReader;

    class LexerLogCollector : public Utils::CopyableAndMoveable
    {
    public:
        enum class LogType
        {
            None,
            Info,
            Warning,
            Error
        };

        struct LogLine
        {
            String message;
            LogType type = LogType::None;
        };

        using Container = std::vector<LogLine>;

    public:
        LexerLogCollector() = default;
        ~LexerLogCollector() override = default;

        void AddLog(const LogLine& logLine);
        [[nodiscard]] const Container& GetLogs() const noexcept { return _logs; }
        [[nodiscard]] bool IsEmpty() const { return _logs.empty(); }

        template<LogType logType>
        [[nodiscard]] bool HasAny() const
        {
            return std::find_if(_logs.cbegin(), _logs.cend(), [](const LogLine& logLine)
            {
                return logLine.type == logType;
            }) != _logs.cend();
        }

    protected:
        Container _logs;
    };

    class BaseLexer : public Utils::CopyableAndMoveable
    {
    public:
        enum class Type
        {
            None,
            Variable,
            Property,
            Value,
            Class,
            Struct,
            Namespace,
            Enum,
            EnumClass,
        };

        enum class TypeAttribute
        {
            None,
            Pointer,
            Reference,
            Const,
            Constexpr,
            Constinit,
            Consteval,
            Attribute,
            Extern,
            Static
        };

        ~BaseLexer() override = default;

        void SetToken(const TokenReader& token);

    protected:
        BaseLexer(const FileReader& reader, Type type);

    protected:
        TokenReader _token;
        const FileReader* _reader = nullptr;

        Type _type = Type::None;
        TypeAttribute _typeQualifier = TypeAttribute::None;
    };

}// namespace Ast