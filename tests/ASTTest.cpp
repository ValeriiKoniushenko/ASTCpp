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

#define CORE_DEBUG

#include "Ast/LogCollector.h"
#include "Ast/Readers/ContentStream.h"
#include "Ast/Tree.h"
#include "AstCpp/Parser.h"
#include "AstCpp/Readers/Filters/CommentFilter.h"
#include "AstCpp/Rules/ClassRules.h"
#include "AstCpp/Rules/CommonRules.h"
#include "AstCpp/Rules/EnumClassRules.h"
#include "AstCpp/Rules/NamespaceRules.h"

#include <fstream>
#include <gtest/gtest.h>
#include <unordered_set>

namespace
{
    const char* const content = R"(#pragma   once
#include "../CommonTypes.h"

#include "Utils/CopyableAndMoveableBehaviour.h"

#include <filesystem>

CLASS(Smth)
class GlobalClass : public std::base_string<char, std::char_traits<char>, std::allocator<char>>,
    private SomeInterface<char>,
    public ElseOne, protected SomeInterface222<char>
{
public:

    class Internal
    {
        int i = 123;
    }

    int publicA = 123;
    std::string publicStr;
    void PublicFunc();
    void PublicFuncImpl(){}

protected:
    constexpr int protectedA = 123;
    std::string protectedStr;
    void ProtectedFunc();
    void ProtectedFuncImpl(){}

private:
    const int privateA = 123;
    std::string privateStr;
    void PrivateFunc();
    void PrivateFuncImpl(){}
};

namespace Ast
{
    class Vector;

    namespace Ast2::Utils
    {
        enum Color
        {
            Red,
            Green,
            Blue
        }

        enum class EType
        {
            Integer,
            Floating,
            Other
        }

        enum class EType2
        {
            Integer2 = 3 + 4,
            Floating2 = 3 + 5,
            Other2 = Floating2 * 2
        }

        class Reader final : public Utils::CopyableAndMoveable
        {
            int smthPrivate = 123;
        public:

            enum class ReaderType : long long
            {
                Smth1,
                Smth2
            }

            static int staticVar = 123;
            static const int staticConstVar = 123;
            static constexpr int staticConstexprVar = 3 * 3;
            constexpr static int staticConstexprVar = 3 * 3;

            static void SomeStaticFunc() {};

            class Temp
            {
                int temp = 213;
            };

            Reader() = default;
            ~Reader() override = default;

            bool Read(const std::filesystem::path& path);

        private:
            constexpr int a = 123;
            String _content_static;
            std::filesystem::path _path;
            std::vector<typename Toolset::Smth<int, std::is_same_v<int, double>>> _someVector;
        };

        class SmthElse : public std::base_string<char, std::char_traits<char>, std::allocator<char>>,
            public SomeInterface<char>,
            public ElseOne
        {
        public:
            int publicA = 123;
            std::string publicStr;
            void PublicFunc();
            void PublicFuncImpl(){}

        protected:
            int protectedA = 123;
            std::string protectedStr;
            void ProtectedFunc();
            void ProtectedFuncImpl(){}

        private:
            int privateA = 123;
            std::string privateStr;
            void PrivateFunc();
            void PrivateFuncImpl(){}
        };
    }

    CLASS(Smth1, Smth2)
    template<class T>
    class Vec2
    {
    public:
        T x{};
        T y{};
    }

    CLASS(Smth1,Smth2)
    template<class T>
    class PrivateVec2
    {
        T x{};
        T y{};
    }
}
)";

} // namespace


TEST(ASTTests, SimpleParse)
{
    using namespace Ast;

    Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    auto found = tree.FindIf(
        [](BaseLexer* lexer)
        {
            return lexer->GetLexerName() == "Internal";
        });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetLexerName(), "Internal");
    ASSERT_TRUE(found->HasParent());
}

