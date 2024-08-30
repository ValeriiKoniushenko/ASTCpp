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

#include "AST/ASTFileTree.h"
#include "AST/LogCollector.h"
#include "AST/Readers/Reader.h"
#include "ASTCpp/FileParser.h"
#include "ASTCpp/Readers/Filters/CommentFilter.h"

#include <gtest/gtest.h>

namespace
{
    const char* const content = R"(
#include "../CommonTypes.h"

#include "Utils/CopyableAndMoveableBehaviour.h"

#include <filesystem>

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

    template<class T>
    class Vec2
    {
    public:
        T x{};
        T y{};
    }

    template<class T>
    class PrivateVec2
    {
        T x{};
        T y{};
    }
}
)";

    Ast::ASTFileTree GetASTFileTree(Ast::LogCollector& logCollector)
    {
        Ast::Reader::Ptr reader = new Ast::Reader;
        reader->Read(content);
        reader->ApplyFilters<Ast::Cpp::CommentFilter>();

        Ast::ASTFileTree tree(reader);
        tree.ParseUsing<Ast::Cpp::FileParser>(logCollector);

        return tree;
    }

} // namespace


TEST(ASTTests, SimpleParse)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    EXPECT_FALSE(logCollector.HasAny<Ast::LogCollector::LogType::Error>());
}

TEST(ASTTests, SimpleGettingLexer)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "Internal";
    });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetName(), "Internal");
    ASSERT_TRUE(found->HasParent());
}

TEST(ASTTests, ParentsChecking)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "Internal";
    });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetName(), "Internal");

    {
        // parent checking
        ASSERT_TRUE(found->HasParent());
        auto parent = found->GetParentLexer();
        ASSERT_TRUE(parent);
        EXPECT_EQ(parent->GetName(), "GlobalClass");
    }

    {
        // parent checking
        ASSERT_TRUE(found->GetParentLexer()->HasParent());
        auto parent = found->GetParentLexer()->GetParentLexer();
        ASSERT_TRUE(parent);
        EXPECT_EQ(parent->GetName(), "none"); // none == {some_file_name}
    }
}

TEST(ASTTests, DetailedLexerClassChecking)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "Internal";
    });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetName(), "Internal");
    EXPECT_FALSE(found->HasChildLexers());
    EXPECT_TRUE(found->GetReader());
    EXPECT_TRUE(found->GetOpenScope().has_value());
    EXPECT_TRUE(found->GetCloseScope().has_value());
    EXPECT_EQ(found->GetLexerType(), Ast::Cpp::ClassLexer::typeName);
    EXPECT_TRUE(found->IsTypeOf<Ast::Cpp::ClassLexer>());

    {
        auto foundNamespace = found->CastTo<Ast::Cpp::NamespaceLexer>();
        ASSERT_FALSE(foundNamespace);
    }

    {
        auto foundClass = found->CastTo<Ast::Cpp::ClassLexer>();
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
        EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
    }
}

TEST(ASTTests, DetailedBiggerLexerClassChecking)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "GlobalClass";
    });

    ASSERT_TRUE(found);
    ASSERT_EQ(found->GetName(), "GlobalClass");
    ASSERT_TRUE(found->HasChildLexers());

    {
        auto lexer = found->CastTo<Ast::Cpp::ClassLexer>();
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Public, field.accessSpecifier);
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Public, field.accessSpecifier);
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Protected, field.accessSpecifier);
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Protected, field.accessSpecifier);
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
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
            EXPECT_EQ(Ast::Cpp::ClassLexer::AccessSpecifier::Private, field.accessSpecifier);
        }

        auto parents = lexer->GetClassParents();
        ASSERT_EQ(4, parents.size());
        // parent #1
        {
            auto parent = parents[0];
            EXPECT_EQ(Ast::Cpp::ClassLexer::InheritanceType::Public, parent.type);
            EXPECT_EQ("std::base_string<char, std::char_traits<char>, std::allocator<char>>", parent.name);
        }

        // parent #2
        {
            auto parent = parents[1];
            EXPECT_EQ(Ast::Cpp::ClassLexer::InheritanceType::Private, parent.type);
            EXPECT_EQ("SomeInterface<char>", parent.name);
        }

        // parent #3
        {
            auto parent = parents[2];
            EXPECT_EQ(Ast::Cpp::ClassLexer::InheritanceType::Public, parent.type);
            EXPECT_EQ("ElseOne", parent.name);
        }

        // parent #4
        {
            auto parent = parents[3];
            EXPECT_EQ(Ast::Cpp::ClassLexer::InheritanceType::Protected, parent.type);
            EXPECT_EQ("SomeInterface222<char>", parent.name);
        }
    }
}

