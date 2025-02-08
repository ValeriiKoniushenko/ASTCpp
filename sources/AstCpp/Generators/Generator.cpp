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

#include "Generator.h"

#include "AstCpp/Utils/UnitBridge.h"
#include "GeneratorUnit.h"

#include <fstream>

namespace Ast::Cpp
{

    void Generator::Generate()
    {
        // Ast::Generator::Generate(); - don't call it
        ForEachOverRegenerateableUnits(
            [this](const ProjectTree::Unit* unit)
            {
                auto declSource = unit->GetGeneratedDummyHeader();
                decltype(declSource) implSource;

                const auto tree = unit->GetTree();
                tree->ForEachOverMarked(
                    [&](const BaseLexer* lexer)
                    {
                        auto declGenerator = GetGeneratorUnitFor(*lexer, GeneratorUnitDecl::IsSelf);
                        if (!Verify(!!declGenerator))
                        {
                            _projectTree->GetLogCollector()->AddLog({ "Generator(declaration) wasn't found for lexer: '{}' by the next path: {}"_f
                                                                          << lexer->GetLexerType() << lexer->GetFullPath().first,
                                                                      LogCollector::LogType::Error });
                        }
                        else
                        {
                            declSource += declGenerator->Generate(lexer) + Code::Endl();
                        }

                        auto implGenerator = GetGeneratorUnitFor(*lexer, GeneratorUnitImpl::IsSelf);
                        if (!Verify(!!declGenerator))
                        {
                            _projectTree->GetLogCollector()->AddLog({ "Generator(implementation) wasn't found for lexer: '{}' by the next path: {}"_f
                                                                          << lexer->GetLexerType() << lexer->GetFullPath().first,
                                                                      LogCollector::LogType::Error });
                        }
                        else
                        {
                            implSource += implGenerator->Generate(lexer) + Code::Endl();
                        }
                    });

                ConstUnitBridge bridge(unit);

                if (!declSource.IsEmpty())
                {
                    std::ofstream outDecl(bridge.GetGeneratedDeclFilePath());
                    if (outDecl.is_open())
                    {
                        outDecl << declSource.c_str();
                    }
                }

                if (!implSource.IsEmpty())
                {
                    std::ofstream outImpl(bridge.GetGeneratedImplFilePath());
                    if (outImpl.is_open())
                    {
                        outImpl << declSource.c_str();
                    }
                }
            });
    }
} // namespace Ast::Cpp
