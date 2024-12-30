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

#pragma once

#include "LogCollector.h"
#include "Readers/ContentStream.h"
#include "Tree.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <set>

namespace Ast
{
    class ProjectTree : public Utils::NotCopyableButMoveable
    {
    public:

        class Unit : public boost::intrusive_ref_counter<Unit>, public Utils::CopyableAndMoveable
        {
        public:
            AST_CLASS(Unit);

            using Permission = std::filesystem::perms;

            enum class Type
            {
                None,
                File,
                Folder,
                Link
            };

            Unit() = default;
            ~Unit() override = default;

            [[nodiscard]] static Ptr Create() { return Ptr(new Unit()); }

            [[nodiscard]] Type GetType() const { return _type; }
            [[nodiscard]] bool IsFile() const { return _type == Type::File; }
            [[nodiscard]] bool IsFolder() const { return _type == Type::Folder; }
            [[nodiscard]] bool IsLink() const { return _type == Type::Link; }
            [[nodiscard]] bool IsExistsOnDisk() const;

            [[nodiscard]] std::filesystem::path GetPath() const { return _path; }
            [[nodiscard]] const FileContentStream::Ptr& GetFileContentStream() const { return _contentStream; }
            [[nodiscard]] FileContentStream::Ptr GetFileContentStream() { return _contentStream; }

            [[nodiscard]] bool operator<(const Unit& rhs) const { return _path < rhs._path; }
            [[nodiscard]] bool operator==(const Unit& rhs) const { return _path == rhs._path; }

            template<class T>
            Unit* AddChild(T&& unit)
            {
                return _AddChild(std::forward<T>(unit), false, false);
            }

            template<class T>
            Unit* ForceAddChild(T&& unit)
            {
                return _AddChild(std::forward<T>(unit), true, true);
            }

            template<class T>
            Unit* TryToAddChild(T&& unit)
            {
                return _AddChild(std::forward<T>(unit), false, true);
            }

            [[nodiscard]] bool HasChild(const Unit& unit) const;
            [[nodiscard]] const Ptr FindChild(const Unit& unit) const;

            [[nodiscard]] const Ptr& GetParent() const noexcept { return _parent; }
            [[nodiscard]] Ptr GetParent() { return _parent; }

            [[nodiscard]] static Unit CreateFromPath(const std::filesystem::path& path);
            [[nodiscard]] static Ptr CreatePtrFromPath(const std::filesystem::path& path);

            [[nodiscard]] Ptr GetUnitByPath(const std::filesystem::path& path);
            [[nodiscard]] bool IsExistUnitByPath(const std::filesystem::path& path);

            /** @brief a subfolder will be created based on logic(will be added to _childs) and will be
             * validated in the real path.
             * If the path will not valid - you will get an assert and the folder will not be created on the hard disk.
             */
            Unit* LinkSubFolder(const String& name);

            /** @brief a file will be created based on logic(will be added to _childs) and will be
             * validated in the real path.
             * If the path will not valid - you will get an assert and the file will not be created on the hard disk.
             */
            Unit* LinkSubFile(const String& name);

        protected:
            template<class T>
            T* _AddChild(T&& unit, const bool isForce, const bool isIgnoreAssert)
            {
                if (!isForce)
                {
                    if (HasChild(unit))
                    {
                        Assert(isIgnoreAssert, ("Impossible to add already existing unit: " + unit.GetPath().string()).c_str());
                        return nullptr;
                    }
                }

                unit._parent = this;
                auto it = _childs.emplace(Ptr(new Unit(std::move(unit))));

                return it.second ? it.first->get() : nullptr;
            }

            template<class T>
            T* _AddChild(boost::intrusive_ptr<T>&& unit, const bool isForce, const bool isIgnoreAssert)
            {
                if (!Verify(!!unit, "Was passed nullptr unit"))
                {
                    return nullptr;
                }

                if (!isForce)
                {
                    if (HasChild(*unit))
                    {
                        Assert(isIgnoreAssert, ("Impossible to add already existing unit: " + unit->GetPath().string()).c_str());
                        return nullptr;
                    }
                }

                unit->_parent = this;
                auto it = _childs.emplace(std::move(unit));

                return it.second ? it.first->get() : nullptr;
            }

            Unit* RawAddToChilds(Ptr&& unit);
        protected:
            Permission _permission = Permission::none;
            std::filesystem::path _path;
            Type _type = Type::None;
            Tree<FileLexer>::Ptr _tree;
            FileContentStream::Ptr _contentStream;

            std::set<Ptr> _childs;
            Ptr _parent;
        };

    public:
        ProjectTree() = default;
        ~ProjectTree() override = default;

        void SetFileExtensions(std::vector<String> extensions);
        [[nodiscard]] const std::set<String>& GetFileExtensions() const { return _fileExtensions; };

        void SetTargetProject(const std::filesystem::path& path);
        [[nodiscard]] std::filesystem::path GetTargetProject() const noexcept { return _root ? _root->GetPath() : std::filesystem::path(); }

        bool Process();

        [[nodiscard]] LogCollector& GetLogCollector() { return _logCollector; }

    protected:
        [[nodiscard]] bool IsValidExtension(const String& path) const;
        void ProcessFile(const std::filesystem::path& folders, const std::filesystem::path& fullPath);

    protected:
        std::set<String> _fileExtensions;
        Unit::Ptr _root;
        LogCollector _logCollector;
    };

} // namespace Ast