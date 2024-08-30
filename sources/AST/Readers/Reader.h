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

namespace Ast
{

    struct ContentFilter : public Utils::CopyableAndMoveable
    {
        virtual void MakeTransform(String& content) = 0;
    protected:
        ContentFilter() = default;
    };

    template<class T>
    concept IsContentFilter = std::is_base_of_v<ContentFilter, T>;

    class Reader : public Utils::CopyableAndMoveable,  public boost::intrusive_ref_counter<Reader>
    {
    public:
        using Ptr = boost::intrusive_ptr<Reader>;
        using CPtr = boost::intrusive_ptr<const Reader>;

        Reader() = default;
        ~Reader() override = default;

        bool Read(const String::CharT* content);
        [[nodiscard]] const String& Data() const noexcept;

        template<IsContentFilter ...Filter>
        void ApplyFilters()
        {
            (Filter{}.MakeTransform(_content), ...);
        }

    protected:
        String _content;
    };

} // namespace Ast