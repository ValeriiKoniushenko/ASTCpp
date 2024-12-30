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

#include "ContentStream.h"

#include "Utils/Functions.h"

namespace Ast
{

    bool ContentStream::Put(String content)
    {
        _content = std::move(content);
        _content.shrink_to_fit();
        if (!_content.IsEmpty())
        {
            OnPut();
        }
        return !_content.IsEmpty();
    }

    bool ContentStream::Put(const String::CharT* content)
    {
        _content = String(content);
        _content.shrink_to_fit();
        if (!_content.IsEmpty())
        {
            OnPut();
        }
        return !_content.IsEmpty();
    }

    const String& ContentStream::Data() const noexcept
    {
        return _content;
    }

    bool FileContentStream::ReadFromFile(const std::filesystem::path& path)
    {
        _path = path;
        _content = Utils::GetTextFileContentAs<String>(path);
        if (!_content.IsEmpty())
        {
            _content.ShrinkToFit();
        }

        return !_content.IsEmpty();
    }

    void FileContentStream::OnPut()
    {
        std::ofstream out(_path);
        if (!Verify(out.is_open()))
        {
            return;
        }

        out.write(_content.c_str(), _content.Size() * sizeof(String::CharT));
    }

} // namespace Ast