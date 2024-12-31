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

Ast::String ValidateAllProjectTree(Ast::ProjectTree& project)
{
    using namespace Ast;

    Core::StringAtom error;
    project.ForEach([&error](ProjectTree::Unit* unit)
    {
        if (!unit)
        {
            error = Core::StringAtom("Unit is nullptr");
            return false;
        }

        auto content = unit->GetFileContentStream();
        if (!content)
        {
            error = Core::StringAtom("Unit's ContentStream is nullptr");
            return false;
        }

        if (unit->GetPermission() == ProjectTree::Unit::Permission::none)
        {
            error = Core::StringAtom("Unit's ProjectTree::Unit::Permission is None");
            return false;
        }

        if (unit->GetPath().empty())
        {
            error = Core::StringAtom("Unit's path is None");
            return false;
        }

        if (unit->GetType() == ProjectTree::Unit::Type::None)
        {
            error = Core::StringAtom("Unit's type is None");
            return false;
        }

        if (unit->GetType() == ProjectTree::Unit::Type::None)
        {
            error = Core::StringAtom("Unit's type is None");
            return false;
        }

        auto tree = unit->GetTree();
        if (!tree)
        {
            error = Core::StringAtom("Unit's tree is nullptr");
            return false;
        }

        if (!tree->GetReader())
        {
            error = Core::StringAtom("Unit's tree->reader is nullptr");
            return false;
        }

        if (!tree->GetRootLexer())
        {
            error = Core::StringAtom("Unit's tree->rootLexer is nullptr");
            return false;
        }

        bool wasError = false;
        tree->ForEach([&](const BaseLexer* lexer, auto)
        {
            if (!lexer)
            {
                error = "Unit's '{}' lexer is nullptr"_f << unit->GetPath().c_str();
                wasError = true;
                return false;
            }

            if (!lexer->GetReader())
            {
                error = "Unit's '{}' reader is nullptr"_f << unit->GetPath().c_str();
                wasError = true;
                return false;
            }

            if (!lexer->GetRootLexer())
            {
                error = "Unit's '{}' root lexer is nullptr"_f << unit->GetPath().c_str();
                wasError = true;
                return false;
            }

            if (lexer->GetLexerName().IsEmpty() || lexer->GetLexerName() == "none"_atom)
            {
                error = "Unit's '{}' lexer doesn't have a lexer name"_f << unit->GetPath().c_str();
                wasError = true;
                return false;
            }

            if (!lexer->IsTypeOf<FileLexer>())
            {
                if (!lexer->GetCloseScope())
                {
                    error = "Unit's '{}' lexer '{}'  doesn't have a closed scope"_f << unit->GetPath().c_str()
                        << lexer->GetLexerName().c_str();
                    wasError = true;
                    return false;
                }

                if (!lexer->GetOpenScope())
                {
                    error = "Unit's '{}' lexer '{}'  doesn't have a opened scope"_f << unit->GetPath().c_str()
                        << lexer->GetLexerName().c_str();
                    wasError = true;
                    return false;
                }
            }

            return true;
        });

        return true;
    });

    return error;
}

} // namespace


TEST(ASTProjectTest, simple_parse_project_tree)
{
    using namespace Ast;

    ProjectTree project;
    project.SetFileExtensions({"*.cpp", ".h"});
    project.SetTargetProject(projectPath);
    project.Process();
    project.ParseUsing<Cpp::Parser, Cpp::CommentFilter>();

    // just trying to get first\any Unit
    ProjectTree::Unit::Ptr found;
    project.ForEach([&found](ProjectTree::Unit* unit)
    {
        found = unit;
        return true;
    });
    ASSERT_TRUE(found);

    // tree checking
    auto tree = found->GetTree();
    ASSERT_TRUE(tree);

    EXPECT_EQ(String::MakeFrom(found->GetPath()), tree->GetRootLexer()->GetLexerName());

    auto error = ValidateAllProjectTree(project);

    ASSERT_TRUE(error.IsEmpty()) << error.c_str();
}

TEST(ASTProjectTest, move_parse_project_tree)
{
    using namespace Ast;

    ProjectTree project;
    {
        ProjectTree temp;
        temp.SetFileExtensions({"*.cpp", ".h"});
        temp.SetTargetProject(projectPath);
        temp.Process();
        temp.ParseUsing<Cpp::Parser, Cpp::CommentFilter>();
        project = std::move(temp);
    }

    auto error = ValidateAllProjectTree(project);

    ASSERT_TRUE(error.IsEmpty()) << error.c_str();
}