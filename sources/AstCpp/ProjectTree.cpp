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

#include "ProjectTree.h"

#include "Parser.h"

namespace Ast::Cpp
{

    void ProjectTree::onFinishScanFilesystem()
    {
        if (!_fstree)
        {
            return;
        }

        _fstree->forEach(
            [](DiskUnit* unit)
            {
                FileUnit* file = nullptr;
                if (!unit || !unit->isValid() || !unit->isExistOnDisk() || !(file = dynamic_cast<FileUnit*>(unit)))
                {
                    return;
                }

                auto path = unit->getPath();
                if (path.extension() != ".cpp" && path.extension() != ".h")
                {
                    return;
                }

                FileContentStream content(path);
                content.ApplyFilters<CommentFilter>();

                auto data = FileDataContainer::Create(Tree<Cpp::FileLexer>::From(Cpp::Parser(&content)));
                if (data->tree.HasAtLeastOneMarkedLexer())
                {
                    file->getData() = std::move(data);
                }
            });
    }

    ProjectTree::ProjectTree()
    {
        getAcceptableFileExtensions() = std::unordered_set<std::string>{ ".cpp", ".cxx", ".c++", ".h", ".hpp", ".hxx", ".H", ".hh" };
    }

} // namespace Ast::Cpp
