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

#include "ClassLexer.h"

#include "AST/LogCollector.h"
#include "AST/Readers/FileReader.h"

namespace Ast::Cpp
{

    ClassLexer::ClassLexer(const FileReader& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    void ClassLexer::Validate(LogCollector& logCollector)
    {
        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(class)|\{)", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({String::Format("Impossible to parse class token at {}", 999), LogCollector::LogType::Error });
            return;
        }

        if (string.RegexReplace("\\sfinal", ""))
        {
            _hasFinal = true;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define a class name"))
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
            for (auto& parentStr : string.Split(","_atom))
            {
                InheritanceType type = InheritanceType::Private;
                if (parentStr.RegexReplace(R"(\s*public\s*)", ""))
                {
                    type = InheritanceType::Public;
                }
                else if (parentStr.RegexReplace(R"(\s*protected\s*)", ""))
                {
                    type = InheritanceType::Protected;
                }
                else
                {
                    parentStr.RegexReplace(R"(\s*private\s*)", "");
                    type = InheritanceType::Private;
                }

                parentStr.Trim(' ');
                parentStr.ShrinkToFit();
                _parents.emplace_back(type, std::move(parentStr));
            }
        }

        logCollector.AddLog({String::Format("successfull parsing of the class: '{}'", _name.CStr()), LogCollector::LogType::Success });
    }

} // namespace Ast::Cpp