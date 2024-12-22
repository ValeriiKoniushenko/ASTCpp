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

#include "Ast/CommonTypes.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <map>

namespace Ast
{
    class TextSourceConfig : public Core::Singleton<TextSourceConfig, ::Utils::NotCopyableAndNotMoveable>
    {
    public:
        enum class EndLineType
        {
            CR,  // \r
            LF,  // \n
            CRLF // \r\n
        };

        struct CodeStyle
        {
            int8_t indentWidth = 4;
            EndLineType endLineType = EndLineType::LF;
            bool isUseTab = true;
        };

    public:
        [[nodiscard]] CodeStyle GetCodeStyle() const { return _codeStyle; }
        void SetCodeStyle(const CodeStyle& codeStyle) { _codeStyle = codeStyle; }

        void SetEndLineType(EndLineType type) noexcept { _codeStyle.endLineType = type; }
        [[nodiscard]] EndLineType GetEndLineType() const noexcept { return _codeStyle.endLineType; }
        [[nodiscard]] const String& GetEndLine() const
        {
            static const auto cr = "\r"_atom;
            static const auto lf = "\n"_atom;
            static const auto crlf = "\r\n"_atom;

            if (_codeStyle.endLineType == EndLineType::CR)
            {
                return cr;
            }
            if (_codeStyle.endLineType == EndLineType::LF)
            {
                return lf;
            }
            return crlf;
        }

        void SetUseTab(bool isUse) noexcept { _codeStyle.isUseTab = isUse; }
        [[nodiscard]] bool IsUseTab() const noexcept { return _codeStyle.isUseTab; }
        [[nodiscard]] String GetTab(const int8_t count = 1) const
        {
            String indents;

            if (_codeStyle.isUseTab)
            {
                for (auto i = 0; i < count; ++i)
                {
                    indents += "\t";
                }
            }
            else
            {
                for (auto i = 0; i < _codeStyle.indentWidth * count; ++i)
                {
                    indents += " ";
                }
            }

            return indents;
        }

    protected:
        TextSourceConfig() = default;
        friend Singleton;

    private:
        CodeStyle _codeStyle;
    };

    struct ITextSourceReader
    {
        ITextSourceReader() = default;
        virtual ~ITextSourceReader() = default;

        struct TextSourceT
        {
            String source;
            std::map<String, uint32_t> carets;
        };

        [[nodiscard]] virtual String GetTextSource() = 0;
        [[nodiscard]] bool TextSourceWasGenerated()
        {
            return _textSourceWasGenerated;
        }

        struct Code
        {
            [[nodiscard]] static String Endl() { return TextSourceConfig::Instance().GetEndLine(); }
            [[nodiscard]] static String Tab(const int8_t count = 1) { return TextSourceConfig::Instance().GetTab(count); }
        };

    protected:
        virtual bool GenerateTextSource(TextSourceT& source)
        {
            if (_textSourceWasGenerated)
            {
                return false;
            }

            return _textSourceWasGenerated = true;
        }

    protected:
        bool _textSourceWasGenerated = false;
    };

} // namespace Ast