TEST(ASTTests, ParentsChecking)
{
    using namespace Ast;
    Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    auto found = tree.FindIf(
        [](BaseLexer* lexer)
        {
            return lexer->GetLexerName() == "Internal";
        });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetLexerName(), "Internal");

    {
        // parent checking
        ASSERT_TRUE(found->HasParent());
        auto parent = found->GetParentLexer();
        ASSERT_TRUE(parent);
        EXPECT_EQ(parent->GetLexerName(), "GlobalClass");
    }

    {
        // parent checking
        ASSERT_TRUE(found->GetParentLexer()->HasParent());
        auto parent = found->GetParentLexer()->GetParentLexer();
        ASSERT_TRUE(parent);
        EXPECT_EQ(parent->GetLexerName(), "none"); // none == {some_file_name}
    }
}

TEST(ASTTests, DetailedLexerClassChecking)
{
    using namespace Ast;
    Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    auto found = tree.FindIf(
        [](BaseLexer* lexer)
        {
            return lexer->GetLexerName() == "Internal";
        });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetLexerName(), "Internal");
    EXPECT_FALSE(found->HasChildLexers());
    EXPECT_TRUE(found->GetReader());
    EXPECT_TRUE(found->GetOpenScope().has_value());
    EXPECT_TRUE(found->GetCloseScope().has_value());
    EXPECT_EQ(found->GetLexerType(), Cpp::ClassLexer::typeName);
    EXPECT_TRUE(found->IsTypeOf<Cpp::ClassLexer>());

    {
        auto foundNamespace = found->CastTo<Cpp::NamespaceLexer>();
        ASSERT_FALSE(foundNamespace);
    }

    {
        auto foundClass = found->CastTo<Cpp::ClassLexer>();
        ASSERT_TRUE(foundClass);
        EXPECT_FALSE(foundClass->IsFinal());
        EXPECT_FALSE(foundClass->HasClassParents());
        ASSERT_TRUE(foundClass->HasFields());
        EXPECT_EQ(1, foundClass->GetFields().size());

        const auto field = foundClass->GetFields().front();
        EXPECT_FALSE(field.isConst);
        EXPECT_FALSE(field.isConstexpr);
        EXPECT_FALSE(field.isConstinit);
        EXPECT_FALSE(field.isStatic);
        EXPECT_EQ("i", field.name);
        EXPECT_EQ("int", field.type);
        EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
    }
}

TEST(ASTTests, DetailedBiggerLexerClassChecking)
{
    using namespace Ast;
    Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    auto found = tree.FindIf(
        [](BaseLexer* lexer)
        {
            return lexer->GetLexerName() == "GlobalClass";
        });

    ASSERT_TRUE(found);
    ASSERT_EQ(found->GetLexerName(), "GlobalClass");
    ASSERT_TRUE(found->HasChildLexers());

    {
        auto lexer = found->CastTo<Cpp::ClassLexer>();
        ASSERT_TRUE(lexer);
        EXPECT_FALSE(lexer->IsFinal());
        EXPECT_TRUE(lexer->HasClassParents());
        ASSERT_TRUE(lexer->HasFields());
        EXPECT_EQ(6, lexer->GetFields().size());

        // field #1
        {
            const auto field = lexer->GetFields()[0];
            EXPECT_FALSE(field.isConst);
            EXPECT_FALSE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("publicA", field.name);
            EXPECT_EQ("int", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Public, field.accessSpecifier);
        }

        // field #2
        {
            const auto field = lexer->GetFields()[1];
            EXPECT_FALSE(field.isConst);
            EXPECT_FALSE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("publicStr", field.name);
            EXPECT_EQ("std::string", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Public, field.accessSpecifier);
        }

        // field #3
        {
            const auto field = lexer->GetFields()[2];
            EXPECT_FALSE(field.isConst);
            EXPECT_TRUE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("protectedA", field.name);
            EXPECT_EQ("int", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Protected, field.accessSpecifier);
        }

        // field #4
        {
            const auto field = lexer->GetFields()[3];
            EXPECT_FALSE(field.isConst);
            EXPECT_FALSE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("protectedStr", field.name);
            EXPECT_EQ("std::string", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Protected, field.accessSpecifier);
        }

        // field #5
        {
            const auto field = lexer->GetFields()[4];
            EXPECT_TRUE(field.isConst);
            EXPECT_FALSE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("privateA", field.name);
            EXPECT_EQ("int", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
        }

        // field #6
        {
            const auto field = lexer->GetFields()[5];
            EXPECT_FALSE(field.isConst);
            EXPECT_FALSE(field.isConstexpr);
            EXPECT_FALSE(field.isConstinit);
            EXPECT_FALSE(field.isStatic);
            EXPECT_EQ("privateStr", field.name);
            EXPECT_EQ("std::string", field.type);
            EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
        }

        auto parents = lexer->GetClassParents();
        ASSERT_EQ(4, parents.size());
        // parent #1
        {
            auto parent = parents[0];
            EXPECT_EQ(Cpp::ClassLexer::InheritanceType::Public, parent.type);
            EXPECT_EQ("std::base_string<char, std::char_traits<char>, std::allocator<char>>", parent.name);
        }

        // parent #2
        {
            auto parent = parents[1];
            EXPECT_EQ(Cpp::ClassLexer::InheritanceType::Private, parent.type);
            EXPECT_EQ("SomeInterface<char>", parent.name);
        }

        // parent #3
        {
            auto parent = parents[2];
            EXPECT_EQ(Cpp::ClassLexer::InheritanceType::Public, parent.type);
            EXPECT_EQ("ElseOne", parent.name);
        }

        // parent #4
        {
            auto parent = parents[3];
            EXPECT_EQ(Cpp::ClassLexer::InheritanceType::Protected, parent.type);
            EXPECT_EQ("SomeInterface222<char>", parent.name);
        }
    }
}

TEST(ASTTests, ScopeChecking)
{
    using namespace Ast;
    Cpp::ClassLexer::Ptr lexer;

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        auto found = tree.FindIf(
            [](BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Internal";
            });

        ASSERT_TRUE(found);
        lexer = found->CastTo<Cpp::ClassLexer>();
        ASSERT_TRUE(lexer);
    }

    ASSERT_TRUE(lexer);
    EXPECT_EQ(lexer->GetLexerName(), "Internal");
    ASSERT_TRUE(lexer->HasParent());
}

