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

#include "FileParser.h"
#include "Lexers/FileLexer.h"
#include "Readers/Reader.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

namespace Ast
{

    class ASTFileTree : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<ASTFileTree>
    {
    public:
        AST_CLASS(ASTFileTree)

        struct Params
        {
            int nesting = 0;
        };

        template<bool IsConst = false>
        using ForEachFunctionT = std::function<bool(std::conditional_t<IsConst, const BaseLexer*, BaseLexer*>, Params)>;

        template<bool IsConst = false>
        using FindFunctionT = std::function<bool(std::conditional_t<IsConst, const BaseLexer*, BaseLexer*>)>;

    public:
        explicit ASTFileTree(const Reader::Ptr& reader);
        ~ASTFileTree() override = default;

        template<IsFileParser Parser>
        void ParseUsing(LogCollector& logCollector)
        {
            if (!Verify(!!_fileReader, "File reader was nullptr"))
            {
                logCollector.AddLog({ "File reader was nullptr", LogCollector::LogType::Error });
                return;
            }

            Parser parser;
            parser.Parse(_fileReader, logCollector);

            parser.IterateOverLexers(
                [&](BaseLexer* lexer)
                {
                    if (!Verify(lexer, "Some lexer was nullptr but expected a valid object."))
                    {
                        logCollector.AddLog({ "Some lexer was nullptr but expected a valid object.", LogCollector::LogType::Error });
                        return true;
                    }

                    if (!lexer->HasParent())
                    {
                        _fileLexer->ForceSetAsChild(lexer);
                    }

                    return true;
                });

            _fileLexer->DoValidate(logCollector);
        }

        [[nodiscard]] Reader::Ptr GetReader() const { return _fileReader; }

        // ===========================================================
        // ================== WORKING WITH LEXERS ====================
        // ===========================================================

        template<IsLexer Lexer = void, bool IsConst = false>
        void ForEach(ForEachFunctionT<IsConst>&& callback)
        {
            Params params;
            ForEachImpl<Lexer, IsConst>(std::forward<ForEachFunctionT<IsConst>>(callback), _fileLexer.get(), params);
        }

        template<IsLexer Lexer = void>
        void ForEach(ForEachFunctionT<true>&& callback) const
        {
            Params params;
            ForEachImpl<Lexer, true>(std::forward<ForEachFunctionT<true>>(callback), _fileLexer.get(), params);
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
            return FindIfImpl<Lexer>(this, std::forward<FindFunctionT<false>>(callback));
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::CPtr FindIf(FindFunctionT<true>&& callback) const
        {
            return FindIfImpl<Lexer, true>(this, std::forward<FindFunctionT<true>>(callback));
        }

        template<IsLexer Lexer>
        [[nodiscard]] BaseLexer::Ptr FindIfAs(FindFunctionT<false>&& callback)
        {
            return boost::dynamic_pointer_cast<Lexer>(FindIfImpl<Lexer>(this, std::forward<FindFunctionT<false>>(callback)));
        }

        template<IsLexer Lexer>
        [[nodiscard]] BaseLexer::CPtr FindIfAs(FindFunctionT<true>&& callback) const
        {
            return boost::dynamic_pointer_cast<const Lexer>(FindIfImpl<Lexer, true>(this, std::forward<FindFunctionT<true>>(callback)));
        }

    private:
        template<IsLexer Lexer = void, bool IsConst = false>
        static bool ForEachImpl(ForEachFunctionT<IsConst>&& callback, BaseLexer::AdaptiveRawPtr<IsConst> base, Params& params)
        {
            if (!base || !callback)
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
                if (!std::invoke(callback, base, params))
                {
                    return false;
                }
            }

            if (base->HasChildLexers())
            {
                ++params.nesting;
                for (auto& child : base->GetChildLexers())
                {
                    if (child)
                    {
                        ForEachImpl<Lexer, IsConst>(std::forward<ForEachFunctionT<IsConst>>(callback), child.get(), params);
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
        FileLexer::Ptr _fileLexer;
        Reader::Ptr _fileReader;
    };

} // namespace Ast