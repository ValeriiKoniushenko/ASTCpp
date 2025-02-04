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

#include "GeneratorUnit.h"

namespace Ast::Cpp
{
    bool GeneratorUnitDecl::OnEqual(const GeneratorUnit& other) const
    {
        return dynamic_cast<const GeneratorUnitDecl*>(&other);
    }

    String GeneratorUnitDecl::OnGenerate(const BaseLexer* lexer) const
    {
        return {};
    }

    String GeneratorUnitDecl::PreGenerate(const BaseLexer* lexer) const
    {
        String out;
        out += "#pragma once" + Code::Endl();
        for (const auto& incl : _includes)
        {
            out += "#include " + incl + Code::Endl();
        }
        out += Code::Endl() + Code::Endl();

        out += "namespace {}::{}{}{{}"_f << namespaceName << _nestedNamespace << Code::Endl() << Code::Endl();

        return out;
    }
    String GeneratorUnitDecl::PostGenerate(const BaseLexer* lexer) const
    {
        return "} // namespace {}::{}{}"_f << namespaceName << _nestedNamespace << Code::Endl();
    }

    void GeneratorUnitDecl::AddLocalInclude(String str)
    {
        if (str.IsEmpty())
        {
            return;
        }

        str.Trim(' ');
        str.TrimStart('<');
        str.TrimEnd('>');
        str.Trim('"');
        str = '"' + str + '"';
        _includes.push_back(std::move(str));
    }

    void GeneratorUnitDecl::AddGlobalInclude(String str)
    {
        if (str.IsEmpty())
        {
            return;
        }

        str.Trim(' ');
        str.TrimStart('<');
        str.TrimEnd('>');
        str.Trim('"');
        str = '<' + str + '>';
        _includes.push_back(std::move(str));
    }

    bool GeneratorUnitImpl::OnEqual(const GeneratorUnit& other) const
    {
        return dynamic_cast<const GeneratorUnitImpl*>(&other);
    }

    String GeneratorUnitImpl::PreGenerate(const BaseLexer* lexer) const
    {
        return "namespace {}::{}{}{{}"_f << Decl::namespaceName << _nestedNamespace << Code::Endl() << Code::Endl();
    }
    String GeneratorUnitImpl::PostGenerate(const BaseLexer* lexer) const
    {
        return "} // namespace {}::{}{}"_f << Decl::namespaceName << _nestedNamespace << Code::Endl();
    }


} // namespace Ast::Cpp