TEST(ASTTests, GetRootLexer)
{
    using namespace Ast;
    Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    auto found = tree.FindIf(
        [](BaseLexer* lexer)
        {
            return lexer->GetLexerName() == "Internal";
        });

    ASSERT_TRUE(found);
    const auto root = found->GetRootLexer();
    ASSERT_TRUE(root);
    EXPECT_EQ("none", root->GetLexerName());
}

TEST(ASTTests, LexerConstAndNonConstMiscTests)
{
    using namespace Ast;
    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        int lexersCount = 0;
        tree.ForEach(
            [&lexersCount](BaseLexer* lexer, auto)
            {
                ++lexersCount;
            });

        EXPECT_GT(lexersCount, 0);
    }

    {
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        int lexersCount = 0;
        tree.ForEach(
            [&lexersCount](const BaseLexer* lexer, auto)
            {
                ++lexersCount;
            });

        EXPECT_GT(lexersCount, 0);
    }

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Internal";
            });

        ASSERT_TRUE(found);
    }

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Internal";
            });

        ASSERT_TRUE(found);
    }

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "GlobalClass";
            });

        ASSERT_TRUE(found);
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::EnumClassLexer>().size(), 0);
    }

    {
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "GlobalClass";
            });

        ASSERT_TRUE(found);
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::EnumClassLexer>().size(), 0);
    }

    {
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIfAs<Cpp::ClassLexer>(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "GlobalClass";
            });

        ASSERT_TRUE(found);
        EXPECT_EQ(Cpp::ClassLexer::typeName, found->GetLexerType());
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Cpp::EnumClassLexer>().size(), 0);
    }
}

TEST(ASTTests, LexerConstAndNonConstMiscTests2)
{
    using namespace Ast;

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Ast2::Utils";
            });

        ASSERT_TRUE(found);
        for (auto&& child : found->GetChildLexers<Cpp::ClassLexer>())
        {
            EXPECT_TRUE(child->IsTypeOf<Cpp::ClassLexer>());
        }
        for (auto&& child : found->GetChildLexers<Cpp::NamespaceLexer>())
        {
            EXPECT_TRUE(child->IsTypeOf<Cpp::NamespaceLexer>());
        }
    }

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

        const auto found = tree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Reader";
            });

        ASSERT_TRUE(found);
        EXPECT_EQ("Ast::Ast2::Utils::Reader", found->GetFullPath().first);
    }
}

