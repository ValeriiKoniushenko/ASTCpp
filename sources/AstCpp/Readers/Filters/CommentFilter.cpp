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

#include "CommentFilter.h"

namespace Ast::Cpp
{

    void CommentFilter::MakeTransform(String& content)
    {
        RemoveSingleLineComments(content);
        RemoveMultiLineComments(content);

        /*std::ofstream file("file.cpp");
        file << content.c_str();
        file.close();*/
    }

    void CommentFilter::RemoveSingleLineComments(String& content)
    {
        // removing '//' comments
        static auto* const regexExpr = R"((?:\/\/(?:\\\n|[^\n])*\n))";
        content.RegexReplace(regexExpr, "\n");
    }

    void CommentFilter::RemoveMultiLineComments(String& content)
    {
        std::size_t offset = 0;

        while (const auto* begin = content.Find("/*"))
        {
            const auto* end = content.Find("*/");

            String endLines = "";
            for (const auto* i = begin; *i && i != end; ++i)
            {
                if (*i == '\n')
                {
                    endLines += '\n';
                }
            }

            static auto* const regexExpr = R"((?:\/\*[\s\S]*?\*\/))";
            content.RegexReplace(regexExpr, endLines, std::regex_constants::match_flag_type::format_first_only);
            offset = end - content.c_str();
        }


    }

} // namespace Ast::Cpp