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

#include "Lexers/FileLexer.h"
#include "Parser.h"
#include "Readers/ContentStream.h"
#include "Utils/CopyableAndMoveableBehaviour.h"
#include "Utils/ITextSourceReader.h"

namespace Ast
{

    template<IsLexerOrBase RootLexerT = FileLexer>
    class Tree : public virtual ::Utils::CopyableAndMoveable, public ITextSourceReader, public boost::intrusive_ref_counter<Tree<RootLexerT>>
    {
    public:
        AST_CLASS(Tree)

        struct Params
        {
            int nesting = 0;
        };

        template<bool IsConst = false>
        using ForEachFunctionT = std::function<bool(std::conditional_t<IsConst, const BaseLexer*, BaseLexer*>, Params)>;

        template<bool IsConst = false>
        using FindFunctionT = std::function<bool(std::conditional_t<IsConst, const BaseLexer*, BaseLexer*>)>;

    public:
        explicit Tree(const ContentStream::Ptr& reader)
            : _rootLexer{ RootLexerT::Create(reader) },
              _contentStream{ reader }
        {
        }

        explicit Tree(const BaseLexer::Ptr& lexer)
            : _rootLexer{ lexer },
              _contentStream{ lexer->GetReader() }
        {
        }

        ~Tree() override = default;

        void FlushToStream()
        {
            auto source = GetTextSource();
            _contentStream->Put(source);
        }

        template<IsParser ParserT>
        [[nodiscard]] static Tree<RootLexerT> From(const ParserT& parser, LogCollector::Ptr logCollector = nullptr)
        {
            Tree<RootLexerT> tree(parser.GetContentStream());
            tree.template ParseUsing<ParserT>(logCollector ? logCollector : new LogCollector());
            return tree;
        }

        template<IsParser ParserT>
        void ParseUsing(LogCollector::Ptr logCollector)
        {
            if (!Verify(!!_contentStream, "File reader was nullptr"))
            {
                logCollector->AddLog({ "File reader was nullptr", LogCollector::LogType::Error });
                return;
            }

            ParserT parser;
            parser.Parse(_contentStream, logCollector);

            parser.IterateOverLexers(
                [&](BaseLexer* lexer)
                {
                    if (!Verify(lexer, "Some lexer was nullptr but expected a valid object."))
                    {
                        logCollector->AddLog({ "Some lexer was nullptr but expected a valid object.", LogCollector::LogType::Error });
                        return true;
                    }

                    if (!lexer->HasParent())
                    {
                        _rootLexer->ForceSetAsChild(lexer);
                    }

                    return true;
                });

            if (_rootLexer->IsTypeOf<FileLexer>())
            {
                _rootLexer->SetLexerName(_contentStream->GetFilePath());
            }
            _rootLexer->DoParse(*logCollector);
        }

        [[nodiscard]] ContentStream::Ptr GetReader() const { return _contentStream; }

        [[nodiscard]] String GetTextSource() override { return _rootLexer->GetTextSource(); }

        // ===========================================================
        // ================== WORKING WITH LEXERS ====================
        // ===========================================================
        /**
         * @brief Can take a functions of next types:
         * 1. bool([const] Lexer*, Param) - this function will work until it gets 'false' in return
         * 2. void([const] Lexer*, Param) - will iterate without stopping through all a tree
         */
        template<IsLexer Lexer = void, class FuncT>
        void ForEach(FuncT&& callback)
        {
            Params params;
            ForEachImpl<FuncT, Lexer, false>(std::forward<decltype(callback)>(callback), _rootLexer.get(), params);
        }

