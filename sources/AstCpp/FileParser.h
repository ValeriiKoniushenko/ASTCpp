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

#include "Ast/FileParser.h"
#include "Ast/Readers/BaseTokenReader.h"
#include "Lexers/ClassLexer.h"
#include "Lexers/EnumClassLexer.h"
#include "Lexers/NamespaceLexer.h"

#include <vector>

namespace Ast
{
    class ContentStream;
} // namespace Ast

namespace Ast::Cpp
{

    class FileParser final : public Ast::FileParser
    {
    public:
        template<class T>
        using Container = std::vector<boost::intrusive_ptr<T>>;

    public:
        FileParser() = default;
        ~FileParser() override = default;

        bool Parse(const ContentStream::Ptr& file, LogCollector& logCollector) override;
        void IterateOverLexers(std::function<bool(BaseLexer*)>&& callback) override;

    protected:
        template<IsLexer Lexer, IsReader ReaderT>
        static void ReadAs(Container<Lexer>& container, const ContentStream::Ptr& reader, LogCollector& logCollector)
        {
            for (auto&& token : ReaderT(reader))
            {
                auto lexer = Lexer::Create(reader);
                lexer->SetToken(token);
                if (lexer->Validate(logCollector))
                {
                    container.push_back(std::move(lexer));
                }
            }
        }

    private:
        void RawParse(const ContentStream::Ptr& file, LogCollector& logCollector);
        void BindScopes(LogCollector& logCollector);
        BaseLexer* BindScopesForLexer(BaseLexer* prevLexer, LogCollector& logCollector);
        BaseLexer* FindNextLexer(const BaseLexer* prevLexer);

    private:
        Container<ClassLexer> _classLexers;
        Container<NamespaceLexer> _namespaceLexers;
        Container<EnumClassLexer> _enumClassLexers;
    };

} // namespace Ast::Cpp