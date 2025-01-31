file(
	GLOB_RECURSE ASTCppSources
	"*.h"
	"*.cpp"
)

add_library(__ASTCppCore STATIC ${ASTCppSources})
add_library(AST::Cpp::Core ALIAS __ASTCppCore)

target_compile_features(__ASTCppCore PUBLIC ${CXX_STANDARD})
set_target_properties(__ASTCppCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(__ASTCppCore PUBLIC ../)
target_link_libraries(__ASTCppCore PUBLIC AST::Core)