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

#include "BaseLexer.h"

#include "../Readers/ContentStream.h"
#include "Ast/LogCollector.h"
#include "Ast/Rule.h"
#include "Ast/Utils/Scopes.h"
#include "Core/Assert.h"

namespace Ast
{

    bool BaseLexer::operator==(const BaseLexer& other) const
    {
        return other._reader == _reader && other._token.beginData == _token.beginData && other._token.endData == _token.endData &&
               other._lexerName == _lexerName && other._parentLexer == _parentLexer;
    }

    void BaseLexer::SetToken(const TokenReader& token)
    {
        _token = token;
    }

    bool BaseLexer::Validate(LogCollector& logCollector)
    {
        if (!DoValidate(logCollector))
        {
            return false;
        }
        if (!DoValidateScope(logCollector))
        {
            return false;
        }
        if (!DoMarkingValidate(logCollector))
        {
            return false;
        }
        if (!DoPostValidate(logCollector))
        {
            return false;
        }

        logCollector.AddLog(
            { String::Format("successfull parsing of the {}: '{}'", _lexerType.CStr(), _lexerName.CStr()), LogCollector::LogType::Success });

        return IsValid();
    }

    bool BaseLexer::IsValid() const
    {
        return !_lexerType.IsEmpty() && !_lexerName.IsEmpty() && _reader;
    }

    bool BaseLexer::IsCorrespondingToRule(const Rule& rule, LogCollector& logCollector, const char* additionalMessage /* = nullptr*/) const
    {
        return rule.IsCorrespondingTheRules(this, logCollector, additionalMessage);
    }

    bool BaseLexer::HasTheSameParentAs(BaseLexer::Ptr parent) const
    {
        if (Verify(!!parent))
        {
            if (_parentLexer)
            {
                return *parent == *_parentLexer;
            }
        }
        return false;
    }

    void BaseLexer::TryToSetParent(const Ptr& parent)
    {
        parent->TryToSetAsChild(this);
    }

    void BaseLexer::TryToSetAsChild(const Ptr& child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            if (_modifierParams.wasModified || IsContainLexer(child.get()))
            {
                auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
                                       [&child](const auto& lexer)
                                       {
                                           return child->GetLexerName() == lexer->GetLexerName();
                                       });

                if (Verify(it == _childLexers.cend(), "Such child already exists"))
                {
                    _childLexers.push_back(child);
                    child->_parentLexer = this;
                }
            }
        }
    }
    void BaseLexer::ForceSetAsChild(const Ptr& child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
                                   [&child](const auto& lexer)
                                   {
                                       return child->GetLexerName() == lexer->GetLexerName();
                                   });

            if (Verify(it == _childLexers.cend(), "Such child already exists"))
            {
                _childLexers.push_back(child);
                child->_parentLexer = this;
            }
        }
    }

    bool BaseLexer::IsContainLexer(const BaseLexer* other, bool isInItsScope /* = false*/) const
    {
        if (Verify(other) && Verify(_closeScope.has_value()) && Verify(_openScope.has_value()) && Verify(other->_closeScope.has_value()) &&
            Verify(other->_openScope.has_value()))
        {
            if (_openScope->string < other->_openScope->string && _closeScope->string > other->_closeScope->string)
            {
                if (isInItsScope)
                {
                    return !Utils::HasUnclosedBracket(_openScope->string, other->_openScope->string, '}', '{');
                }

                return true;
            }
        }
        return false;
    }

    void BaseLexer::Clear()
    {
        _token.Clear();
        _openScope.reset();
        _closeScope.reset();
        _lexerName.Clear();
        _parentLexer.reset();
        _childLexers.clear();
    }

    BaseLexer::BaseLexer(const ContentStream::Ptr& reader, const String& type)
        : _reader{ reader },
          _lexerType{ type }
    {
        Assert(!!_reader);
        Assert(!_lexerType.IsEmpty());
    }

} // namespace Ast