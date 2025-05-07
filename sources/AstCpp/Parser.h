//  MIT License
//
//  Copyright (c) 2019-2025 Valerii Koniushenko
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "Ast/Parser.h"
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

    class Parser final : public Ast::Parser
    {
    public:
        template<class T>
        using Container = std::vector<boost::intrusive_ptr<T>>;

    public:
        Parser() = default;
        explicit Parser(ContentStream& stream);
        explicit Parser(ContentStream&& stream);
        ~Parser() override = default;

        void Parse(const ContentStream::Ptr& file) override;

        void IterateOverLexers(std::function<bool(BaseLexer*)>&& callback) override;
        [[nodiscard]] ContentStream::Ptr GetContentStream() const { return _contentStream; }

    protected:
        template<IsLexer Lexer, IsReader ReaderT>
        void ReadAs(Container<Lexer>& container, const ContentStream::Ptr& reader)
        {
            ReaderT readerObject(reader);
            for (auto&& token : readerObject)
            {
                auto lexer = Lexer::Create(reader);
                lexer->SetToken(token);
                if (lexer->Parse())
                {
                    container.push_back(std::move(lexer));
                }
            }
        }

    private:
        void RawParse(const ContentStream::Ptr& file);
        void MakeCorrectDependencies();
        void OnParse();
        BaseLexer* MakeCorrectDependenciesForLexer(BaseLexer* prevLexer);
        BaseLexer* FindNextLexer(const BaseLexer* prevLexer);

    private:
        Container<ClassLexer> _classLexers;
        Container<NamespaceLexer> _namespaceLexers;
        Container<EnumClassLexer> _enumClassLexers;
        ContentStream::Ptr _contentStream;
    };

} // namespace Ast::Cpp