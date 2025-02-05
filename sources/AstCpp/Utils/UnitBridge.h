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
#include "Ast/ProjectTree.h"
#include "AstCpp/Generators/GeneratorUnit.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

namespace Ast::Cpp
{

    template<bool IsConst>
    class BaseUnitBridge : public Utils::CopyableAndMoveable
    {
    public:
        using Unit = Ast::ProjectTree::Unit;
        using UnitPtr = Ast::ProjectTree::Unit::AdaptiveRawPtr<IsConst>;

    public:
        explicit BaseUnitBridge(UnitPtr unit)
            : _unit(unit)
        {
        }

        [[nodiscard]] bool IsValid() const { return _unit; }

        [[nodiscard]] std::filesystem::path GetGeneratedSiblingImplFilePath()
        {
            if (!IsValid())
            {
                return {};
            }

            auto path = _unit->GetGeneratedSiblingFilePath();
            const auto mainExt = path.extension();
            path.replace_extension("");
            if (!path.extension().empty() && path.extension().string() == ProjectTree::Unit::generatedSuffixDecl)
            {
                String finalExt = AbstractGeneratorUnit<>::generatedSuffixImpl;
                finalExt += ProjectTree::Unit::generatedSuffixDecl;
                finalExt += mainExt.string();

                path.replace_extension(finalExt.c_str());
                return path;
            }

            return {};
        }

    protected:
        UnitPtr _unit;
    };

    using UnitBridge = BaseUnitBridge<false>;
    using ConstUnitBridge = BaseUnitBridge<true>;

} // namespace Ast::Cpp
