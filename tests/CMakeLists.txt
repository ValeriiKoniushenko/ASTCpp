cmake_minimum_required(VERSION 3.30.0..)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
    GLOB Sources
    "*.cpp"
)

add_executable(ASTTests ${Sources})
target_compile_features(ASTTests PUBLIC ${CXX_STANDARD})
set_target_properties(ASTTests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ASTTests PUBLIC gtest ASTCppCore)