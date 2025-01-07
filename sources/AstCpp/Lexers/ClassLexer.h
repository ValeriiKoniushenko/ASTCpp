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

#include "Ast/Lexers/BaseLexer.h"
#include "Core/Enum.h"

namespace Ast
{
    class ContentStream;
} // namespace Ast

class Temp
{
public:
    enum Type
    {
        One,
        Two,
        Three
    };
};

namespace Ast::Cpp
{
    class ClassLexer;

    template<class T>
    concept IsClassLexerBase = std::is_base_of_v<ClassLexer, T> || std::is_same_v<T, ClassLexer>;

    class ClassLexer final : public BaseLexer
    {
    public:
        AST_CLASS(ClassLexer)
        inline static const String marker = "CLASS";

        CreateEnum(InheritanceType, int, Public, Protected, Private);

        // TODO: change it to the lexer
        struct TemplateUnit : public ITextSourceReader
        {
            [[nodiscard]] String GetTextSource() override { return expression; }

            String expression;
        };

        // TODO: change it to the lexer
        struct ParentUnit : public ITextSourceReader
        {
            ParentUnit() = default;
            explicit ParentUnit(const String& name)
                : type{ InheritanceType::Public },
                  name{ name }
            {
            }
            [[nodiscard]] String GetTextSource() override { return String(type.ToStr()).ToLowerCase() + " " + name; }

            InheritanceType type = InheritanceType::Private;
            String name;
        };

        CreateEnum(AccessSpecifier, int, Public, Protected, Private);

        // TODO: change it to the lexer
        struct Field : public ITextSourceReader
        {
            Field() = default;
            Field(const String& type, const String& name, AccessSpecifier accessSpecifier, const String& value = ""_atom)
                : name{ name },
                  type{ type },
                  value{ value },
                  accessSpecifier{ accessSpecifier }
            {
            }

            [[nodiscard]] String GetTextSource() override;

            bool isConst = false;
            bool isConstexpr = false;
            bool isConstinit = false;
            bool isStatic = false;
            // Now, this field will not read from a file, but possible to set for writing back
            bool isInline = false;

            String name;
            String type;
            String value;
            AccessSpecifier accessSpecifier = AccessSpecifier::Private;
        };

        struct Method : public ITextSourceReader
        {
            [[nodiscard]] String GetTextSource() override;

            String comment;
            String header;
            String body;
        };

    public:
        inline static const auto typeName = "class"_atom;

        ~ClassLexer() override = default;

        [[nodiscard]] static Ptr Create(const ContentStream::Ptr& fileReader) { return { new ClassLexer(fileReader) }; }

        [[nodiscard]] bool IsFinal() const noexcept { return _hasFinal; }
        void SetFinal(bool value = true) noexcept { _hasFinal = value; }

        [[nodiscard]] bool IsTemplate() const noexcept { return _isTemplate; }
        void SetHasTemplate(bool value = true) noexcept { _isTemplate = value; }

        [[nodiscard]] std::vector<TemplateUnit>& GetTemplate() { return _templateUnits; }
        [[nodiscard]] const std::vector<TemplateUnit>& GetTemplate() const { return _templateUnits; }
        [[nodiscard]] bool HasTemplate() const noexcept { return _templateUnits.size(); }
        bool AddTemplate(TemplateUnit template_);

        [[nodiscard]] std::vector<ParentUnit>& GetClassParents() { return _parents; }
        [[nodiscard]] const std::vector<ParentUnit>& GetClassParents() const { return _parents; }
        [[nodiscard]] bool HasClassParents() const noexcept { return _parents.size(); }
        bool AddClassParents(ParentUnit parent);

        [[nodiscard]] std::vector<Field>& GetFields() { return _fields; }
        [[nodiscard]] const std::vector<Field>& GetFields() const { return _fields; }
        [[nodiscard]] bool HasFields() const noexcept { return _fields.size(); }
        bool AddField(Field field);

        [[nodiscard]] std::vector<Method>& GetMethods() { return _methods; }
        [[nodiscard]] const std::vector<Method>& GetMethods() const { return _methods; }
        [[nodiscard]] bool HasMethods() const noexcept { return _methods.size(); }
        bool AddMethod(Method method);

        bool GenerateTextSource(TextSourceT& source) override;

    protected:
        explicit ClassLexer(const ContentStream::Ptr& fileReader);

        bool DoParse(LogCollector& logCollector) override;
        bool DoScopeParse(LogCollector& logCollector) override;
        bool DoMarkingParse(LogCollector& logCollector) override;
        bool DoPostParse(LogCollector& logCollector) override;

    private:
        void TryToFindTemplate(LogCollector& logCollector);
        void RecognizeFields(LogCollector& logCollector);
        void RemoveNestedScopes(String& body);

        void IterateOverChilds(AccessSpecifier accessSpecifier, std::function<void(ITextSourceReader&)>&& callback);

    private:
        bool _hasFinal = false;
        bool _isTemplate = false;
        std::vector<TemplateUnit> _templateUnits;
        std::vector<ParentUnit> _parents;
        std::vector<Field> _fields;
        std::vector<Method> _methods;

        template<IsClassLexerBase>
        friend class ClassLexerModifier;
    };

} // namespace Ast::Cpp