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
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            logCollector.AddLog({ "EnumClassLexer: Impossible to work with an invalid token", LogCollector::LogType::Error });
            return;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(enum class)|\{)", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({ String::Format("Impossible to parse enum class token at {}", 999), LogCollector::LogType::Error });
            return;
        }

        auto match = string.FindRegex("^\\w+");
        if (Verify(!match.empty(), "Impossible to define an enum class name"))
        {
            _name = match.str();
            _name.ShrinkToFit();
        }
        else
        {
            logCollector.AddLog({ "Impossible to parse class token at {line}" });
            return;
        }

        if (string.RegexReplace(R"(^\w+\s*:)", ""))
        {
            _type = string.Trim(' ');
        }

        logCollector.AddLog({ String::Format("successfull parsing of the enum class: '{}'", _name.CStr()), LogCollector::LogType::Success });
    }

    void EnumClassLexer::ValidateScope(LogCollector& logCollector)
    {
        BaseLexer::ValidateScope(logCollector);

        const auto* openedBracket = _token.endData;
        while(String::Toolset::IsSpace(*openedBracket)) ++openedBracket;
        if (!Verify(*openedBracket == '{', "Impossible to define an enum class scope."))
        {
            logCollector.AddLog({String::Format("Impossible to define an enum class scope '{}'", _name.c_str()), LogCollector::LogType::Error});
            return;
        }

        std::size_t curclyBracketCounter = 1;
        const auto* closedBracket = openedBracket + 1;
        for (; *closedBracket && curclyBracketCounter != 0; ++closedBracket)
        {
            if (*closedBracket == '{')
            {
                ++curclyBracketCounter;
            }
            else if (*closedBracket == '}')
            {
                --curclyBracketCounter;
            }
        }
        --closedBracket;

        if (!Verify(curclyBracketCounter == 0 && closedBracket && *closedBracket != 0, "Can't define enum class scope"))
        {
            logCollector.AddLog({String::Format("Impossible to define an enum class scope '{}'", _name.c_str()), LogCollector::LogType::Error});
            return;
        }

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data(), closedBracket) };

        RecognizeConstants(logCollector);
    }

    void EnumClassLexer::RecognizeConstants(LogCollector& logCollector)
    {
        if (!Verify(_openScope.has_value() && _openScope->IsValid() && _closeScope.has_value() && _closeScope->IsValid()))
        {
            logCollector.AddLog({String::Format("Impossible to get an enum class scope '{}'", _name.c_str()), LogCollector::LogType::Error});
            return;
        }

        String buffer(_openScope->string, _closeScope->string - _openScope->string);
        buffer.Trim('{').Trim('}').RegexReplace(R"(\s*)","");
        for (auto& constant : buffer.Split(","_atom))
        {
            if (auto match = constant.FindRegex(R"(^\w+)"); !match.empty())
            {
                _constants.emplace_back(String(match.str()), std::nullopt);
                _constants.back().name.ShrinkToFit();
            }
        }
    }

} // namespace Ast::Cpp