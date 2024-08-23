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

#include "../Readers/FileReader.h"
#include "AST/LogCollector.h"
#include "Core/Assert.h"

namespace Ast
{

    bool BaseLexer::operator==(const BaseLexer& other) const
    {
        return other._reader == _reader
            && other._token.beginData == _token.beginData
            && other._token.endData == _token.endData
            && other._name == _name
            && other._parentLexer == _parentLexer;
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
        if (!DoPostValidate(logCollector))
        {
            return false;
        }

        logCollector.AddLog(
            { String::Format("successfull parsing of the {}: '{}'", _lexerType.CStr(), _name.CStr()), LogCollector::LogType::Success });
        return true;
    }

    bool BaseLexer::HasTheSameParent(boost::intrusive_ptr<BaseLexer> parent) const
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

    void BaseLexer::TryToSetAsChild(boost::intrusive_ptr<BaseLexer> child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            if (IsContainLexer(child.get()))
            {
                auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
                   [&child](const auto& lexer)
                   {
                       return child->GetName() == lexer->GetName();
                   });

                if (Verify(it == _childLexers.cend(), "Such child already exists"))
                {
                    _childLexers.push_back(child);
                    child->_parentLexer = this;
                }
            }
        }
    }
    void BaseLexer::ForceSetAsChild(boost::intrusive_ptr<BaseLexer> child)
    {
        if (Verify(!!child) && !child->HasParent())
        {
            auto it = std::find_if(_childLexers.cbegin(), _childLexers.cend(),
               [&child](const auto& lexer)
               {
                   return child->GetName() == lexer->GetName();
               });

            if (Verify(it == _childLexers.cend(), "Such child already exists"))
            {
                _childLexers.push_back(child);
                child->_parentLexer = this;
            }
        }
    }

    bool BaseLexer::IsContainLexer(const BaseLexer* other) const
    {
        if (Verify(other) && Verify(_closeScope.has_value()) && Verify(_openScope.has_value()) && Verify(other->_closeScope.has_value()) &&
            Verify(other->_openScope.has_value()))
        {
            if (_openScope->string < other->_openScope->string && _closeScope->string > other->_closeScope->string)
            {
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
        _name.Clear();
        _parentLexer.reset();
        _childLexers.clear();
    }

    BaseLexer::BaseLexer(const FileReader& reader, const String& type)
        : _reader{ &reader },
          _lexerType{ type }
    {
        Assert(_reader);
        Assert(!_lexerType.IsEmpty());
    }

} // namespace Ast