TEST(ASTTests, TryToGetLexerByXXX)
{
    using namespace Ast;
    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        auto found = tree.FindFirstByName<Cpp::ClassLexer>("GlobalClass");
        ASSERT_TRUE(found);
    }

    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        auto found = tree.FindFirstByName<Cpp::ClassLexer>("1111111111111111");
        ASSERT_FALSE(found);
    }

    {
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        const auto found = tree.FindFirstByName<Cpp::ClassLexer>("GlobalClass");
        ASSERT_TRUE(found);
    }
    {
        Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("GlobalClass");
        ASSERT_TRUE(found);
    }

    {
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        const auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("GlobalClass");
        ASSERT_TRUE(found);
    }
}

TEST(ASTTests, CheckRulesForClass)
{
    {
        using namespace Ast;
        const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });
        const auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("Vec2");
        ASSERT_TRUE(found);
        LogCollector logCollector;
        found->IsCorrespondingToRule(Cpp::NameRule(R"([A-Z]\w+)"), logCollector);
    }
}

TEST(ASTTests, Marks)
{
    using namespace Ast;
    const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    {
        const auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("GlobalClass");
        ASSERT_TRUE(found);
        EXPECT_FALSE(found->IsTemplate());
        ASSERT_TRUE(found->IsMarked());
        EXPECT_EQ("CLASS", found->GetMark()->rule);
        EXPECT_EQ("Smth", found->GetMark()->params.front());
    }

    {
        const auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("Vec2");
        ASSERT_TRUE(found);
        EXPECT_TRUE(found->IsTemplate());
        ASSERT_TRUE(found->IsMarked());
        EXPECT_EQ("CLASS", found->GetMark()->rule);
        EXPECT_EQ("Smth1", found->GetMark()->params.front());
        EXPECT_EQ("Smth2", found->GetMark()->params.back());
    }

    {
        const auto found = tree.FindFirstByNameAs<Cpp::ClassLexer>("PrivateVec2");
        ASSERT_TRUE(found);
        EXPECT_TRUE(found->IsTemplate());
        ASSERT_TRUE(found->IsMarked());
        EXPECT_EQ("CLASS", found->GetMark()->rule);
        EXPECT_EQ("Smth1", found->GetMark()->params.front());
        EXPECT_EQ("Smth2", found->GetMark()->params.back());
    }
}

TEST(ASTTests, ApplyClassRule)
{
    using namespace Ast;
    const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    const auto foundClass = tree.FindFirstByNameAs<Cpp::ClassLexer>("GlobalClass");
    ASSERT_TRUE(foundClass);

    LogCollector logCollector;
    ASSERT_TRUE(foundClass->IsCorrespondingToRule(Cpp::Class::BaseRule{}, logCollector));

    {
        Cpp::NameRule nameRule(R"(([A-Z_]\w*)+)");
        nameRule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_TRUE(foundClass->IsCorrespondingToRule(nameRule, logCollector));
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }

    {
        Cpp::NameRule rule(R"(([a-z_]\w*)+)");
        rule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_FALSE(foundClass->IsCorrespondingToRule(rule, logCollector));
        EXPECT_TRUE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }

    {
        Cpp::LineCountRule rule(1);
        rule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_FALSE(foundClass->IsCorrespondingToRule(rule, logCollector));
        EXPECT_TRUE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }

    {
        Cpp::LineCountRule rule(300);
        rule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_TRUE(foundClass->IsCorrespondingToRule(rule, logCollector));
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }
}

TEST(ASTTests, ApplyEnumClassRule)
{
    using namespace Ast;
    const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    const auto found = tree.FindFirstByNameAs<Cpp::EnumClassLexer>("EType");
    ASSERT_TRUE(found);

    LogCollector logCollector;
    ASSERT_TRUE(found->IsCorrespondingToRule(Cpp::EnumClass::BaseRule{}, logCollector));

    {
        Cpp::NameRule nameRule(R"(([A-Z_]\w*)+)");
        nameRule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_TRUE(found->IsCorrespondingToRule(nameRule, logCollector));
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }

    {
        Cpp::NameRule rule(R"(([a-z_]\w*)+)");
        rule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_FALSE(found->IsCorrespondingToRule(rule, logCollector));
        EXPECT_TRUE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }
}

