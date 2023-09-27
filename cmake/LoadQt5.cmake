find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools Test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(qt5_config INTERFACE)
target_compile_definitions(qt5_config INTERFACE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)
