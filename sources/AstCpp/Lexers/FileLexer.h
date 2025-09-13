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

#include "Ast/Lexers/FileLexer.h"

namespace Ast::Cpp
{

    class FileLexer final : public Ast::FileLexer
    {
    public:
        ~FileLexer() override = default;

        [[nodiscard]] static Ptr Create(const ContentStream::Ptr& stream) { return { new FileLexer(stream) }; }

        void SetPragmaOnce(bool value = true) { _hasPragmaOnce = value; }
        [[nodiscard]] bool HasPragmaOnce() const noexcept { return _hasPragmaOnce; }

        bool GenerateTextSource(TextSourceT& source) override;

    private:
        void OnPutAdditionalInfoToXml(Xml& xml, XmlNode* output) const override;
        bool DoPostParse() override;

        explicit FileLexer(const ContentStream::Ptr& stream)
            : Ast::FileLexer(stream)
        {
        }

    private:
        bool _hasPragmaOnce = false;
    };

} // namespace Ast::Cpp
