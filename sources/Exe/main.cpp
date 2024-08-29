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

#include "AST/ASTFileTree.h"
#include "AST/ConvertibleTrees/XMLTree.h"
#include "AST/Readers/FileReader.h"
#include "ASTCpp/FileParser.h"
#include "ASTCpp/Readers/Filters/CommentFilter.h"

#include <iostream>

int main()
{

    Ast::FileReader::Ptr fileReader = new Ast::FileReader;
    Ast::LogCollector logCollector;
    logCollector.onValidationEvent.Subscribe([](const Ast::String& message, Ast::LogCollector::LogType logType)
    {
        using namespace std;
        const char* typeStr = [logType](){
            if (logType == Ast::LogCollector::LogType::Error) return "Error";
            if (logType == Ast::LogCollector::LogType::Warning) return "Warning";
            if (logType == Ast::LogCollector::LogType::Success) return "Success";
            if (logType == Ast::LogCollector::LogType::Info) return "Info";
            return "None";
        }();

        cout << "ASTCpp: [" << typeStr << "]: " << message.CStr() << endl;
    });

    if (fileReader->Read("D:\\Workspace\\test.cpp"))
    {
        fileReader->ApplyFilters<Ast::Cpp::CommentFilter>();
        Ast::ASTFileTree tree(fileReader);
        tree.ParseUsing<Ast::Cpp::FileParser>(logCollector);

        tree.ForEach([&](Ast::BaseLexer* lexer, Ast::ASTFileTree::Params params)
        {
            std::cout << "> ";
            for (int i = 0; i < params.nesting; ++i)
            {
                std::cout << "\t";
            }
            std::cout << lexer->GetName().c_str() << std::endl;

            return true;
        });

        auto found = tree.FindIf([](Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "Vec2";
        });

        if (Verify(!!found))
        {
            int i = 1;
        }

        // auto xmlTree = tree.ConvertTo<Ast::XMLTree>(logCollector);
    }

    return 0;
}
