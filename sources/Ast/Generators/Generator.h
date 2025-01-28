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

#include "Ast/LogCollector.h"
#include "Ast/ProjectTree.h"
#include "GeneratorUnit.h"

namespace Ast
{

    class Generator : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<Generator>
    {
    public:
        AST_CLASS(Generator)

        struct FileInfo
        {
            CreateEnum(Action, int, None, Regenerate, DoNothing);

            std::filesystem::path path;
            Action action = Action::None;
            String info;
            std::vector<BaseLexer::Ptr> participantLexers;
        };

        using GeneratorContainerT = std::unordered_set<GeneratorUnit::Ptr, GeneratorUnit::HasherPtr>;

    public:
        Generator() = default;
        ~Generator() override = default;

        void SetTargetProject(const ProjectTree::Ptr& project);
        virtual void Generate();
        [[nodiscard]] bool IsNeedRegenerate() const;
        void ForEachOverRegenerateableUnits(std::function<void(const ProjectTree::Unit*)> callback) const;
        void ForEachOverRegenerateableUnits(std::function<void(ProjectTree::Unit*)> callback);

        [[nodiscard]] const GeneratorContainerT& GetGeneratorUnits() const noexcept { return _generatorUnits; }
        [[nodiscard]] GeneratorUnit::CPtr GetGeneratorUnitFor(const String& type) const;
        [[nodiscard]] GeneratorUnit::CPtr GetGeneratorUnitFor(const BaseLexer& lexer) const;
        [[nodiscard]] GeneratorUnit::CPtr GetGeneratorUnitFor(const BaseLexer::Ptr& lexer) const;

    protected:
        ProjectTree::Ptr _projectTree;
        GeneratorContainerT _generatorUnits;
    };

} // namespace Ast
