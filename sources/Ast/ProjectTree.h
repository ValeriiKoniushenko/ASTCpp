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
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <set>

namespace Ast
{
    class ProjectTree : public Utils::NotCopyableButMoveable
    {
    public:
        class Unit : public Utils::NotCopyableButMoveable
        {
        public:
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

            [[nodiscard]] Type GetType() const { return _type; }
            [[nodiscard]] bool IsFile() const { return _type == Type::File; }
            [[nodiscard]] bool IsFolder() const { return _type == Type::Folder; }
            [[nodiscard]] bool IsLink() const { return _type == Type::Link; }

            [[nodiscard]] std::filesystem::path GetPath() const { return _path; }
            [[nodiscard]] FileContentStream GetFileContentStream() const { return _contentStream; }

            [[nodiscard]] bool operator<(const Unit& rhs) const { return _path < rhs._path; }

        protected:
            Permission _permission = Permission::none;
            std::filesystem::path _path;
            Type _type = Type::None;
            FileContentStream _contentStream;

            std::set<Unit> _childs;

        };

    public:
        ProjectTree() = default;
        ~ProjectTree() override = default;

        void SetFileExtensions(std::vector<String> extensions);
        [[nodiscard]] const std::set<String>& GetFileExtensions() const { return _fileExtensions; };

        void SetTargetProject(const std::filesystem::path& path);
        [[nodiscard]] const std::filesystem::path& GetTargetProject() const noexcept { return _targetPath; }

        bool Process();

        [[nodiscard]] LogCollector& GetLogCollector() { return _logCollector; }

    protected:
        [[nodiscard]] bool IsValidExtension(const String& path) const;
        void ProcessFile(const std::filesystem::path& folders, const std::filesystem::path& fullPath);
        Unit& GetOrCreateUnit(const String& path);

    protected:
        std::set<String> _fileExtensions;
        std::set<Unit> _units;
        std::filesystem::path _targetPath;
        LogCollector _logCollector;
    };

} // namespace Ast