TEST(ASTTests, ScopeChecking)
{
    Ast::Cpp::ClassLexer::Ptr lexer;

    {
        Ast::LogCollector logCollector;
        auto tree = GetASTFileTree(logCollector);

        auto found = tree.FindIf([](Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "Internal";
        });

        lexer = found->CastTo<Ast::Cpp::ClassLexer>();
        ASSERT_TRUE(lexer);
    }

    ASSERT_TRUE(lexer);
    EXPECT_EQ(lexer->GetName(), "Internal");
    ASSERT_TRUE(lexer->HasParent());
}

TEST(ASTTests, GetRootLexer)
{
    Ast::LogCollector logCollector;
    auto tree = GetASTFileTree(logCollector);

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "Internal";
    });

    ASSERT_TRUE(found);
    const auto root = found->GetRootLexer();
    ASSERT_TRUE(root);
    EXPECT_EQ("none", root->GetName());
}

TEST(ASTTests, LexerConstAndNonConstMiscTests)
{
    {
        Ast::LogCollector logCollector;
        auto tree = GetASTFileTree(logCollector);

        int lexersCount = 0;
        tree.ForEach([&lexersCount](Ast::BaseLexer* lexer, auto)
        {
            ++lexersCount;
            return true;
        });

        EXPECT_GT(lexersCount, 0);
    }

    {
        Ast::LogCollector logCollector;
        const auto tree = GetASTFileTree(logCollector);

        int lexersCount = 0;
        tree.ForEach([&lexersCount](const Ast::BaseLexer* lexer, auto)
        {
            ++lexersCount;
            return true;
        });

        EXPECT_GT(lexersCount, 0);
    }

    {
        Ast::LogCollector logCollector;
        auto tree = GetASTFileTree(logCollector);

        const auto found = tree.FindIf([](const Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "Internal";
        });

        ASSERT_TRUE(found);
    }

    {
        Ast::LogCollector logCollector;
        const auto tree = GetASTFileTree(logCollector);

        const auto found = tree.FindIf([](const Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "Internal";
        });

        ASSERT_TRUE(found);
    }

    {
        Ast::LogCollector logCollector;
        auto tree = GetASTFileTree(logCollector);

        const auto found = tree.FindIf([](const Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "GlobalClass";
        });

        ASSERT_TRUE(found);
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::EnumClassLexer>().size(), 0);
    }

    {
        Ast::LogCollector logCollector;
        const auto tree = GetASTFileTree(logCollector);

        const auto found = tree.FindIf([](const Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "GlobalClass";
        });

        ASSERT_TRUE(found);
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::EnumClassLexer>().size(), 0);
    }

    {
        Ast::LogCollector logCollector;
        const auto tree = GetASTFileTree(logCollector);

        const auto found = tree.FindIfAs<Ast::Cpp::ClassLexer>([](const Ast::BaseLexer* lexer)
        {
            return lexer->GetName() == "GlobalClass";
        });

        ASSERT_TRUE(found);
        EXPECT_EQ(Ast::Cpp::ClassLexer::typeName, found->GetLexerType());
        EXPECT_GT(found->GetChildLexers().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::NamespaceLexer>().size(), 0);
        EXPECT_EQ(found->GetChildLexers<Ast::Cpp::EnumClassLexer>().size(), 0);
    }
}