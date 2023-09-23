cmake_minimum_required(VERSION 3.13)

project(6502emu LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(project_config INTERFACE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
target_compile_definitions(project_config INTERFACE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

find_package(Qt5 COMPONENTS Widgets LinguistTools Test REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
