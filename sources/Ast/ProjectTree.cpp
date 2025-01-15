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

namespace
{
    const auto separator = []
    {
        Ast::String temp;
        temp += static_cast<Ast::String::CharT>(std::filesystem::path::preferred_separator);
        return temp;
    }();
} // namespace

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
            if (Verify(!!(unit._contentStream = FileContentStream::Ptr(new FileContentStream()), "Impossible to allocate an object")))
            {
                Assert(unit._contentStream->ReadFromFile(path), "Can't read a file: "_dyn + String::MakeFrom(path));
            }
            if (Verify(!!(unit._tree = Tree<FileLexer>::Ptr(new Tree<FileLexer>(unit._contentStream)), "Impossible to allocate an object")))
            {

            }

            unit._type = Type::File;
        }

        unit._permission = std::filesystem::status(path).permissions();

        return unit;
    }

    ProjectTree::Unit::Ptr ProjectTree::Unit::CreatePtrFromPath(const std::filesystem::path& path)
    {
        return Ptr(new Unit(std::move(CreateFromPath(path))));
    }

    ProjectTree::Unit::Ptr ProjectTree::Unit::GetUnitByPath(const std::filesystem::path& path)
    {
        auto relative = std::filesystem::relative(path, _path);
        if (relative.empty())
        {
            return nullptr;
        }

        const auto pathVector = Core::WStringAtom(relative.native().c_str()).ToASCII().Split(separator);
        if (pathVector.empty())
        {
            return nullptr;
        }

        Unit* temp = this;

        for (const auto& name : pathVector)
        {
            bool isFound = false;
            for (const auto& child : temp->_childs)
            {
                if (child->GetPath().string() == (temp->_path / name.ToStdStringView()).string())
                {
                    temp = child.get();
                    isFound = true;
                    break;
                }
            }
            if (!isFound)
            {
                temp = nullptr;
                break;
            }
        }

        return temp;
    }

    bool ProjectTree::Unit::IsExistUnitByPath(const std::filesystem::path& path)
    {
        return !!GetUnitByPath(path);
    }

    ProjectTree::Unit* ProjectTree::Unit::LinkSubFolder(const String& name)
    {
        if (!Verify(std::filesystem::exists(_path), "Invalid unit"))
        {
            return nullptr;
        }

        auto unit = Unit::Create();
        unit->_path = _path / name.ToStdStringView();
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

    bool ProjectTree::Unit::HasGeneratedFile() const
    {
        if (!IsFile())
        {
            return false;
        }

        if (_path.empty())
        {
            return false;
        }

        return GetGeneratedFilePath().empty();
    }

    std::filesystem::path ProjectTree::Unit::GetGeneratedFilePath() const
    {
        if (_path.empty() || !_path.has_extension())
        {
            return {};
        }

        auto path = _path;
        path.replace_extension(generatedSuffix + _path.extension().string());
        return path;
    }

    ProjectTree::Unit* ProjectTree::Unit::RawAddToChilds(Ptr&& unit)
    {
        auto it = _childs.emplace(std::move(unit));
        Assert(it.second, "Undefined error. Impossible to add new subfolder to the childs");

        return it.second ? it.first->get() : nullptr;
    }

    ProjectTree::ProjectTree()
    {
        _logCollector = LogCollector::Ptr(new LogCollector());
    }

    bool ProjectTree::IsValid() const
    {
        if ( _root != nullptr)
        {
            bool foundAtLeastOneFile = false;
            ForEach([&foundAtLeastOneFile](const auto*)
            {
                foundAtLeastOneFile = true;
                return false;
            });
            return foundAtLeastOneFile;
        }
        return false;
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

    void ProjectTree::ExcludeFromProject(std::filesystem::path path)
    {
        if (path.empty())
        {
            _logCollector->AddLog({"Was passed a path to exclude it. But the path is empty", LogCollector::LogType::Warning});
            return;
        }
        _excluded.emplace(std::move(path));
    }

    bool ProjectTree::IsExcludedPath(std::filesystem::path path) const
    {
        if (_excluded.contains(path))
        {
            return true;
        }

        if (path.is_relative())
        {
            if (Verify(!!_root))
            {
                path = _root->GetPath() / path;
                path = std::filesystem::canonical(path);
                if (path.empty())
                {
                    _logCollector->AddLog({"Was trying to convert a path to absolute, but met some problem.", LogCollector::LogType::Error});
                    return false;
                }
            }
        }

        for (auto p : _excluded)
        {
            if (p.is_relative())
            {
                if (Verify(!!_root))
                {
                    p = _root->GetPath() / p;
                    p = std::filesystem::canonical(p);
                    if (p.empty())
                    {
                        _logCollector->AddLog({"Was trying to convert a path to absolute, but met some problem.", LogCollector::LogType::Error});
                        return false;
                    }
                }
            }

            if (path.string().contains(p.string()))
            {
                return true;
            }
        }

        return false;
    }

    const std::unordered_set<std::filesystem::path>& ProjectTree::GetExcludedPaths() const noexcept
    {
        return _excluded;
    }

    /*
    bool ProjectTree::ApplyGitignore(std::filesystem::path path)
    {
        if (!_root)
        {
            return false;
        }

        if (path.empty())
        {
            path = _root->GetPath() / ".gitignore";
        }

        if (!std::filesystem::exists(path))
        {
            _logCollector->AddLog({"By the next path: {} - .gitignore wasn't found"_f << path.string(), LogCollector::LogType::Error });
            return false;
        }

        std::ifstream file(path);
        if (!file.is_open())
        {
            _logCollector->AddLog({"Can't open a .gitignore file by the next path: {}"_f << path.string(), LogCollector::LogType::Error });
            return false;
        }

        constexpr std::size_t size = 1024;
        char line[size];

        while (!file.eof())
        {
            file.getline(line, size);
            if (strlen(line) > 0)
            {
                _excluded.emplace(line);
            }
        }

        file.close();

        return true;
    }
    */

    void ProjectTree::SetTargetProject(const std::filesystem::path& path)
    {
        if (Verify(std::filesystem::exists(path), "Incorrect project path"))
        {
            _root = Unit::CreatePtrFromPath(path);
        }
    }

    bool ProjectTree::Process()
    {
        if (_fileExtensions.empty())
        {
            _logCollector->AddLog({ "File reader was nullptr", LogCollector::LogType::Error });
            return false;
        }
        if (!_root)
        {
            _logCollector->AddLog({ "Target path is invalid", LogCollector::LogType::Error });
            return false;
        }

        for (const auto& i : std::filesystem::recursive_directory_iterator(_root->GetPath()))
        {
            auto tmp = String(i.path().string());
            if (!Verify(tmp.Find(_root->GetPath().string())))
            {
                _logCollector->AddLog(
                    { "Can't process the next file: {} - it's not a part of the project"_f << tmp, LogCollector::LogType::Warning });
                continue;
            }
            if (std::filesystem::is_directory(i))
            {
                continue;
            }
            const auto targetPathSize = _root->GetPath().string().size();
            tmp.SubStr(targetPathSize).TrimStart('\\');

            if (Verify(!tmp.IsEmpty()))
            {
                auto newPath = std::filesystem::path(tmp.c_str());

                if (IsValidExtension(String(newPath.extension().string())) && !IsExcludedPath(newPath))
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
        if (!Verify(!!_root))
        {
            return;
        }

        Unit* i = nullptr;
        for (const auto& folder : String(folders.string()).Split(separator))
        {
            if (i)
            {
                auto ptr = _root->GetUnitByPath(i->GetPath() / folder.ToStdStringView());
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
                const auto finalPath = _root->GetPath() / folder.ToStdStringView();
                if (auto found = _root->GetUnitByPath(finalPath))
                {
                    i = found.get();
                }
                else
                {
                    auto* newUnit = _root->TryToAddChild(Unit::CreateFromPath(finalPath));
                    if (Verify(newUnit, "Impossible to create new unit"))
                    {
                        i = newUnit;
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
