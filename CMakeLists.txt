cmake_minimum_required(VERSION 3.3.0...3.5)

project(AST-Cpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(DEPENDENCIES_DIR dependencies)

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
