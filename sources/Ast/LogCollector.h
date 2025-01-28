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

#include "CommonTypes.h"
#include "Core/Delegate.h"
#include "Utils/CopyableAndMoveableBehaviour.h"
#include "Core/Enum.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include <algorithm>

namespace Ast
{

    class LogCollector final : public boost::intrusive_ref_counter<LogCollector>, public virtual ::Utils::CopyableAndMoveable
    {
    public:
        AST_CLASS(LogCollector)

        CreateEnum(LogType, int,
            None,
            Info,
            Warning,
            Error,
            Success
        );

        struct LogLine final
        {
            String message;
            LogType type = LogType::None;

            LogLine(String message, LogType type) : message{std::move(message)}, type(type) {}
        };

        using Container = std::vector<LogLine>;

    public:
        LogCollector() = default;
        ~LogCollector() override = default;

        void AddLog(const LogLine& logLine);
        [[nodiscard]] const Container& GetLogs() const noexcept { return _logs; }
        [[nodiscard]] bool IsEmpty() const { return _logs.empty(); }

        [[nodiscard]] static Ptr Create() { return new Self; }

        void ClearLogs() { _logs.clear(); }

        template<int logType>
        [[nodiscard]] bool HasAny() const
        {
            return std::find_if(_logs.cbegin(), _logs.cend(),
                                [](const LogLine& logLine)
                                {
                                    return logLine.type.Cast() == logType;
                                }) != _logs.cend();
        }

        template<int logType>
        [[nodiscard]] Container GetFilteredLogs() const
        {
            Container temp;
            std::copy_if(_logs.cbegin(), _logs.cend(), std::back_inserter(temp),
                         [](const LogLine& logLine)
                         {
                             return logLine.type.Cast() == logType;
                         });
            return temp;
        }

        Core::Delegate<void(const LogLine&)> onValidationEvent;

    private:
        Container _logs;
    };

    std::ostream& operator<<(std::ostream& os, const LogCollector::LogLine& line);

} // namespace Ast
