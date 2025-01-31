cmake_minimum_required(VERSION 3.30.0..)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
    GLOB Sources
    "*.cpp"
)

add_executable(ASTCppTests ${Sources})
target_compile_features(ASTCppTests PUBLIC ${CXX_STANDARD})
set_target_properties(ASTCppTests PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ASTCppTests PUBLIC gtest AST::Cpp::Core)
target_compile_definitions(ASTCppTests PRIVATE PATH_TO_TEST_PROJECT="${CMAKE_CURRENT_SOURCE_DIR}/test_projects/")