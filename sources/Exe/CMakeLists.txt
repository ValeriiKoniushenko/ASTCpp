# ==================== EXE ====================
file(
	GLOB_RECURSE ExeSources
	"*.h"
	"*.cpp"
)

add_executable(ASTCpp ${ExeSources})
target_compile_features(ASTCpp PUBLIC ${CXX_STANDARD})
set_target_properties(ASTCpp PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCpp PUBLIC ../)
target_link_libraries(ASTCpp PUBLIC ASTCppCore)
