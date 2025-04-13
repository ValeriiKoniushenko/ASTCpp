cmake_minimum_required(VERSION 3.30.0..)

project(AST-Cpp VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(Flags)
include(Features)
include(Predefinitions)

# ========= Base setup & Predefinitions =========
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(REQUIRED_CXX_STANDARD cxx_std_23) # read-only
set(CXX_STANDARD)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(DEPENDENCIES_DIR dependencies)
set(DEPENDENCIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${DEPENDENCIES_DIR})
set(BENCHMARK_ENABLE_TESTING)

set(CMAKE_MSVC_RUNTIME_LIBRARY)
set(CMAKE_CXX_FLAGS_RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

InstallWin32MSVCFlags()
InstallCXXStandardVersion()
InstallDebugLevel()
InstallBenchmarkDependenciesTesting()

add_subdirectory(${DEPENDENCIES_DIR}/AST)
add_subdirectory(sources)
add_subdirectory(tests)
add_subdirectory(benchmarks)