TEST(ASTTests, ApplyNamespaceRule)
{
    using namespace Ast;
    const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    const auto found = tree.FindFirstByNameAs<Cpp::NamespaceLexer>("Ast");
    ASSERT_TRUE(found);

    LogCollector logCollector;
    ASSERT_TRUE(found->IsCorrespondingToRule(Cpp::Namespace::BaseRule{}, logCollector));

    {
        Cpp::NameRule nameRule(R"(([A-Z_]\w*)+)");
        nameRule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_TRUE(found->IsCorrespondingToRule(nameRule, logCollector));
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }

    {
        Cpp::NameRule rule(R"(([a-z_]\w*)+)");
        rule.OverrideLogType(LogCollector::LogType::Warning);
        EXPECT_FALSE(found->IsCorrespondingToRule(rule, logCollector));
        EXPECT_TRUE(logCollector.HasAny<LogCollector::LogType::Warning>());
        EXPECT_FALSE(logCollector.HasAny<LogCollector::LogType::Error>());
        logCollector.ClearLogs();
    }
}

TEST(ASTTests, GenerateNewClass)
{
    using namespace Ast;

    auto stream = ContentStream::Create();
    ASSERT_TRUE(stream);

    auto myFile = FileLexer::Create(stream);
    ASSERT_TRUE(myFile);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    EXPECT_EQ("smth.cpp", myFile->GetFileName());
    EXPECT_TRUE(myFile->HasPragmaOnce());

    auto myClass = Cpp::ClassLexer::Create(stream);
    ASSERT_TRUE(myClass);
    myClass->SetLexerName("MyClass");

    EXPECT_EQ("MyClass", myClass->GetLexerName());

    myClass->ForceSetParent(myFile);

    EXPECT_EQ(myClass->GetParentLexer(), myFile);
}

TEST(ASTTests, BuildNewSimpleTree)
{
    using namespace Ast;
    auto stream = ContentStream::Create();
    ASSERT_TRUE(stream);

    auto myFile = FileLexer::Create(stream);
    ASSERT_TRUE(myFile);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    EXPECT_EQ("smth.cpp", myFile->GetFileName());
    EXPECT_TRUE(myFile->HasPragmaOnce());

    auto myClass = Cpp::ClassLexer::Create(stream);
    ASSERT_TRUE(myClass);
    ASSERT_TRUE(myClass->GetLexerType() == Cpp::ClassLexer::typeName);

    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });

    EXPECT_EQ("MyClass", myClass->GetLexerName());
    ASSERT_EQ(2, myClass->GetFields().size());

    EXPECT_EQ("int", myClass->GetFields()[0].type);
    EXPECT_EQ("age", myClass->GetFields()[0].name);
    EXPECT_EQ("", myClass->GetFields()[0].value);
    EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Private, myClass->GetFields()[0].accessSpecifier);

    EXPECT_EQ("std::string", myClass->GetFields()[1].type);
    EXPECT_EQ("name", myClass->GetFields()[1].name);
    EXPECT_EQ("\"Mark\"", myClass->GetFields()[1].value);
    EXPECT_EQ(Cpp::ClassLexer::AccessSpecifier::Private, myClass->GetFields()[1].accessSpecifier);

    myClass->ForceSetParent(myFile);

    EXPECT_EQ(myClass->GetParentLexer(), myFile);

    Tree astFileTree(myFile);

    auto found = astFileTree.FindFirstByNameAs<Cpp::ClassLexer>("MyClass");
    ASSERT_TRUE(found);
    ASSERT_EQ(2, found->GetFields().size());
}

TEST(ASTTests, CreateTreeFromClass)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myFile);

    Tree tree(myFile);
    auto source = tree.GetTextSource();
    {
        const Cpp::Parser parser{ ContentStream(source.c_str()) };

        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Error>());
        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Warning>());

        const Tree newTree = BaseTree::From(parser);
        auto found = newTree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "MyClass";
            });
        ASSERT_TRUE(found);
        EXPECT_EQ("MyClass", found->GetLexerName());
    }
}

