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
        struct Params
        {
            int nesting = 0;
        };

        using Ptr = boost::intrusive_ptr<ASTFileTree>;

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
                logCollector.AddLog({"File reader was nullptr", LogCollector::LogType::Error});
                return;
            }

            Parser parser;
            parser.Parse(_fileReader, logCollector);

            parser.IterateOverLexers([&](BaseLexer* lexer)
            {
                if (!Verify(lexer, "Some lexer was nullptr but expected a valid object."))
                {
                    logCollector.AddLog({"Some lexer was nullptr but expected a valid object.", LogCollector::LogType::Error});
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
        [[nodiscard]] BaseLexer::Ptr FindIf(FindFunctionT<false>&& callback)
        {
            return FindIfImpl<Lexer>(this, std::forward<FindFunctionT<false>>(callback));
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::CPtr FindIf(FindFunctionT<true>&& callback) const
        {
            return FindIfImpl<Lexer, true>(this, std::forward<FindFunctionT<true>>(callback));
        }

    private:
        template<IsLexer Lexer = void, bool IsConst = false>
        static bool ForEachImpl(ForEachFunctionT<IsConst>&& callback, std::conditional_t<IsConst, const BaseLexer*, BaseLexer*> base, Params& params)
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
                for(auto& child : base->GetChildLexers())
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
        [[nodiscard]] static std::conditional_t<IsConst, BaseLexer::CPtr, BaseLexer::Ptr> FindIfImpl(std::conditional_t<IsConst, const ASTFileTree, ASTFileTree>* fileTree, FindFunctionT<IsConst>&& callback)
        {
            if (!callback)
            {
                return {};
            }

            std::conditional_t<IsConst, BaseLexer::CPtr, BaseLexer::Ptr> ret;
            fileTree->ForEach<Lexer>([&callback, &ret](std::conditional_t<IsConst, const BaseLexer, BaseLexer>* lexer, auto)
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

    private:
        FileLexer::Ptr _fileLexer;
        Reader::Ptr _fileReader;
    };

} // namespace Ast