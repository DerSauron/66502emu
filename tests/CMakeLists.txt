include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
)

macro(simple_test name)
    add_executable(test_${name} test_${name}.cpp)
    target_link_libraries(test_${name} PRIVATE
        project_config
        qt5_config
        app
        Qt5::Test
    )
    source_group("" FILES test_${name}.cpp)
    add_test(test_${name} test_${name})
endmacro()

simple_test(BitManipulations)
simple_test(Bus)
simple_test(LCDCharPanel)
simple_test(TransposeData)