         /**
         * @brief Can take a functions of next types:
         * 1. bool(const Lexer*, Param) - this function will work until it gets 'false' in return
         * 2. void(const Lexer*, Param) - will iterate without stopping through all a tree
         */
        template<IsLexer Lexer = void, class FuncT>
        void ForEach(FuncT&& callback) const
        {
            Params params;
            ForEachImpl<FuncT, Lexer, true>(std::forward<decltype(callback)>(callback), _rootLexer.get(), params);
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::Ptr FindFirstByName(const String& lexerName)
        {
            return FindFirstByNameImpl<Lexer>(this, lexerName);
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::CPtr FindFirstByName(const String& lexerName) const
        {
            return FindFirstByNameImpl<Lexer, true>(this, lexerName);
        }

        template<IsLexer Lexer>
        [[nodiscard]] typename Lexer::Ptr FindFirstByNameAs(const String& lexerName)
        {
            return boost::dynamic_pointer_cast<Lexer>(FindFirstByNameImpl<Lexer>(this, lexerName));
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] typename Lexer::CPtr FindFirstByNameAs(const String& lexerName) const
        {
            return boost::dynamic_pointer_cast<const Lexer>(FindFirstByNameImpl<Lexer, true>(this, lexerName));
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::Ptr FindIf(FindFunctionT<false>&& callback)
        {
            return FindIfImpl<Lexer>(this, std::forward<decltype(callback)>(callback));
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::CPtr FindIf(FindFunctionT<true>&& callback) const
        {
            return FindIfImpl<Lexer, true>(this, std::forward<decltype(callback)>(callback));
        }

        template<IsLexer Lexer>
        [[nodiscard]] BaseLexer::Ptr FindIfAs(FindFunctionT<false>&& callback)
        {
            return boost::dynamic_pointer_cast<Lexer>(FindIfImpl<Lexer>(this, std::forward<decltype(callback)>(callback)));
        }

        template<IsLexer Lexer>
        [[nodiscard]] BaseLexer::CPtr FindIfAs(FindFunctionT<true>&& callback) const
        {
            return boost::dynamic_pointer_cast<const Lexer>(FindIfImpl<Lexer, true>(this, std::forward<decltype(callback)>(callback)));
        }

        [[nodiscard]] BaseLexer::Ptr GetRootLexer() { return _rootLexer; }
        [[nodiscard]] BaseLexer::CPtr GetRootLexer() const { return _rootLexer; }

    private:
        // ======================= PIMPLs =======================

        template<class FuncT, IsLexer Lexer = void, bool IsConst = false>
        static bool ForEachImpl(FuncT&& callback, BaseLexer::AdaptiveRawPtr<IsConst> base, Params& params)
        {
            if (!base)
            {
                return false;
            }

            bool isNeedToInvoke = false;
            if constexpr (std::is_void_v<Lexer>)
            {
                isNeedToInvoke = true;
            }
            else
            {
                if (Lexer::typeName == base->GetLexerType())
                {
                    isNeedToInvoke = true;
                }
            }

            if (isNeedToInvoke)
            {
                if constexpr (std::is_invocable_v<FuncT, decltype(base), decltype(params)>)
                {
                    if constexpr (std::is_void_v<decltype(callback(base, params))>)
                    {
                        std::invoke(std::forward<decltype(callback)>(callback), base, params);
                    }
                    else
                    {
                        if (!std::invoke(std::forward<decltype(callback)>(callback), base, params))
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    if constexpr (std::is_void_v<decltype(callback(base))>)
                    {
                        std::invoke(std::forward<decltype(callback)>(callback), base);
                    }
                    else
                    {
                        if (!std::invoke(std::forward<decltype(callback)>(callback), base))
                        {
                            return false;
                        }
                    }
                }
            }

            if (base->HasChildLexers())
            {
                ++params.nesting;
                for (auto& child : base->GetChildLexers())
                {
                    if (child)
                    {
                        ForEachImpl<FuncT, Lexer, IsConst>(std::forward<decltype(callback)>(callback), child.get(), params);
                    }
                }
                --params.nesting;
            }

            return true;
        }

        template<IsLexer Lexer = void, bool IsConst = false>
        [[nodiscard]] static BaseLexer::AdaptivePtr<IsConst> FindIfImpl(AdaptiveRawPtr<IsConst> fileTree, FindFunctionT<IsConst>&& callback)
        {
            if (!callback)
            {
                return {};
            }

            BaseLexer::AdaptivePtr<IsConst> ret;
            fileTree->template ForEach<Lexer>(
                [&callback, &ret](BaseLexer::AdaptiveRawPtr<IsConst> lexer, auto)
                {
                    if (callback(lexer))
                    {
                        ret = lexer;
                        return false;
                    }
                    return true;
                });

            return ret;
        }

        template<IsLexer Lexer = void, bool IsConst = false>
        [[nodiscard]] static BaseLexer::AdaptivePtr<IsConst> FindFirstByNameImpl(AdaptiveRawPtr<IsConst> fileTree, const String& lexerName)
        {
            return FindIfImpl<Lexer, IsConst>(fileTree,
                                              [&lexerName](const BaseLexer* lexer)
                                              {
                                                  if (lexer->GetLexerName() == lexerName)
                                                  {
                                                      return true;
                                                  }
                                                  return false;
                                              });
        }

    private:
        BaseLexer::Ptr _rootLexer;
        ContentStream::Ptr _contentStream;
    };

    using BaseTree = Tree<FileLexer>;

} // namespace Ast