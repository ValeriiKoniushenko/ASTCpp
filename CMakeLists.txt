cmake_minimum_required(VERSION 3.27.0)

project(Utils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DEPENDENCIES_DIR dependencies)

add_subdirectory(${DEPENDENCIES_DIR}/glm)
add_subdirectory(${DEPENDENCIES_DIR}/googletest)
add_subdirectory(${DEPENDENCIES_DIR}/benchmark)
add_subdirectory(benchmark)
add_subdirectory(tests)
add_subdirectory(sources)