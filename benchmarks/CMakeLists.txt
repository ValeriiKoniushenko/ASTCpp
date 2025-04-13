cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
        GLOB Sources
        "*.cpp"
)

add_executable(ASTCppBenchmark ${Sources})
target_link_libraries(ASTCppBenchmark PUBLIC benchmark AST::Cpp::Core)