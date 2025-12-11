cmake_minimum_required(VERSION 3.16)
project(
  CPLib
  VERSION 0.0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# As a header-only library, we define it as an INTERFACE library. This makes it
# easy for other targets (like tests) to consume it.
add_library(cplib INTERFACE)
target_include_directories(cplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(CPLIB_USE_FMT_LIB "Use external fmt library" OFF)
if(CPLIB_USE_FMT_LIB)
  message(STATUS "Using external fmt library")
  include(FetchContent)
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(fmt)
  target_link_libraries(cplib INTERFACE fmt::fmt)
  target_compile_definitions(cplib INTERFACE CPLIB_USE_FMT_LIB)
endif()

enable_testing()
add_subdirectory(tests)

# This part is for the 'single_header' build target.
find_package(
  Python3
  COMPONENTS Interpreter
  REQUIRED)

# Find all source header files to set them as dependencies.
file(GLOB_RECURSE CPLIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.i.hpp")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cplib.hpp
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed.py
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cplib.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include >${CMAKE_CURRENT_BINARY_DIR}/cplib.hpp
  DEPENDS ${CPLIB_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed.py
  COMMENT "Generating single-header file cplib.hpp")

# cmake --build build --target single_header
add_custom_target(single_header DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cplib.hpp)
