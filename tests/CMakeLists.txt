cmake_minimum_required(VERSION 3.27.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

file(
    GLOB Sources
    "*.cpp"
)

add_executable(ASTTests ${Sources})
target_link_libraries(ASTTests PUBLIC gtest Utils ASTCppCore)