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

#include <gtest/gtest.h>

// ============== EXAMPLE OF GENERATED DECL CODE ============
// ======= !!!WARNING!!! ========
// This file was generated automatically, don't change it,
// because it will be replaced with the next generation
#pragma once
#include "Ast/CommonTypes.h"

#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <vector>

enum class ExampleEnum : int;

namespace Reflect::Enum
{
    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, const Ast::String&> Name();

    [[nodiscard]] inline const Ast::String& ToString(const ExampleEnum value);

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::optional<ExampleEnum>> FromString(const Ast::String& value);

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, uint32_t> Size() noexcept;

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::vector<ExampleEnum>> ToVector();

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_set<ExampleEnum>> ToSet();

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_map<ExampleEnum, Ast::String>> ToMap();

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, Ast::String> ToJsonString();

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, Ast::String> ToXmlString();
} // namespace Reflect::Enum
// =================================================


// ============== THE PART OF THE INPUT FILE ============
enum class ExampleEnum : int
{
    Hello,
    World
};
// =================================================


// ============== EXAMPLE OF GENERATED IMPL CODE ============
// ======= !!!WARNING!!! ========
// This file was generated automatically, don't change it,
// because it will be replaced with the next generation

namespace Reflect::Enum
{
    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, const Ast::String&> Name()
    {
        static const auto returnValue = "ExampleEnum"_atom;
        return returnValue;
    }

    [[nodiscard]] inline const Ast::String& ToString(const ExampleEnum value)
    {
        if (value == ExampleEnum::Hello)
        {
            static const auto returnValue = "Hello"_atom;
            return returnValue;
        }
        if (value == ExampleEnum::World)
        {
            static const auto returnValue = "World"_atom;
            return returnValue;
        }

        static const auto returnValue = ""_atom;
        return returnValue;
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::optional<ExampleEnum>> FromString(const Ast::String& value)
    {
        if (value == "Hello"_atom)
        {
            return ExampleEnum::Hello;
        }
        if (value == "World"_atom)
        {
            return ExampleEnum::World;
        }

        return std::nullopt;
    }

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, uint32_t> Size() noexcept
    {
        return 2;
    }

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::vector<ExampleEnum>> ToVector()
    {
        return { ExampleEnum::Hello, ExampleEnum::World };
    }

    template<class T>
    [[nodiscard]] constexpr std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_set<ExampleEnum>> ToSet()
    {
        return { ExampleEnum::Hello, ExampleEnum::World };
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, std::unordered_map<ExampleEnum, Ast::String>> ToMap()
    {
        return { { ExampleEnum::Hello, "Hello"_atom }, { ExampleEnum::World, "World"_atom } };
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, Ast::String> ToJsonString()
    {
        return R"({
    "Name": "ExampleEnum",
    "Type": "int",
    "Constants": {
        "Hello": {},
        "World": {}
    }
})"_f << static_cast<int>(ExampleEnum::Hello)
      << static_cast<int>(ExampleEnum::World);
    }

    template<class T>
    [[nodiscard]] std::enable_if_t<std::is_same_v<T, ExampleEnum>, Ast::String> ToXmlString()
    {
        return R"(<Enum name="ExampleEnum" underlyingType="int">
    <Enumerator name="Hello" value="{}" />
    <Enumerator name="World" value="{}" />
</Enum>
)"_f << static_cast<int>(ExampleEnum::Hello)
      << static_cast<int>(ExampleEnum::World);
    }
} // namespace Reflect::Enum

// =================================================

TEST(GeneratedEnum, FromString)
{
    EXPECT_EQ(ExampleEnum::Hello, Reflect::Enum::FromString<ExampleEnum>("Hello"_atom));
    EXPECT_EQ(ExampleEnum::World, Reflect::Enum::FromString<ExampleEnum>("World"_atom));
    EXPECT_EQ(std::nullopt, Reflect::Enum::FromString<ExampleEnum>("123123"_atom));
}

TEST(GeneratedEnum, GetSize)
{
    EXPECT_EQ(2, Reflect::Enum::Size<ExampleEnum>());
}

TEST(GeneratedEnum, ToString)
{
    EXPECT_EQ("Hello"_atom, Reflect::Enum::ToString(ExampleEnum::Hello));
    EXPECT_EQ("World"_atom, Reflect::Enum::ToString(ExampleEnum::World));
    EXPECT_EQ(""_atom, Reflect::Enum::ToString(static_cast<ExampleEnum>(999)));
}

TEST(GeneratedEnum, ToVector)
{
    const auto vec = Reflect::Enum::ToVector<ExampleEnum>();
    EXPECT_EQ(2, vec.size());
    EXPECT_EQ("Hello"_atom, Reflect::Enum::ToString(vec[0]));
    EXPECT_EQ("World"_atom, Reflect::Enum::ToString(vec[1]));
}

TEST(GeneratedEnum, ToSet)
{
    const auto set = Reflect::Enum::ToSet<ExampleEnum>();
    EXPECT_EQ(2, set.size());
    EXPECT_NE(set.end(), set.find(ExampleEnum::Hello));
    EXPECT_NE(set.end(), set.find(ExampleEnum::World));
}

TEST(GeneratedEnum, ToMap)
{
    auto map = Reflect::Enum::ToMap<ExampleEnum>();
    EXPECT_EQ(2, map.size());
    EXPECT_EQ("Hello"_atom, map[ExampleEnum::Hello]);
    EXPECT_EQ("World"_atom, map[ExampleEnum::World]);
}

TEST(GeneratedEnum, GetName)
{
    EXPECT_EQ("ExampleEnum"_atom, Reflect::Enum::Name<ExampleEnum>());
}

TEST(GeneratedEnum, ToJsonString)
{
    std::cout << Reflect::Enum::ToJsonString<ExampleEnum>() << std::endl;
}

TEST(GeneratedEnum, ToXmlString)
{
    std::cout << Reflect::Enum::ToXmlString<ExampleEnum>() << std::endl;
}