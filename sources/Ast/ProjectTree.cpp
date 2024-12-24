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

    ProjectTree::Unit ProjectTree::Unit::CreateFromPath(const std::filesystem::path& path)
    {
        Unit unit;
        unit._path = path;
        if (std::filesystem::is_directory(path))
        {
            unit._type = Type::Folder;
        }
        else if (std::filesystem::is_symlink(path))
        {
            unit._type = Type::Link;
        }
        else
        {
            unit._type = Type::File;
        }

        unit._permission = std::filesystem::status(path).permissions();

        return unit;
    }

    ProjectTree::Unit::Ptr ProjectTree::Unit::CreatePtrFromPath(const std::filesystem::path& path)
    {
        return Ptr(new Unit(std::move(CreateFromPath(path))));
    }

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

        for (const auto& i : std::filesystem::recursive_directory_iterator(_targetPath))
        {
            auto tmp = String(i.path().string());
            if (!Verify(tmp.Find(_targetPath.string())))
            {
                continue;
            }
            if (std::filesystem::is_directory(i))
            {
                continue;
            }
            const auto targetPathSize = _targetPath.string().size();
            tmp.SubStr(targetPathSize).TrimStart('\\');

            if (Verify(!tmp.IsEmpty()))
            {
                auto newPath = std::filesystem::path(tmp.c_str());

                if (IsValidExtension(String(newPath.extension().string())))
                {
                    ProcessFile(newPath.parent_path(), i.path());
                }
            }
        }

        return true;
    }

    bool ProjectTree::IsValidExtension(const String& ex) const
    {
        for (const auto& extension : _fileExtensions)
        {
            if (extension == ex)
            {
                return true;
            }
        }

        return false;
    }

    void ProjectTree::ProcessFile(const std::filesystem::path& folders, const std::filesystem::path& fullPath)
    {
        const auto separator = String(static_cast<String::CharT>(std::filesystem::path::preferred_separator));

        Unit* i = nullptr;
        for (const auto& folder : String(folders.string()).Split(separator))
        {
            auto& unit = GetOrCreateUnit(folder);
            if (i)
            {
                i->ForceAddChild(Unit::CreateFromPath(fullPath));
            }
            else
            {
                i = &unit;
            }
        }
    }

    ProjectTree::Unit& ProjectTree::GetOrCreateUnit(const String& path)
    {
        auto found = std::ranges::find_if(std::as_const(_units),
                                  [&path](const Unit::Ptr& a)
                                  {
                                      return a->GetPath().string().find(path.ToStringView());
                                  });

        if (found != std::ranges::end(_units))
        {
            return *found->get();
        }

        std::filesystem::path finalPath;
        {
            auto newPath = _targetPath.string() + static_cast<String::CharT>(std::filesystem::path::preferred_separator);
            newPath += path.ToStringView();
            finalPath = std::filesystem::path(newPath);
        }

        return *_units.emplace(Unit::CreatePtrFromPath(finalPath)).first->get();
    }

} // namespace Ast
