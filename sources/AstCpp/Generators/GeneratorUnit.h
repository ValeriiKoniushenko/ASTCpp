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

#include "Ast/Generators/GeneratorUnit.h"

namespace Ast::Cpp
{
    template<class FinalGeneratorT = void>
    class AbstractGeneratorUnit
    {
    public:
        virtual ~AbstractGeneratorUnit() = default;

        inline static const char* generatedSuffixImpl = ".impl";
        [[nodiscard]] virtual bool IsDeclaration() const = 0;
        [[nodiscard]] bool IsImplementation() const { return !IsDeclaration(); }
        static bool IsSelf(const Ast::GeneratorUnit::Ptr& unit)
        {
            if (Verify(!!unit, "Was passed nullptr unit"))
            {
                return dynamic_cast<const FinalGeneratorT*>(unit.get());
            }
            return false;
        };

    };

    class GeneratorUnitDecl : public Ast::GeneratorUnit, public AbstractGeneratorUnit<GeneratorUnitDecl>
    {
    public:
        using Code = ITextSourceReader::Code;

        [[nodiscard]] bool IsDeclaration() const override { return true; }

    protected:
        template<IsLexer Lexer>
        [[nodiscard]] static GeneratorUnitDecl Create(const String& nestedNamespace)
        {
            return GeneratorUnitDecl(Lexer::typeName, nestedNamespace);
        }

        [[nodiscard]] bool OnEqual(const GeneratorUnit& other) const override;
        [[nodiscard]] String OnGenerate(const BaseLexer* lexer) const override;
        [[nodiscard]] String PreGenerate(const BaseLexer* lexer) const override;
        [[nodiscard]] String PostGenerate(const BaseLexer* lexer) const override;
        [[nodiscard]] virtual String OnFinishGenerateNeededStartOfFile(const BaseLexer* lexer) const {return {}; }

        GeneratorUnitDecl(const String& type, const String& nestedNamespace)
            : Ast::GeneratorUnit(type),
              _nestedNamespace{ nestedNamespace }
        {
        }

        /**
         * @brief Will add local include. Pass str(e.g Smth.h) -> you will get #include "Smth.h"
         */
        void AddLocalInclude(String str);

        /**
         * @brief Will add global include. Pass str(e.g Smth.h) -> you will get #include <Smth.h>
         */
        void AddGlobalInclude(String str);

    protected:
        const String _nestedNamespace;
        std::vector<String> _includes;

    private:
        [[nodiscard]] String GenerateNeededStartOfFile(const BaseLexer* lexer) const;
    };

    class GeneratorUnitImpl : public Ast::GeneratorUnit, public AbstractGeneratorUnit<GeneratorUnitImpl>
    {
    public:
        using Decl = GeneratorUnitDecl;
        using Code = ITextSourceReader::Code;

    public:
        [[nodiscard]] bool IsDeclaration() const override { return false; }

    protected:
        template<IsLexer Lexer>
        [[nodiscard]] static GeneratorUnitImpl Create(const String& nestedNamespace)
        {
            return GeneratorUnitImpl(Lexer::typeName, nestedNamespace);
        }

        [[nodiscard]] bool OnEqual(const GeneratorUnit& other) const override;
        [[nodiscard]] String PreGenerate(const BaseLexer* lexer) const override;
        [[nodiscard]] String PostGenerate(const BaseLexer* lexer) const override;

        GeneratorUnitImpl(const String& type, const String& nestedNamespace)
            : Ast::GeneratorUnit(type),
              _nestedNamespace{ nestedNamespace }
        {
        }

    protected:
        const String _nestedNamespace;
    };

    // TODO: move to another file
    // TODO: create base class for it
    class FileBasedGeneratorForUnitDecl : public Utils::NotCopyableButMoveable
    {
    public:
        inline static const char* namespaceName = "Reflect";

    public:
        FileBasedGeneratorForUnitDecl() = default;
        ~FileBasedGeneratorForUnitDecl() override = default;
    };
} // namespace Ast::Cpp