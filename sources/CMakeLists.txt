file(
	GLOB Sources
	"AST/*.h"
	"AST/*.cpp"
	"AST/Lexers/*.h"
	"AST/Lexers/*.cpp"
)

add_library(AST STATIC ${Sources})

set_target_properties(AST PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(AST PUBLIC ./)
target_link_libraries(AST PUBLIC Utils)