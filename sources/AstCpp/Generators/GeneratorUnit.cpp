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
#if 0
    #include "GeneratorUnit.h"

namespace Ast::Cpp
{
    bool GeneratorUnitDecl::OnEqual(const GeneratorUnit& other) const
    {
        return dynamic_cast<const GeneratorUnitDecl*>(&other);
    }

    void GeneratorUnitDecl::AddLocalInclude(String str)
    {
        if (str.isEmpty())
        {
            return;
        }

        str.trim(' ');
        str.trimStart('<');
        str.trimEnd('>');
        str.trim('"');
        str = '"' + str + '"';
        _includes.push_back(std::move(str));
    }

    void GeneratorUnitDecl::AddGlobalInclude(String str)
    {
        if (str.isEmpty())
        {
            return;
        }

        str.trim(' ');
        str.trimStart('<');
        str.trimEnd('>');
        str.trim('"');
        str = '<' + str + '>';
        _includes.push_back(std::move(str));
    }

    String GeneratorUnitDecl::OnGenerate() const
    {
        return {};
    }

    String GeneratorUnitDecl::PreGenerate() const
    {
        return {};
    }

    String GeneratorUnitDecl::PostGenerate() const
    {
        return {};
    }

    bool GeneratorUnitImpl::OnEqual(const GeneratorUnit& other) const
    {
        return dynamic_cast<const GeneratorUnitImpl*>(&other);
    }

    String GeneratorUnitImpl::OnGenerate() const
    {
        return {};
    }

    String GeneratorUnitImpl::PreGenerate() const
    {
        return {};
    }

    String GeneratorUnitImpl::PostGenerate() const
    {
        return {};
    }

} // namespace Ast::Cpp
#endif
