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

#include "Ast/Cache.h"
#include "Ast/Generators/Generator.h"
#include "Ast/ProjectTree.h"
#include "Ast/Tree.h"
#include "Ast/Utils/IO.h"
#include "AstCpp/Parser.h"
#include "AstCpp/Readers/Filters/CommentFilter.h"

#include <iostream>

int main()
{
    using namespace Ast;

    std::filesystem::copy(PATH_TO_TEST_PROJECT + std::string("small_project"), "small_project",
                          std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);


    LogCollector::Ptr logCollector = new Ast::LogCollector();
    logCollector->onValidationEvent.Subscribe(
        [](const Ast::String& message, Ast::LogCollector::LogType logType)
        {
            using namespace std;
            cout << "ASTCpp: [" << logType.ToStr() << "]: " << message.CStr() << endl;
        });


    auto project = ProjectTree::Ptr(new ProjectTree());
    project->SetFileExtensions({ "*.cpp", ".h" });
    project->SetTargetProject("small_project");
    project->ExcludeFromProject("excludedDirs");
    project->Process();
    project->ParseUsing<Cpp::Parser, Cpp::CommentFilter>();

    Generator generator;

    return 0;
}
