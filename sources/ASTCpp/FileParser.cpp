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

#include "ASTCpp/FileParser.h"

#include "Readers/ClassReader.h"
#include "Readers/NamespaceReader.h"
#include "Readers/EnumClassReader.h"

namespace Ast::Cpp
{

    bool FileParser::Parse(const Ast::FileReader& file)
    {
        ReadAs<NamespaceLexer, NamespaceReader>(_namespaceLexers, file);
        ReadAs<ClassLexer, ClassReader>(_classLexers, file);
        ReadAs<EnumClassLexer, EnumClassReader>(_enumClassLexers, file);

        return true;
    }

} // namespace Ast::Cpp