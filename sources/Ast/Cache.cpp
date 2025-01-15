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

#include "Cache.h"

namespace Ast::experimental
{

    bool Cache::IsExist() const
    {
        using std::filesystem::directory_iterator;
        std::size_t count = 0;
        return std::count_if(directory_iterator(_cachePath), directory_iterator{},
                             [&count](const std::filesystem::path& p)
                             {
                                 ++count;
                                 return true;
                             });

        return count > 0 && std::filesystem::exists(_cachePath);
    }

    bool Cache::Read()
    {
        if (!IsExist())
        {
            return false;
        }

        return true;
    }

    bool Cache::Write(WriteAction action)
    {
        if (!Verify(!_cachePath.empty(), "Cache path is invalid, impossible to write"))
        {
            return false;
        }

        if (!Verify(!!_projectTree, "Project tree is invalid, impossible to generate a cache"))
        {
            return false;
        }

        // creating of the directory
        if (!std::filesystem::exists(_cachePath))
        {
            try
            {
                std::filesystem::create_directory(_cachePath);
            }
            catch (std::filesystem::filesystem_error& e)
            {
                Assert(false);
                _projectTree->GetLogCollector()->AddLog(
                    { "Impossible to create a cache directory by the next reason: {}"_f << e.what(), LogCollector::LogType::Error });
                return false;
            }

            if (!std::filesystem::exists(_cachePath))
            {
                _projectTree->GetLogCollector()->AddLog({ "By some reasons a cache directory wasn't created", LogCollector::LogType::Error });
                return false;
            }
        }



        return true;
    }

} // namespace Ast::experimental