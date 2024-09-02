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

#include "Core/Delegate.h"
#include "Utils/CopyableAndMoveableBehaviour.h"
#include "CommonTypes.h"

namespace Ast
{

    class LogCollector final : public Utils::CopyableAndMoveable
    {
    public:
        enum class LogType
        {
            None,
            Info,
            Warning,
            Error,
            Success
        };

        struct LogLine final
        {
            String message;
            LogType type = LogType::None;
        };

        using Container = std::vector<LogLine>;

    public:
        LogCollector() = default;
        ~LogCollector() override = default;

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

        template<LogType logType>
        [[nodiscard]] Container GetFilteredLogs() const
        {
            Container temp;
            std::copy_if(_logs.cbegin(), _logs.cend(), [&temp](const LogLine& logLine)
            {
                return logLine.type == logType;
            });
            return temp;
        }

        Core::Delegate<void(const String&, LogType)> onValidationEvent;

    private:
        Container _logs;
    };

} // namespace Ast