TEST(ASTTests, CreateDifficultTreeFromClass)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myFile);

    Tree tree(myFile);
    auto source = tree.GetTextSource();
    {
        const Cpp::Parser parser{ ContentStream(source.c_str()) };

        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Error>());
        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Warning>());

        const Tree newTree = BaseTree::From(parser);
        auto found = newTree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "MyClass";
            });
        ASSERT_TRUE(found);
        ASSERT_EQ("MyClass", found->GetLexerName());
        std::cout << source.c_str() << std::endl;
    }
}

TEST(ASTTests, CreateTreeWithEnumClass)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myEnum = Cpp::EnumClassLexer::Create(stream);
    myEnum->SetLexerName("MyEnum");
    ASSERT_TRUE(myEnum->AddConstant("Hello"));
    ASSERT_TRUE(myEnum->AddConstant("World"));
    ASSERT_TRUE(myEnum->AddConstant("World1", 333));
    ASSERT_EQ(3, myEnum->GetConstants().size());

    EXPECT_EQ(0, myEnum->GetConstant("Hello").value.value());
    EXPECT_EQ("World", myEnum->GetConstant(1).name);

    myEnum->ForceSetParent(myFile);

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myFile);

    Tree tree(myFile);
    auto source = tree.GetTextSource();
    {
        const Cpp::Parser parser{ ContentStream(source.c_str()) };

        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Error>());
        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Warning>());

        const Tree newTree = BaseTree::From(parser);
        auto found = newTree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "MyEnum";
            });
        ASSERT_TRUE(found);
        ASSERT_EQ("MyEnum", found->GetLexerName());
        std::cout << source.c_str() << std::endl;
    }
}

TEST(ASTTests, CreateTreeWithEnumClassAndNamespaceLexer)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myNamespace = Cpp::NamespaceLexer::Create(stream);
    myNamespace->SetNamespace("Some::Ns");
    myNamespace->ForceSetParent(myFile);

    auto myEnum = Cpp::EnumClassLexer::Create(stream);
    myEnum->SetLexerName("MyEnum");
    ASSERT_TRUE(myEnum->AddConstant("Hello"));
    ASSERT_TRUE(myEnum->AddConstant("World"));
    ASSERT_TRUE(myEnum->AddConstant("World1", 333));
    ASSERT_EQ(3, myEnum->GetConstants().size());

    EXPECT_EQ(0, myEnum->GetConstant("Hello").value.value());
    EXPECT_EQ("World", myEnum->GetConstant(1).name);

    myEnum->ForceSetParent(myNamespace);

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myNamespace);

    Tree tree(myFile);
    auto source = tree.GetTextSource();
    {
        const Cpp::Parser parser{ ContentStream(source.c_str()) };

        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Error>());
        EXPECT_FALSE(parser.GetLogCollector().HasAny<LogCollector::LogType::Warning>());

        const Tree newTree = BaseTree::From(parser);
        auto found = newTree.FindIf(
            [](const BaseLexer* lexer)
            {
                return lexer->GetLexerName() == "Some::Ns";
            });
        ASSERT_TRUE(found);
        ASSERT_EQ("Some::Ns", found->GetLexerName());
        std::cout << source.c_str() << std::endl;
    }
}

TEST(ASTTests, GenerateNewClassAndFlushToStream)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myNamespace = Cpp::NamespaceLexer::Create(stream);
    myNamespace->SetNamespace("Some::Ns");
    myNamespace->ForceSetParent(myFile);

    auto myEnum = Cpp::EnumClassLexer::Create(stream);
    myEnum->SetLexerName("MyEnum");
    ASSERT_TRUE(myEnum->AddConstant("Hello"));
    ASSERT_TRUE(myEnum->AddConstant("World"));
    ASSERT_TRUE(myEnum->AddConstant("World1", 333));
    ASSERT_EQ(3, myEnum->GetConstants().size());

    EXPECT_EQ(0, myEnum->GetConstant("Hello").value.value());
    EXPECT_EQ("World", myEnum->GetConstant(1).name);

    myEnum->ForceSetParent(myNamespace);

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myNamespace);

    Tree tree(myFile);
    tree.FlushToStream();
}

