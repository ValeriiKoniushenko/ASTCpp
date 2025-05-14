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

#pragma once

#include "Ast/FileSystem/DiskUnits.h"
#include "Ast/ProjectTree.h"
#include "AstCpp/Lexers/FileLexer.h"
#include "AstCpp/Readers/Filters/CommentFilter.h"

namespace Ast::Cpp
{

    struct FileDataContainer : public Ast::FileUnit::DataContainer
    {
        AST_CLASS(FileDataContainer)

        [[nodiscard]] static Ptr Create(Tree<Cpp::FileLexer>&& newTree) { return new FileDataContainer(std::move(newTree)); }

        explicit FileDataContainer(Tree<Cpp::FileLexer>&& newTree)
            : tree(std::move(newTree))
        {
        }

        Tree<Cpp::FileLexer> tree;
    };

    class ProjectTree : public Ast::ProjectTree
    {
    public:
        AST_CLASS(ProjectTree)

    public:
        ProjectTree();
        ~ProjectTree() override = default;

        [[nodiscard]] static Ptr Create() { return new ProjectTree(); }
        void generate();

    protected:
        void onFinishScanFilesystem() override;

    private:
        [[nodiscard]] bool requireAbilityToGenerate() const;
    };

} // namespace Ast::Cpp
