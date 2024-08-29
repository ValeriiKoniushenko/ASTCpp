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
    public SomeInterface<char>,
    public ElseOne, public SomeInterface222<char>
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
} // namespace

TEST(CoreTests, SimpleParse)
{
    Ast::LogCollector logCollector;

    Ast::Reader::Ptr reader = new Ast::Reader;
    reader->Read(content);
    reader->ApplyFilters<Ast::Cpp::CommentFilter>();

    Ast::ASTFileTree tree(reader);
    tree.ParseUsing<Ast::Cpp::FileParser>(logCollector);

    EXPECT_FALSE(logCollector.HasAny<Ast::LogCollector::LogType::Error>());
}

TEST(CoreTests, SimpleGettingLexer)
{
    Ast::LogCollector logCollector;

    Ast::Reader::Ptr reader = new Ast::Reader;
    reader->Read(content);
    reader->ApplyFilters<Ast::Cpp::CommentFilter>();

    Ast::ASTFileTree tree(reader);
    tree.ParseUsing<Ast::Cpp::FileParser>(logCollector);

    ASSERT_FALSE(logCollector.HasAny<Ast::LogCollector::LogType::Error>());

    auto found = tree.FindIf([](Ast::BaseLexer* lexer)
    {
        return lexer->GetName() == "Internal";
    });

    ASSERT_TRUE(found);
    EXPECT_EQ(found->GetName(), "Internal");
    ASSERT_TRUE(found->HasParent());
}
