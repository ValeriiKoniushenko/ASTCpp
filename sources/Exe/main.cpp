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

#include "Ast/Generators/deprGenerator.h"
#include "Ast/ProjectTree.h"
#include "AstCpp/Generators/EnumClassGenerator.h"
#include "AstCpp/Parser.h"
#include "AstCpp/Readers/Filters/CommentFilter.h"

#include <algorithm>
#include <iostream>

int main(int argc, char* argv[])
{
    using namespace Ast;

    // if (argc != 2)
    // {
    //     std::cerr << "Usage: " << argv[0] << " <path_to_project_root>" << std::endl;
    //     return 1;
    // }
    // std::filesystem::path project_path(argv[1]);

    auto project = ProjectTree::Create();
    project->setPathToProject("/home/valerii/workspace/draft");

    if (!project->canBeScanned())
    {
        return 1;
    }

    project->addIgnorePath("ASTCpp");
    project->addIgnorePath("*.idea");
    project->addIgnorePath("*.git");
    project->addIgnorePath("*dependencies*");
    project->addIgnorePath("*build*");

    project->scanProject();

    project->getFSTree()->prettyPrint();

#if 0
    project->SetPreferableExtensionForGeneration(".h");
    project->SetFileExtensions({ "*.cpp", ".h" });
    project->SetTargetProject(project_path);
    project->ExcludeFromProject(".git");
    project->ExcludeFromProject("ASTCpp");
    project->ExcludeFromProject(".idea");
    project->ExcludeFromProject(".vs");
    project->ExcludeFromProject("dependencies");
    project->ExcludeFromProject("cmake-build-debug");
    project->Process();
    project->ParseUsing<Cpp::Parser, Cpp::CommentFilter>();

    /*
    Generator generator;
    generator.AddGenerator<Cpp::EnumClassGeneratorDecl, Cpp::EnumClassLexer>();
    generator.AddGenerator<Cpp::EnumClassGeneratorImpl, Cpp::EnumClassLexer>();

    generator.SetTargetProject(project);
    generator.ForEachOverRegenerateableUnits(
        [](ProjectTree::Unit* unit)
        {
            std::cout << "Generated file will be [re]created for this unit: " << unit->GetPath() << std::endl;
        });

    generator.Generate();
    */
#endif

    return 0;
}
