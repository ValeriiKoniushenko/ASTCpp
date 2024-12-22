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

#include "ProjectTree.h"

namespace Ast
{

    void ProjectTree::SetFileExtensions(std::vector<String> extensions)
    {
        for (auto& extension : extensions)
        {
            extension.Trim('*');
            extension.Trim('.');
            if (Verify(!extension.IsEmpty(), "Was passed invalid extension"))
            {
                extension.PushFront('.');
                _fileExtensions.emplace(std::move(extension));
            }
        }
    }

    void ProjectTree::SetTargetProject(const std::filesystem::path& path)
    {
        if (Verify(std::filesystem::exists(path), "Incorrect project path"))
        {
            _targetPath = path;
        }
    }

    bool ProjectTree::Process()
    {
        if (_fileExtensions.empty())
        {
            _logCollector.AddLog({ "File reader was nullptr", LogCollector::LogType::Error });
            return false;
        }
        if (_targetPath.empty())
        {
            _logCollector.AddLog({ "Target path is invalid", LogCollector::LogType::Error });
            return false;
        }

        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(_targetPath))
        {
            dirEntry.status()
            std::cout << dirEntry << std::endl;
        }


        return true;
    }

} // namespace Ast
