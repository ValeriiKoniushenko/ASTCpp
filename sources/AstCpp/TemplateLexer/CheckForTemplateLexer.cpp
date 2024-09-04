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

#include "CheckForTemplateLexer.h"

#include "Ast/Lexers/BaseLexer.h"
#include "Ast/Utils/Scopes.h"
#include "Core/Assert.h"

namespace
{
    std::optional<std::pair<const Ast::String::CharT*, Ast::String>> FindTemplate(const Ast::BaseLexer* lexer)
    {
        if (!Verify(lexer))
        {
            return {};
        }

        if (auto scope = lexer->GetTokenReader(); scope.beginData)
        {
            auto end = scope.beginData - 1;
            while (Ast::String::Toolset::IsSpace(*end))
            {
                --end;
            }

            if (auto* src = Ast::Utils::FindClosedBracketR(end, '>', '<'))
            {
                while (Ast::String::IsSpace(*src) || *src == '<')
                {
                    --src;
                }
                while (!Ast::String::IsSpace(*src))
                {
                    --src;
                }
                ++src;

                auto string = Ast::String(src, end - src + 1).Trim(' ');
                if (string.RegexReplace(R"(^template[ ]*)", ""))
                {
                    return { { src, string } };
                }
            }
        }

        return {};
    }
} // namespace

namespace Ast::Cpp
{
    const String::CharT* TryToFindTemplateBegin(const BaseLexer* lexer)
    {
        if (const auto temp = FindTemplate(lexer))
        {
            return temp->first;
        }
        return nullptr;
    }

    String TryToExtrudeTemplate(const BaseLexer* lexer)
    {
        if (const auto temp = FindTemplate(lexer))
        {
            return temp->second;
        }
        return {};
    }

    bool IsTemplate(const BaseLexer* lexer)
    {
        return FindTemplate(lexer).has_value();
    }
} // namespace Ast::Cpp