TEST(ASTTests, GenerateNewClassAndFlushToFileStream)
{
    using namespace Ast;
    auto stream = FileContentStream::Create();
    stream->SetFilePath("smth.cpp");

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myNamespace = Cpp::NamespaceLexer::Create(stream);
    myNamespace->SetNamespace("Some::Ns");
    myNamespace->ForceSetParent(myFile);

    auto myEnum = Cpp::EnumClassLexer::Create(stream);
    myEnum->SetLexerName("MyEnum");
    ASSERT_TRUE(myEnum->AddConstant("Hello"));
    ASSERT_TRUE(myEnum->AddConstant("World"));
    ASSERT_TRUE(myEnum->AddConstant("World1", 333));
    ASSERT_EQ(3, myEnum->GetConstants().size());

    EXPECT_EQ(0, myEnum->GetConstant("Hello").value.value());
    EXPECT_EQ("World", myEnum->GetConstant(1).name);

    myEnum->ForceSetParent(myNamespace);

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myNamespace);

    Tree tree(myFile);
    tree.FlushToStream();
}

TEST(ASTTests, GenerateNewDifficultTreeAndFlushToFileStream)
{
    using namespace Ast;
    auto stream = ContentStream::Create();

    auto myFile = FileLexer::Create(stream);
    myFile->SetFileName("smth.cpp");
    myFile->SetPragmaOnce();

    auto myNamespace = Cpp::NamespaceLexer::Create(stream);
    myNamespace->SetNamespace("Some::Ns");
    myNamespace->ForceSetParent(myFile);

    auto myEnum = Cpp::EnumClassLexer::Create(stream);
    myEnum->SetLexerName("MyEnum");
    ASSERT_TRUE(myEnum->AddConstant("Hello"));
    ASSERT_TRUE(myEnum->AddConstant("World"));
    ASSERT_TRUE(myEnum->AddConstant("World1", 333));
    ASSERT_EQ(3, myEnum->GetConstants().size());

    EXPECT_EQ(0, myEnum->GetConstant("Hello").value.value());
    EXPECT_EQ("World", myEnum->GetConstant(1).name);

    myEnum->ForceSetParent(myNamespace);

    auto myClass = Cpp::ClassLexer::Create(stream);
    myClass->SetLexerName("MyClass");
    myClass->AddField({ "int", "age", Cpp::ClassLexer::AccessSpecifier::Private });
    myClass->AddField({ "std::string", "name", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit1"));
    myClass->AddClassParents(Cpp::ClassLexer::ParentUnit("SomeParentUnit2"));
    myClass->ForceSetParent(myNamespace);

    auto myClass2 = Cpp::ClassLexer::Create(stream);
    myClass2->SetLexerName("myClass2");
    myClass2->AddField({ "float", "smth", Cpp::ClassLexer::AccessSpecifier::Private, "123.312f" });
    myClass2->AddField({ "std::vector<int>", "smthelse", Cpp::ClassLexer::AccessSpecifier::Private, "\"Mark\"" });
    myClass2->AddClassParents(Cpp::ClassLexer::ParentUnit("SSSS"));
    myClass2->ForceSetParent(myClass);

    Tree tree(myFile);
    tree.FlushToStream();

    std::cout << stream->Data().c_str() << std::endl;
}

namespace
{
    void Pred1(const Ast::BaseLexer*)
    {

    }
    void Pred2(const Ast::BaseLexer*, Ast::Tree<Ast::FileLexer>::Params)
    {

    }
}

TEST(ASTTests, TreeForEach)
{
    using namespace Ast;
    const Tree tree = BaseTree::From(Cpp::Parser{ ContentStream(content) });

    {
        std::vector<Core::StringAtom> strings;
        auto pred = [&](const auto* lexer)
        {
            strings.push_back(lexer->GetLexerName());
        };

        tree.ForEach(pred);

        EXPECT_GT(strings.size(), 1);
    }

    {
        tree.ForEach(Pred1);
        tree.ForEach(Pred2);
    }
}
