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

#include "spdlog/spdlog.h"
#include "Ast/Readers/ContentStream.h"
#include "Ast/Utils/Scopes.h"

namespace Ast::Cpp
{

    void NamespaceLexer::SetNamespace(const String& nameList)
    {
        for (auto&& name : nameList.split("::"_atom))
        {
            _nameList.push_back(std::move(name));
        }

        SetLexerName(nameList);
    }

    bool NamespaceLexer::GenerateTextSource(TextSourceT& source)
    {
        if (!ITextSourceReader::GenerateTextSource(source))
        {
            return false;
        }

        uint32_t pos = 0;

        if (auto i = source.carets.find("write-point"_atom); i != source.carets.end())
        {
            pos = i->second;
        }

        String namespaceSource = "namespace " + GetLexerName() + Code::Endl() + "{" + Code::Endl();

        for (auto& child : _childLexers)
        {
            source.carets["write-point"_atom] = source.source.size();
            namespaceSource += child->GetTextSource() + Code::Endl();
        }

        namespaceSource += "}" + Code::Endl();

        source.source.insert(pos, namespaceSource.c_str());
        source.carets["write-point"_atom] = source.source.size();

        return true;
    }

    NamespaceLexer::NamespaceLexer(const ContentStream::Ptr& fileReader)
        : BaseLexer(fileReader, typeName)
    {
    }

    bool NamespaceLexer::DoParse()
    {
        if (!Verify(_token.IsValid(), "Impossible to work with an invalid token"))
        {
            spdlog::error("NamespaceLexer: Impossible to work with an invalid token");
            return false;
        }

        String string(_token.beginData, _token.endData - _token.beginData);
        string.regexReplace(R"(\n|\r|(namespace))", " ");
        string.trim(' ');
        if (string.isEmpty())
        {
            spdlog::error(("Impossible to parse namespace token at {}"_f << _token.startLine).toStdStringView());
            return false;
        }

        _lexerName = string; // absolute name

        for (auto&& name : string.split("::"_atom))
        {
            _nameList.push_back(std::move(name));
        }

        return true;
    }

    bool NamespaceLexer::DoScopeParse()
    {
        if (!BaseLexer::DoScopeParse())
        {
            return false;
        }

        const auto* openedBracket = _token.endData;
        while (String::Toolset::IsSpace(*openedBracket))
        {
            ++openedBracket;
        }
        if (!Verify(*openedBracket == '{', "Impossible to define a namespace scope."))
        {
            spdlog::error(("Impossible to define a namespace scope '{}'"_f << _lexerName.c_str()).toStdStringView());
            return false;
        }

        const auto* closedBracket = Utils::FindClosedBracket(openedBracket, '}', '{');

        _openScope = { openedBracket, String::GetLinesCountInText(_reader->Data().c_str(), openedBracket) };
        _closeScope = { closedBracket, String::GetLinesCountInText(_reader->Data().c_str(), closedBracket) };

        return true;
    }

} // namespace Ast::Cpp