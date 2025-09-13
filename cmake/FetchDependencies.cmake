include(FetchContent)

FetchContent_Declare(AST
    GIT_REPOSITORY "https://github.com/ValeriiKoniushenko/AST.git"
    GIT_TAG origin/develop
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(AST)
