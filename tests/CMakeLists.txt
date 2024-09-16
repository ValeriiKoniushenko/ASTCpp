cmake_minimum_required(VERSION 3.30.0..)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
    GLOB Sources
    "*.cpp"
)

add_executable(ASTTests ${Sources})
target_link_libraries(ASTTests PUBLIC gtest ASTCppCore)