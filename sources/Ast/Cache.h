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

#include "CommonTypes.h"
#include "ProjectTree.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

namespace Ast
{

    class Cache : public boost::intrusive_ref_counter<Cache>, public Utils::NotCopyableButMoveable
    {
    public:
        AST_CLASS(Cache)
        using PTree = boost::property_tree::ptree;

        inline static const char* defaultPath = ".generator-cache";

        CreateEnum(WriteAction, int,
            Overwrite, // overwrite absolutely all
            Update // will write if not exists, and will update if exists
        );

    public:
        explicit Cache(const ProjectTree::Ptr& projectTree) : _projectTree{projectTree} {}

        [[nodiscard]] bool IsExist() const;
        bool ReadFromCache();
        bool WriteToCache(WriteAction action = WriteAction::Update) const;

    protected:
        ProjectTree::Ptr _projectTree;
        std::filesystem::path _cachePath = defaultPath;
    };

} // namespace Ast