file(
	GLOB_RECURSE ASTCppSources
	"*.h"
	"*.cpp"
)

add_library(ASTCppCore STATIC ${ASTCppSources})
add_library(AST::Cpp::Core ALIAS ASTCppCore)

target_compile_features(ASTCppCore PUBLIC ${CXX_STANDARD})
set_target_properties(ASTCppCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCppCore PUBLIC ../)
target_link_libraries(ASTCppCore PUBLIC AST::Core)