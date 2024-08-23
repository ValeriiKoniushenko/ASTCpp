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
#include "Utils/CopyableAndMoveableBehaviour.h"

namespace Ast
{

    class ASTFileTree : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<ASTFileTree>
    {
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
        }

    private:
        FileLexer _fileLexer;
        FileReader::Ptr _fileReader;
    };

} // namespace Ast