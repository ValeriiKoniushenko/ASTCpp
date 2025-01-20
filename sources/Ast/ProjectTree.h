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

#include <ppltasks.h>
#include <set>
#include <unordered_set>

namespace Ast
{
    class ProjectTree : public Utils::NotCopyableButMoveable, public boost::intrusive_ref_counter<ProjectTree>
    {
    public:
        AST_CLASS(ProjectTree)

        class Unit : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<Unit>
        {
        public:
            AST_CLASS(Unit);

            inline static const char* generatedFileHeader_Head = "// Time of generation: ";
            inline static const char* generatedFileHeader_Body = R"(// This file was generated automatically don't change it and don't remove it
// If you see some compile errors you can fix it in the code-gen setup of
// your project. If the issue was caused by core of the code-gen - find a
// contact in the github repository and author will fix it.
)";
            inline static const char* generatedSuffix = ".generated";

            using Permission = std::filesystem::perms;

            struct GenerateData
            {
                bool isDirty = false;
                uint32_t lastWriteTime = 0;
            };

            enum class Type
            {
                None,
                File,
                Folder,
                Link
            };

        public:
            Unit() = default;
            ~Unit() override = default;

            [[nodiscard]] static bool IsGenerated(const Unit& unit) noexcept
            {
                return unit.IsGenerated();
            }
            [[nodiscard]] bool IsGenerated() const noexcept
            {
                return _generateData.has_value();
            }

            [[nodiscard]] static Ptr Create() { return new Self; }

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

            [[nodiscard]] uint64_t GetLastModificationTime() const;

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

            /**
             * @brief Can take a functions of next types:
             * bool([const] Unit*) - this function will work until it gets 'false' in return
             * void([const] Unit*) - will iterate without stopping through all a tree
             */
            template<class FuncT>
            void ForEach(FuncT&& callback)
            {
                ForEachImpl<false, FuncT>(this, std::forward<decltype(callback)>(callback));
            }

            /**
             * @brief Can take a functions of next types:
             * bool(const Unit*) - this function will work until it gets 'false' in return
             * void(const Unit*) - will iterate without stopping through all a tree
             */
            template<class FuncT>
            void ForEach(FuncT&& callback) const
            {
                ForEachImpl<true, FuncT>(this, std::forward<decltype(callback)>(callback));
            }

            [[nodiscard]] Tree<FileLexer>::AdaptivePtr<false> GetTree() { return _tree; }
            [[nodiscard]] Tree<FileLexer>::AdaptivePtr<true> GetTree() const { return _tree; }

            [[nodiscard]] Permission GetPermission() const noexcept { return _permission; }

            [[nodiscard]] bool HasGeneratedSiblingFile() const;
            [[nodiscard]] std::filesystem::path GetGeneratedSiblingFilePath() const;

            void _SetTree(Tree<FileLexer>&& tree)
            {
                if (!IsGenerated())
                {
                    _tree = Tree<FileLexer>::Ptr(new Tree<FileLexer>(std::move(tree)));
                }
            }

        protected:
            // ================== PIPMPLs =======================
            template<bool IsConst, class FuncT>
            static bool ForEachImpl(AdaptiveRawPtr<IsConst> base, FuncT&& callback)
            {
                if (base->IsFile())
                {
                    if constexpr (std::is_void_v<decltype(callback(base))>)
                    {
                        std::invoke(std::forward<decltype(callback)>(callback), base);
                    }
                    else
                    {
                        if (!std::invoke(std::forward<decltype(callback)>(callback), base))
                        {
                            return false;
                        }
                    }
                }

                for (auto& child : base->_childs)
                {
                    if (child)
                    {
                        if (!ForEachImpl<IsConst>(child.get(), std::forward<decltype(callback)>(callback)))
                        {
                            return false;
                        }
                    }
                }

                return true;
            }

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
            [[nodiscard]] bool CheckByPathIfWasGenerated() const;
            [[nodiscard]] uint64_t ExtrudeGenerationTime() const;

        protected:
            Permission _permission = Permission::none;
            std::filesystem::path _path;
            Type _type = Type::None;
            // this field using if a file was generated
            std::optional<GenerateData> _generateData;
            Tree<FileLexer>::Ptr _tree;
            FileContentStream::Ptr _contentStream;

            std::set<Ptr> _childs;
            Ptr _parent;
        };

