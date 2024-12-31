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

#include "../CommonTypes.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <filesystem>

namespace Ast
{

    struct ContentFilter : public virtual ::Utils::CopyableAndMoveable
    {
        virtual void MakeTransform(String& content) = 0;

    protected:
        ContentFilter() = default;
    };

    template<class T>
    concept IsContentFilter = std::is_base_of_v<ContentFilter, T>;

    class ContentStream : public virtual ::Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<ContentStream>
    {
    public:
        AST_CLASS(ContentStream)

        ContentStream() = default;

        template<IsContentFilter... Filter>
        explicit ContentStream(const String::CharT* data)
        {
            Put(data);
            ApplyFilters<Filter...>();
        }

        ~ContentStream() override = default;

        bool Put(String content);
        bool Put(const String::CharT* content);
        [[nodiscard]] const String& Data() const noexcept;

        template<IsContentFilter... Filter>
        void ApplyFilters()
        {
            (Filter{}.MakeTransform(_content), ...);
            _content.ShrinkToFit();
        }

        [[nodiscard]] virtual String GetFilePath() const { return "None"_atom; }

        [[nodiscard]] static Ptr Create() { return boost::intrusive_ptr<ContentStream>(new ContentStream()); }

    protected:
        virtual void OnPut() {}

    protected:
        String _content;
    };

    class FileContentStream : public ContentStream
    {
    public:
        AST_CLASS(FileContentStream)

        FileContentStream() = default;

        template<IsContentFilter... Filter>
        explicit FileContentStream(const std::filesystem::path& path)
        {
            ReadFromFile(path);
        }

        void SetFilePath(const std::filesystem::path& path)
        {
            _path = path;
        }

        bool ReadFromFile(const std::filesystem::path& path);

        ~FileContentStream() override = default;

        [[nodiscard]] String GetFilePath() const override
        {
            return String::MakeFrom(_path);
        }

        [[nodiscard]] static Ptr Create() { return boost::intrusive_ptr<FileContentStream>(new FileContentStream()); }

    protected:
        void OnPut() override;

    protected:
        std::filesystem::path _path;
    };

} // namespace Ast