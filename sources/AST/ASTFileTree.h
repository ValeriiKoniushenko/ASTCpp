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
#include "Readers/FileReader.h"
#include "ConvertibleTrees/ConvertibleTree.h"
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
        using ForEachFunctionT = std::function<bool(BaseLexer*, Params)>;

    public:
        explicit ASTFileTree(const FileReader::Ptr& reader);
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
            parser.Parse(*_fileReader.get(), logCollector);

            parser.IterateOverLexers([&](BaseLexer* lexer)
            {
                if (!Verify(lexer, "Some lexer was nullptr but expected a valid object."))
                {
                    logCollector.AddLog({"Some lexer was nullptr but expected a valid object.", LogCollector::LogType::Error});
                    return true;
                }

                if (!lexer->HasParent())
                {
                    _fileLexer.ForceSetAsChild(lexer);
                }

                return true;
            });

            _fileLexer.DoValidate(logCollector);
        }

        template<IsConvertibleToTree T>
        [[nodiscard]] T ConvertTo(LogCollector& logCollector) const
        {
            T t;
            t.Convert(this, logCollector);
            return t;
        }

        [[nodiscard]] FileReader::Ptr GetFileReader() const { return _fileReader; }

        template<IsLexer Lexer = void>
        void ForEach(ForEachFunctionT&& callback)
        {
            Params params;
            BaseForEach<Lexer>(std::forward<ForEachFunctionT>(callback), &_fileLexer, params);
        }

        template<IsLexer Lexer = void>
        [[nodiscard]] BaseLexer::Ptr FindIf(std::function<bool(BaseLexer*)>&& callback)
        {
            if (!callback)
            {
                return {};
            }

            BaseLexer::Ptr ret;
            ForEach<Lexer>([&callback, &ret](BaseLexer* lexer, auto)
            {
                if (!callback(lexer))
                {
                    ret = lexer;
                    return false;
                }
                return true;
            });

            return ret;
        }

    private:
        template<IsLexer Lexer = void>
        bool BaseForEach(ForEachFunctionT&& callback, BaseLexer* base, Params& params)
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
                        BaseForEach<Lexer>(std::forward<ForEachFunctionT>(callback), child.get(), params);
                    }
                }
                --params.nesting;
            }

            return true;
        }

    private:
        FileLexer _fileLexer;
        FileReader::Ptr _fileReader;
    };

} // namespace Ast