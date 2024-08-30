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

#include "NamespaceLexer.h"

#include "AST/LogCollector.h"
#include "AST/Readers/Reader.h"
#include "AST/Utils/Scopes.h"

namespace Ast::Cpp
{

    NamespaceLexer::NamespaceLexer(const Reader::Ptr&fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool NamespaceLexer::DoValidate(LogCollector& logCollector)
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            logCollector.AddLog({ "NamespaceLexer: Impossible to work with an invalid token", LogCollector::LogType::Error });
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.RegexReplace(R"(\n|\r|(namespace))", " ");
        string.Trim(' ');
        if (string.IsEmpty())
        {
            logCollector.AddLog({ String::Format("Impossible to parse namespace token at {}", _token.startLine), LogCollector::LogType::Error });
            return false;
        }

        _lexerName = string; // absolute name

        for (auto&& name : string.Split("::"_atom))
        {
            _nameList.push_back(std::move(name));
        }

        return true;
    }

    bool NamespaceLexer::DoValidateScope(LogCollector& logCollector)
    {
        if (!BaseLexer::DoValidateScope(logCollector))
        {
            return false;
        }

        const auto* openedBracket = _token.endData;
        while(String::Toolset::IsSpace(*openedBracket)) ++openedBracket;
        if (!Verify(*openedBracket == '{', "Impossible to define a namespace scope."))
        {
            logCollector.AddLog({String::Format("Impossible to define a namespace scope '{}'", _lexerName.c_str()), LogCollector::LogType::Error});
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data(), closedBracket) };

        return true;
    }

} // namespace Ast::Cpp