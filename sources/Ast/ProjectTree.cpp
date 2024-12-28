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

#include "Utils/Functions.h"

namespace Ast
{

    bool ProjectTree::Unit::IsExistsOnDisk() const
    {
        std::error_code ec;
        return std::filesystem::exists(_path, ec);
    }

    bool ProjectTree::Unit::HasChild(const Unit& unit) const
    {
        auto found = std::find_if(_childs.cbegin(), _childs.cend(),
                                  [&unit](const Ptr& a)
                                  {
                                      return *a.get() == unit;
                                  });

        return found != _childs.cend();
    }

    const ProjectTree::Unit::Ptr ProjectTree::Unit::FindChild(const Unit& unit) const
    {
        auto found = std::find_if(_childs.cbegin(), _childs.cend(),
                                  [&unit](const Ptr& a)
                                  {
                                      return *a.get() == unit;
                                  });

        if (found != _childs.cend())
            return *found;

        return nullptr;
    }

    ProjectTree::Unit ProjectTree::Unit::CreateFromPath(const std::filesystem::path& path)
    {
        Unit unit;
        unit._path = path;

        if (!Verify(unit.IsExistsOnDisk()))
        {
            return {};
        }

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
            unit._contentStream.ReadFromFile(path);
            unit._type = Type::File;
        }

        unit._permission = std::filesystem::status(path).permissions();

        return unit;
    }

    ProjectTree::Unit::Ptr ProjectTree::Unit::CreatePtrFromPath(const std::filesystem::path& path)
    {
        return Ptr(new Unit(std::move(CreateFromPath(path))));
    }

    ProjectTree::Unit* ProjectTree::Unit::LinkSubFolder(const String& name)
    {
        if (!Verify(std::filesystem::exists(_path), "Invalid unit"))
        {
            return nullptr;
        }

        auto unit = Unit::Create();
        unit->_path = _path / name.ToStringView();
        unit->_type = Type::Folder;
        unit->_parent = this;
        unit->_permission = _permission;
        if (unit->IsExistsOnDisk())
        {
            if (!Verify(!HasChild(*unit), "Such subfolder already exists"))
            {
                return nullptr;
            }

            return RawAddToChilds(std::move(unit));
        }

        return nullptr;
    }

    ProjectTree::Unit* ProjectTree::Unit::LinkSubFile(const String& name)
    {
        if (!Verify(std::filesystem::exists(_path), "Invalid unit"))
        {
            return nullptr;
        }

        auto unit = CreatePtrFromPath(_path.string() + static_cast<String::CharT>(std::filesystem::path::preferred_separator) + name.ToStdString());
        if (!Verify(!!unit))
        {
            return nullptr;
        }

        unit->_parent = this;
        if (!Verify(!HasChild(*unit), "Such file already exists"))
        {
            return nullptr;
        }

        return RawAddToChilds(std::move(unit));
    }

    ProjectTree::Unit* ProjectTree::Unit::RawAddToChilds(Ptr&& unit)
    {
        auto it = _childs.emplace(std::move(unit));
        Assert(it.second, "Undefined error. Impossible to add new subfolder to the childs");

        return it.second ? it.first->get() : nullptr;
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

    bool ProjectTree::IsExistUnitByPath(const std::filesystem::path& path) const
    {
        return !!GetUnitByPath(path);
    }

    ProjectTree::Unit::Ptr ProjectTree::GetUnitByPath(const std::filesystem::path& path)
    {
        auto found = std::ranges::find_if(_units,
                                     [&path](const Unit::Ptr& a)
                                     {
                                         return a->GetPath() == path;
                                     });
        return found != _units.end() ? *found : nullptr;
    }

    const ProjectTree::Unit::Ptr ProjectTree::GetUnitByPath(const std::filesystem::path& path) const
    {
        auto found = std::ranges::find_if(std::as_const(_units),
                                 [&path](const Unit::Ptr& a)
                                 {
                                     return a->GetPath() == path;
                                 });
        return found != _units.cend() ? *found : nullptr;
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
        static const auto separator = []
        {
            String temp;
            temp += static_cast<String::CharT>(std::filesystem::path::preferred_separator);
            return temp;
        }();

        Unit* i = nullptr;
        for (const auto& folder : String(folders.string()).Split(separator))
        {
            if (i)
            {
                auto ptr = GetUnitByPath(i->GetPath() / folder.ToStringView());
                if (!ptr)
                {
                    auto* newUnit = i->LinkSubFolder(folder);
                    if (Verify(newUnit, "Impossible to create subfolder"))
                    {
                        i = newUnit;
                    }
                }
                else
                {
                    i = ptr.get();
                }
            }
            else
            {
                auto found = GetUnitByPath(_targetPath / folder.ToStringView());
                if (found)
                {
                    i = found.get();
                }
                else
                {
                    auto it = _units.emplace(Unit::CreatePtrFromPath(_targetPath / folder.ToStringView()));
                    if (Verify(it.second))
                    {
                        i = it.first->get();
                    }
                }
            }
        }
        if (Verify(i, "Undefined error. Unit is nullptr"))
        {
            i->LinkSubFile(String(fullPath.filename().string()));
        }
    }

} // namespace Ast
