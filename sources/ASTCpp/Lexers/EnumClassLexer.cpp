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

#include "EnumClassLexer.h"

#include "AST/LogCollector.h"
#include "AST/Readers/FileReader.h"

namespace Ast::Cpp
{

    EnumClassLexer::EnumClassLexer(const FileReader& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    void EnumClassLexer::Validate(LogCollector& logCollector)
    {
        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(enum class)|\{)", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({String::Format("Impossible to parse enum class token at {}", 999), LogCollector::LogType::Error });
            return;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define a enum class name"))
        {
            _name = match.str();
            _name.ShrinkToFit();
        }
        else
        {
            logCollector.AddLog({"Impossible to parse class token at {line}"});
            return;
        }

        if (string.RegexReplace(R"(^\w+\s*:)", ""))
        {
            _type = string.Trim(' ');
        }

        logCollector.AddLog({String::Format("successfull parsing of the enum class: '{}'", _name.CStr()), LogCollector::LogType::Success });
    }

} // namespace Ast::Cpp