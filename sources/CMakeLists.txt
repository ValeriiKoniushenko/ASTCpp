# ==================== AST ====================
file(
	GLOB ASTSources
	"AST/*.h"
	"AST/*.cpp"
	"AST/Lexers/*.h"
	"AST/Lexers/*.cpp"
	"AST/Readers/*.h"
	"AST/Readers/*.cpp"
)

add_library(ASTCore STATIC ${ASTSources})
set_target_properties(ASTCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCore PUBLIC ./)
target_link_libraries(ASTCore PUBLIC Utils boost_smart_ptr)


# ==================== ASTCpp ====================
file(
	GLOB ASTCppSources
	"ASTCpp/*.h"
	"ASTCpp/*.cpp"
	"ASTCpp/Lexers/*.h"
	"ASTCpp/Lexers/*.cpp"
	"ASTCpp/Readers/*.h"
	"ASTCpp/Readers/*.cpp"
)

add_library(ASTCppCore STATIC ${ASTCppSources})
set_target_properties(ASTCppCore PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCppCore PUBLIC ./)
target_link_libraries(ASTCppCore PUBLIC ASTCore)


# ==================== EXE ====================
file(
	GLOB ExeSources
	"Exe/*.h"
	"Exe/*.cpp"
)

add_executable(ASTCpp ${ExeSources})
set_target_properties(ASTCpp PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ASTCpp PUBLIC ./)
target_link_libraries(ASTCpp PUBLIC ASTCppCore)
