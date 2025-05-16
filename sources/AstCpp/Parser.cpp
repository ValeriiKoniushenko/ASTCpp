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

#include "AstCpp/Parser.h"

#include "Ast/Tree.h"
#include "Readers/ClassReader.h"
#include "Readers/EnumClassReader.h"
#include "Readers/NamespaceReader.h"

#include <filesystem>

namespace Ast::Cpp
{

    bool Parser::Parse(const ContentStream::Ptr& stream)
    {
        if (stream != _contentStream)
        {
            _contentStream = stream;
        }

        if (!RawParse(stream))
        {
            return false;
        }

        MakeCorrectDependencies();
        OnParse();

        infoLog("Successfully parsed the file: '{}'"_f << _contentStream->GetFilePath());

        return true;
    }

    bool Parser::RawParse(const ContentStream::Ptr& reader)
    {
        if (!ReadAs<NamespaceLexer, NamespaceReader>(_namespaceLexers, reader))
        {
            return false;
        }
        if (!ReadAs<ClassLexer, ClassReader>(_classLexers, reader))
        {
            return false;
        }
        if (!ReadAs<EnumClassLexer, EnumClassReader>(_enumClassLexers, reader))
        {
            return false;
        }

        return true;
    }

    void Parser::MakeCorrectDependencies()
    {
        BaseLexer* lexer = MakeCorrectDependenciesForLexer(nullptr);
        while ((lexer = FindNextLexer(lexer)))
        {
            lexer = MakeCorrectDependenciesForLexer(lexer);
        }
    }

    void Parser::OnParse()
    {
        IterateOverLexers([this](BaseLexer* lexer)
        {
            if (Verify(lexer))
            {
                lexer->ValidateAfterParse();
            }
            return true;
        });
    }

    BaseLexer* Parser::MakeCorrectDependenciesForLexer(BaseLexer* prevLexer)
    {
        const auto startChildsCount = prevLexer ? prevLexer->GetChildLexers().size() : 0;
        IterateOverLexers(
            [&](BaseLexer* lexer)
            {
                if (!lexer || lexer == prevLexer)
                {
                    return true;
                }

                if (const auto scope = lexer->GetOpenScope(); scope && scope->IsValid())
                {
                    if (!prevLexer)
                    {
                        prevLexer = lexer;
                    }
                    else
                    {
                        auto prevLexerName = prevLexer->GetLexerName();
                        auto lexerName = lexer->GetLexerName();

                        if (prevLexer->IsContainLexer(lexer, true))
                        {
                            MakeCorrectDependenciesForLexer(lexer);
                            if (!lexer->HasParent())
                            {
                                prevLexer->TryToSetAsChild(lexer);
                            }
                        }
                    }
                }

                return true;
            });

        if (!prevLexer || prevLexer->GetChildLexers().size() == startChildsCount)
        {
            return nullptr;
        }

        return prevLexer;
    }

    BaseLexer* Parser::FindNextLexer(const BaseLexer* prevLexer)
    {
        if (!prevLexer)
        {
            return nullptr;
        }

        BaseLexer* nearestLexer = nullptr;

        // find any first lexer after 'prevLexer'
        IterateOverLexers(
            [&](BaseLexer* lexer)
            {
                if (!Verify(lexer) || *lexer == *prevLexer)
                {
                    return true;
                }

                const auto distance = prevLexer->GetDistanceToLexer(lexer);
                if (!prevLexer->IsContainLexer(lexer) && distance > 0)
                {
                    nearestLexer = lexer;
                    return false;
                }
                return true;
            });

        if (!nearestLexer)
        {
            return nullptr;
        }

        IterateOverLexers(
            [&](BaseLexer* lexer)
            {
                if (!lexer)
                {
                    return true;
                }

                if (prevLexer->IsContainLexer(lexer) || *prevLexer == *lexer)
                {
                    return true;
                }

                const auto d1 = prevLexer->GetDistanceToLexer(lexer);
                const auto d2 = prevLexer->GetDistanceToLexer(nearestLexer);
                if (d1 < d2)
                {
                    nearestLexer = lexer;
                }

                return true;
            });

        return nearestLexer;
    }

    void Parser::IterateOverLexers(std::function<bool(BaseLexer*)>&& callback)
    {
        if (!callback)
        {
            return;
        }

        for (auto&& lexer : _classLexers)
        {
            if (!callback(lexer.get()))
            {
                return;
            }
        }
        for (auto&& lexer : _namespaceLexers)
        {
            if (!callback(lexer.get()))
            {
                return;
            }
        }
        for (auto&& lexer : _enumClassLexers)
        {
            if (!callback(lexer.get()))
            {
                return;
            }
        }
    }

} // namespace Ast::Cpp
