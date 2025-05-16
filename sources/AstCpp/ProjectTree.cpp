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

#include "Core/Timer.h"
#include "Parser.h"

namespace Ast::Cpp
{

    void ProjectTree::onFinishScanFilesystem()
    {
        if (!_fstree)
        {
            return;
        }

        uint64_t count = 0;

        Core::Repeater repeater(0.25);
        repeater.setCallback(
            [&count, this](auto)
            {
                infoLog("Status: have build dependencies for {} entries."_f << count);
            });

        _fstree->forEach(
            [&count, &repeater](DiskUnit* unit)
            {
                ++count;
                repeater.startOrUpdate();

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

                FileContentStream::Ptr content = new FileContentStream(path);
                if (content->Data().isEmpty())
                {
                    return;
                }

#if AST_DEBUG == 1
                const auto beforeCommentFilterLen = String::GetLinesCountInText(content->Data().c_str());
                const String __firstDump = content->Data();
#endif
                content->ApplyFilters<CommentFilter>();
#if AST_DEBUG == 1
                const auto afterCommentFilterLen = String::GetLinesCountInText(content->Data().c_str());
                const String __secondDump = content->Data();
                Assert(beforeCommentFilterLen == afterCommentFilterLen);
#endif

                if (content->Data().isEmpty())
                {
                    return;
                }

                auto data = FileDataContainer::Create(Tree<Cpp::FileLexer>::From(Cpp::Parser(content)));
                if (data->tree.HasAtLeastOneMarkedLexer())
                {
                    file->getData() = std::move(data);
                }
            });

        String metricsStr;
        const auto timeGap = repeater.getTimeGap();
        if (timeGap >= 0.001)
        {
            metricsStr = "~{} entries per second."_f << int(static_cast<double>(count) / timeGap);
        }

        infoLog("Was took {}s for building of {} entries. {}"_f << timeGap << count << metricsStr);
    }

    ProjectTree::ProjectTree()
    {
        getAcceptableFileExtensions() = std::unordered_set<std::string>{ ".cpp", ".cxx", ".c++", ".h", ".hpp", ".hxx", ".H", ".hh" };
    }

    void ProjectTree::generate()
    {
        auto root = _fstree->getRootAs<DirectoryUnit>();
        if (!requireAbilityToGenerate() || !root)
        {
            return;
        }

        auto generatedDir = root->makeOrGetDir(_config.generatedDirName);

        if (!generatedDir)
        {
            criticalLog("Internal problem. Impossible to create '{}' dir."_f << _config.generatedDirName.generic_string());
            return;
        }

        forEachFilesWithMarkedLexers(
            [&generatedDir, this](FileUnit* file)
            {
                auto data = boost::dynamic_pointer_cast<FileDataContainer>(file->getData());

                std::vector<BaseLexer*> lexers;
                data->tree.ForEachOverMarked(
                    [&lexers](BaseLexer* lexer)
                    {
                        lexers.push_back(lexer);
                    });

                _composer->generate(lexers);
            });
    }

    bool ProjectTree::requireAbilityToGenerate() const
    {
        auto root = _fstree->getRoot();

        if (!root->isValid())
        {
            criticalLog("Internal problem while generating. Root path is invalid.");
            return false;
        }

        if (boost::dynamic_pointer_cast<DirectoryUnit>(root))
        {
            if (!root->isExistOnDisk())
            {
                criticalLog("Can't generate the code, because the passed project path doesn't exist.");
                return false;
            }
            if (!root->isWriteable())
            {
                criticalLog("Can't generate the code, because the passed project path doesn't have needed write permissions.");
                return false;
            }
        }
        else
        {
            criticalLog("Can't generate the code, because while trying of creating 'generated' folder, the project path is non-folder.");
            return false;
        }

        if (!_composer)
        {
            criticalLog("Can't generate the code, because the composer is missed.");
            return false;
        }
        else
        {
            if (_composer->getGeneratorsCount() == 0)
            {
                criticalLog("Can't generate the code, because the composer doesn't have any generators. Add it and try again.");
                return false;
            }
        }

        debugLog("Trying to generate a code. The root path was successfully validated: {}"_f << root->getPath().generic_string());

        return true;
    }

    void ProjectTree::forEachFilesWithMarkedLexers(const std::function<void(FileUnit*)>& callback)
    {
        if (!callback)
        {
            return;
        }

        forEachFilesWithData(
            [&callback](FileUnit* file)
            {
                auto data = boost::dynamic_pointer_cast<FileDataContainer>(file->getData());
                if (!data)
                {
                    return;
                }

                if (data->tree.HasAtLeastOneMarkedLexer())
                {
                    callback(file);
                }
            });
    }

} // namespace Ast::Cpp
