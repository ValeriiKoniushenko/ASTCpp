cmake_minimum_required(VERSION 3.30.0..)

project(AST-Cpp VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(ExternalDependencies)
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


# ========= Fetch & apply dependencies =========
set(BOOST_DIR "boost-1.86.0")
set(BOOST_ZIP_FILE ${BOOST_DIR}.7z)
set(BOOST_ZIP_FILE_PATH ${CMAKE_BINARY_DIR}/${BOOST_ZIP_FILE})
set(BOOST_DIR_EXTRACT ${CMAKE_SOURCE_DIR}/${DEPENDENCIES_DIR})
set(BOOST_PATH ${CMAKE_SOURCE_DIR}/${DEPENDENCIES_DIR}/${BOOST_DIR})
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${DEPENDENCIES_DIR}/${BOOST_DIR}/boost)

	if (NOT EXISTS ${CMAKE_BINARY_DIR}/${BOOST_ZIP_FILE})
		message(STATUS "Boost: downloading")
		file(DOWNLOAD https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.7z ${BOOST_ZIP_FILE_PATH})
	endif ()

	if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${DEPENDENCIES_DIR}/${BOOST_DIR})
		message(STATUS "Boost: extracting")
		file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/${BOOST_ZIP_FILE} DESTINATION ${BOOST_DIR_EXTRACT})
	endif ()

	execute_process(COMMAND cmd /C "${CMAKE_SOURCE_DIR}/scripts/install_boost.bat")
	execute_process(COMMAND cmd /C "${CMAKE_SOURCE_DIR}/scripts/run_b2_boost.bat")
endif ()

add_subdirectory(${DEPENDENCIES_DIR}/Utils)
add_subdirectory(${BOOST_PATH})

add_subdirectory(sources)
add_subdirectory(tests)
