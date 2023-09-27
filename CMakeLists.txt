cmake_minimum_required(VERSION 3.20)

project(6502emu LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

include(PreventInSourceBuilds)
include(CompilerWarnings)

add_library(project_config INTERFACE)
set_compiler_warnings(project_config INTERFACE)
target_compile_features(project_config INTERFACE
    cxx_std_20
)

include(LoadQt5)

add_subdirectory(src)
add_subdirectory(tests)
