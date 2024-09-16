# ==================== EXE ====================
file(
	GLOB_RECURSE ExeSources
	"*.h"
	"*.cpp"
)

add_executable(ASTCpp ${ExeSources})
set_target_properties(ASTCpp PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCpp PUBLIC ../)
target_link_libraries(ASTCpp PUBLIC ASTCppCore)
