file(
	GLOB Sources
	"AST/*.h"
	"AST/*.cpp"
	"AST/Lexers/*.h"
	"AST/Lexers/*.cpp"
	"AST/Reader/*.h"
	"AST/Reader/*.cpp"
)

add_library(AST STATIC ${Sources})

set_target_properties(AST PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(AST PUBLIC ./)
target_link_libraries(AST PUBLIC Utils)

add_executable(ASTCpp Exe/main.cpp)
set_target_properties(ASTCpp PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCpp PUBLIC ./)
target_link_libraries(ASTCpp PUBLIC Utils AST)
