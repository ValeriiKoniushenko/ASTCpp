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

#include "../CommonTypes.h"
#include "../Readers/Reader.h"
#include "../Readers/Token.h"
#include "Utils/CopyableAndMoveableBehaviour.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace Ast
{
    class Rule;
    class LogCollector;
    class BaseLexer;

    template<class T>
    concept IsLexer = (std::derived_from<T, BaseLexer> && requires(T) {
                          { T::typeName };
                      } && std::is_class_v<typename T::Ptr>) || std::is_void_v<T>;

    class BaseLexer : public Utils::CopyableAndMoveable, public boost::intrusive_ref_counter<BaseLexer>
    {
    public:
        AST_CLASS(BaseLexer)

        struct LineToken final
        {
            const String::CharT* string = nullptr;
            std::size_t line = 0;

            [[nodiscard]] bool IsValid() const noexcept { return string && line != 0; }

            [[nodiscard]] bool operator<(const LineToken& other) const noexcept
            {
                return Verify(other.IsValid() && IsValid()) ? string < other.string : false;
            }
        };

    public:
        ~BaseLexer() override = default;

        [[nodiscard]] bool operator==(const BaseLexer&) const;

        void SetToken(const TokenReader& token);
        bool Validate(LogCollector& logCollector);
        [[nodiscard]] bool IsValid() const;

        bool IsCorrespondingToRule(const Rule& rule, LogCollector& logCollector, const char* additionalMessage = nullptr) const;

        template<IsLexer Lexer>
        [[nodiscard]] bool IsTypeOf() const noexcept
        {
            return _lexerType == Lexer::typeName && dynamic_cast<const Lexer*>(this) != nullptr;
        }

        template<IsLexer Lexer>
        [[nodiscard]] typename Lexer::Ptr CastTo() noexcept
        {
            if (auto* newType = dynamic_cast<Lexer*>(this))
            {
                return boost::intrusive_ptr<Lexer>(newType);
            }
            return {};
        }

        template<IsLexer Lexer>
        [[nodiscard]] typename Lexer::CPtr CastTo() const noexcept
        {
            if (auto* newType = dynamic_cast<const Lexer*>(this))
            {
                return boost::intrusive_ptr<const Lexer>(newType);
            }
            return {};
        }

        [[nodiscard]] String GetLexerName() const noexcept { return _lexerName; }
        [[nodiscard]] String GetLexerType() const noexcept { return _lexerType; }

        // ================================================================
        // ================== WORKING WITH LEXERS TREE ====================
        // ================================================================

        [[nodiscard]] Ptr GetRootLexer() { return GetRootLexerImpl(this); }
        [[nodiscard]] CPtr GetRootLexer() const { return GetRootLexerImpl<true>(this); }

        [[nodiscard]] bool HasTheSameParentAs(Ptr parent) const;
        [[nodiscard]] bool HasParent() const noexcept { return !!_parentLexer; }
        [[nodiscard]] CPtr GetParentLexer() const { return _parentLexer; }
        [[nodiscard]] Ptr GetParentLexer() { return _parentLexer; }

        template<IsLexer Lexer>
        [[nodiscard]] bool HasTheSameParentAs() const
        {
            if (HasParent())
            {
                return _parentLexer->IsTypeOf<Lexer>();
            }
            return false;
        }

        [[nodiscard]] std::vector<Ptr>& GetChildLexers() { return _childLexers; }
        [[nodiscard]] const std::vector<Ptr>& GetChildLexers() const { return _childLexers; }
        [[nodiscard]] bool HasChildLexers() const noexcept { return !_childLexers.empty(); }

        template<IsLexer Lexer>
        [[nodiscard]] std::vector<Ptr> GetChildLexers()
        {
            return GetChildLexersImpl<Lexer>(this);
        }

        template<IsLexer Lexer>
        [[nodiscard]] std::vector<CPtr> GetChildLexers() const
        {
            return GetChildLexersImpl<Lexer, true>(this);
        }

        [[nodiscard]] std::pair<String, std::vector<CPtr>> GetFullPath() const { return GetFullPathImpl<true>(this); }

        [[nodiscard]] std::pair<String, std::vector<Ptr>> GetFullPath() { return GetFullPathImpl(this); }

        void TryToSetAsChild(const Ptr& child);
        void ForceSetAsChild(const Ptr& child);
        [[nodiscard]] bool IsContainLexer(const BaseLexer* other, bool isInItsScope = false) const;
        [[nodiscard]] bool IsContainLexer(const Ptr& other, bool isInItsScope = false) const { return IsContainLexer(other.get(), isInItsScope); }
        [[nodiscard]] std::optional<LineToken> GetOpenScope() const noexcept { return _openScope; }
        [[nodiscard]] std::optional<LineToken> GetCloseScope() const noexcept { return _closeScope; }

        [[nodiscard]] long long GetDistanceToLexer(const BaseLexer* lexer) const noexcept
        {
            return Verify(lexer && lexer->GetOpenScope() && _closeScope) ? lexer->_openScope->string - _closeScope->string : 0;
        }

        [[nodiscard]] long long GetDistanceToLexer(const Ptr& lexer) const noexcept { return GetDistanceToLexer(lexer.get()); }

        void Clear();

        [[nodiscard]] Reader::Ptr GetReader() { return _reader; }
        [[nodiscard]] Reader::CPtr GetReader() const { return _reader; }
        [[nodiscard]] TokenReader GetTokenReader() const noexcept { return _token; }

    protected:
        virtual bool DoValidate(LogCollector& logCollector) = 0;
        virtual bool DoValidateScope(LogCollector& logCollector) { return true; }
        virtual bool DoPostValidate(LogCollector& logCollector) { return true; }

        BaseLexer(const Reader::Ptr& reader, const String& type);

    protected:
        TokenReader _token;
        Reader::Ptr _reader;

        std::optional<LineToken> _openScope;
        std::optional<LineToken> _closeScope;

        const String _lexerType;
        String _lexerName = "none"_atom;
        Ptr _parentLexer;
        std::vector<Ptr> _childLexers;

    private:
        template<IsLexer Lexer, bool IsConst = false>
        [[nodiscard]] static std::vector<AdaptivePtr<IsConst>> GetChildLexersImpl(AdaptiveRawPtr<IsConst> lexer)
        {
            std::vector<AdaptivePtr<IsConst>> childs;

            for (auto&& child : lexer->_childLexers)
            {
                if (child->template IsTypeOf<Lexer>())
                {
                    childs.push_back(child);
                }
            }

            return childs;
        }

        template<bool IsConst = false>
        [[nodiscard]] static AdaptivePtr<IsConst> GetRootLexerImpl(AdaptiveRawPtr<IsConst> lexer)
        {
            auto* i = const_cast<BaseLexer*>(lexer);
            if (Verify(i))
            {
                while (i->HasParent())
                {
                    i = i->GetParentLexer().get();
                }
            }
            return { i };
        }

        template<bool IsConst = false>
        [[nodiscard]] static std::pair<String, std::vector<AdaptivePtr<IsConst>>> GetFullPathImpl(AdaptiveRawPtr<IsConst> lexer)
        {
            String path;
            std::vector<AdaptivePtr<IsConst>> pathLexers;

            auto* i = const_cast<BaseLexer*>(lexer);
            if (Verify(i))
            {
                do
                {
                    path.PushFront(i->GetLexerName());
                    pathLexers.push_back(i);
                    i = i->GetParentLexer().get();
                    if (i->HasParent())
                    {
                        path.PushFront("::"_atom);
                    }
                } while (i->HasParent());
            }
            std::reverse(pathLexers.begin(), pathLexers.end());
            return { std::move(path), std::move(pathLexers) };
        }
    };
} // namespace Ast