    public:
        ProjectTree();
        ~ProjectTree() override = default;
        ProjectTree(ProjectTree&&) = default;
        ProjectTree& operator=(ProjectTree&&) = default;

        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] bool operator!() const { return IsValid(); }

        void SetFileExtensions(std::vector<String> extensions);
        [[nodiscard]] const std::set<String>& GetFileExtensions() const { return _fileExtensions; };

        void ExcludeFromProject(std::filesystem::path path);
        [[nodiscard]] bool IsExcludedPath(std::filesystem::path path) const;
        [[nodiscard]] const std::unordered_set<std::filesystem::path>& GetExcludedPaths() const noexcept;
        // bool ApplyGitignore(std::filesystem::path path = "");

        void SetTargetProject(const std::filesystem::path& path);
        [[nodiscard]] std::filesystem::path GetTargetProject() const noexcept { return _root ? _root->GetPath() : std::filesystem::path(); }

        bool Process();

        template<IsParser ParserT, IsContentFilter ContentFilterT = void>
        void ParseUsing()
        {
            ForEach([this](Unit* unit)
            {
                if constexpr (!std::is_void_v<ContentFilterT>)
                {
                    unit->GetFileContentStream()->ApplyFilters<ContentFilterT>();
                }

                if (!unit->IsGenerated())
                {
                    Tree<FileLexer> tree(unit->GetFileContentStream());
                    tree.ParseUsing<ParserT>(_logCollector);
                    unit->_SetTree(std::move(tree));
                }

                return true;
            });
        }

        [[nodiscard]] LogCollector::Ptr GetLogCollector() { return _logCollector; }
        [[nodiscard]] LogCollector::CPtr GetLogCollector() const { return _logCollector; }

        // ==========================================================
        // ================== WORKING WITH UNITS ====================
        // ==========================================================
        [[nodiscard]] Unit::AdaptivePtr<true> GetUnitByPath() const
        {
            return _root;
        }
        [[nodiscard]] Unit::AdaptivePtr<false> GetUnitByPath()
        {
            return _root;
        }

        [[nodiscard]] Unit::AdaptivePtr<true> GetUnitByPath(const std::filesystem::path& path) const
        {
            return _root && !path.empty() ? _root->GetUnitByPath(path) : nullptr;
        }
        [[nodiscard]] Unit::AdaptivePtr<false> GetUnitByPath(const std::filesystem::path& path)
        {
            return _root && !path.empty() ? _root->GetUnitByPath(path) : nullptr;
        }

        [[nodiscard]] Unit::AdaptivePtr<false> GetGeneratedFileOfUnit(const Unit::Ptr& unit);
        [[nodiscard]] Unit::AdaptivePtr<true> GetGeneratedFileOfUnit(const Unit::CPtr& unit) const;
        [[nodiscard]] Unit::AdaptivePtr<false> GetGeneratedFileOfUnit(const Unit& unit);
        [[nodiscard]] Unit::AdaptivePtr<true> GetGeneratedFileOfUnit(const Unit& unit) const;
        [[nodiscard]] bool IsNeedRegeneration(const Unit::CPtr& unit) const;
        [[nodiscard]] bool IsNeedRegeneration(const Unit& unit) const;

        /**
         * @brief Can take a functions of next types:
         * 1. bool(const Unit*) - this function will work until it gets 'false' in return
         * 2. void(const Unit*) - will iterate without stopping through all a tree
         */
        template<class FuncT>
        void ForEach(FuncT&& callback) const
        {
            if (_root)
            {
                static_cast<const Unit*>(_root.get())->template ForEach<FuncT>(std::forward<decltype(callback)>(callback));
            }
        }
        /**
         * @brief Can take a functions of next types:
         * 1. bool([const] Unit*) - this function will work until it gets 'false' in return
         * 2. void([const] Unit*) - will iterate without stopping through all a tree
         */
        template<class FuncT>
        void ForEach(FuncT&& callback)
        {
            if (_root)
            {
                _root->template ForEach<FuncT>(std::forward<decltype(callback)>(callback));
            }
        }

    protected:
        [[nodiscard]] bool IsValidExtension(const String& path) const;
        void ProcessFile(const std::filesystem::path& folders, const std::filesystem::path& fullPath);

    protected:
        std::set<String> _fileExtensions;
        Unit::Ptr _root;
        LogCollector::Ptr _logCollector;
        std::unordered_set<std::filesystem::path> _excluded;
    };

} // namespace Ast