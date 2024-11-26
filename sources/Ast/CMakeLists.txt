# ==================== AST ====================
file(
	GLOB_RECURSE  ASTSources
	"*.h"
	"*.cpp"
)

add_library(ASTCore STATIC ${ASTSources})
target_compile_features(ASTCore PUBLIC ${CXX_STANDARD})
set_target_properties(ASTCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCore PUBLIC ../)
target_link_libraries(ASTCore PUBLIC Utils boost_smart_ptr)