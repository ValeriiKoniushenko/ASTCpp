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

#define CORE_DEBUG

#include "Ast/ProjectTree.h"
#include "AstCpp/Parser.h"
#include "AstCpp/Readers/Filters/CommentFilter.h"
#include "AstCpp/TemplateLexer/CheckForTemplateLexer.h"

#include <gtest/gtest.h>

namespace
{

std::filesystem::path projectPath = "C:\\Users\\Valerii\\Downloads\\WindowsProfiler-lib-develop";

} // namespace


TEST(ASTProjectTest, ParseProjectTree)
{
    using namespace Ast;

    ProjectTree project;
    project.SetFileExtensions({"*.cpp", ".h"});
    project.SetTargetProject(projectPath);
    project.Process();
    project.ParseUsing<Cpp::Parser, Cpp::CommentFilter>();

    ProjectTree::Unit::Ptr found;
    project.ForEach([&found](ProjectTree::Unit* unit)
    {
        found = unit;
        return true;
    });
    ASSERT_TRUE(found);


    auto tree = found->GetTree();
    ASSERT_TRUE(tree);



    int iasdfas = 1;
}