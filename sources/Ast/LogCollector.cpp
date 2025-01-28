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

#include "LogCollector.h"

using namespace std::chrono;

namespace Ast
{

    String LogCollector::LogLine::GetHumanTime() const
    {
        auto timeTValue = system_clock::to_time_t(system_clock::time_point{timestamp});

        std::tm tm;
        localtime_s(&tm, &timeTValue);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        oss << '.' << std::setfill('0') << std::setw(3) << timestamp.count() % 1000;

        return String{oss.str()};

    }

    LogCollector::LogLine::LogLine(String message, LogType type)
        : message{ std::move(message) },
          type(type)
    {
        timestamp = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
        );


    }

    void LogCollector::AddLog(const LogLine& logLine)
    {
        if (Verify(logLine.type.Cast() != LogType::None, "Was passed LogType::None but expected NOT LogType::None") &&
            Verify(!logLine.message.IsEmpty(), "Was passed an empty message to the log"))
        {
            _logs.emplace_back(logLine);
            onValidationEvent.Trigger(logLine);
        }
    }

    std::ostream& operator<<(std::ostream& os, const LogCollector::LogLine& line)
    {
        return os << line.type.ToStr() << ": " << line.message.c_str();
    }

} // namespace Ast
