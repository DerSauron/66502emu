add_library(6502emu STATIC
    board/ACIA.cpp
    board/ACIA.h
    board/Board.cpp
    board/Board.h
    board/Bus.cpp
    board/Bus.h
    board/BusConnection.cpp
    board/BusConnection.h
    board/Clock.cpp
    board/Clock.h
    board/CPU.cpp
    board/CPU.h
    board/Debugger.cpp
    board/Debugger.h
    board/Device.cpp
    board/Device.h
    board/LCD.cpp
    board/LCD.h
    board/Memory.cpp
    board/Memory.h
    board/VIA.cpp
    board/VIA.h
    board/WireState.h
    codeeditor/CodeEditor.cpp
    codeeditor/CodeEditor.h
    codeeditor/Highlighter.cpp
    codeeditor/Highlighter.h
    codeeditor/LineNumberArea.cpp
    codeeditor/LineNumberArea.h
    impl/hd44780u.cpp
    impl/hd44780u.h
    impl/hd44780u_data.cpp
    impl/hd44780u_data.h
    impl/m6502.cpp
    impl/m6502.h
    impl/m6522.cpp
    impl/m6522.h
    utils/ArrayView.cpp
    utils/ArrayView.h
    utils/BitManipulations.cpp
    utils/BitManipulations.h
    utils/Strings.cpp
    utils/Strings.h
    views/ACIAView.cpp
    views/ACIAView.h
    views/ACIAView.ui
    views/BitsView.cpp
    views/BitsView.h
    views/BusView.cpp
    views/BusView.h
    views/BusView.ui
    views/ClockView.cpp
    views/ClockView.h
    views/ClockView.ui
    views/Console.cpp
    views/Console.h
    views/CPUView.cpp
    views/CPUView.h
    views/CPUView.ui
    views/DeviceView.cpp
    views/DeviceView.h
    views/DisassemblerView.cpp
    views/DisassemblerView.h
    views/DisassemblerView.ui
    views/LCDCharPanel.cpp
    views/LCDCharPanel.h
    views/LCDView.cpp
    views/LCDView.h
    views/LCDView.ui
    views/MemoryPageView.cpp
    views/MemoryPageView.h
    views/MemoryView.cpp
    views/MemoryView.h
    views/MemoryView.ui
    views/RegisterView.cpp
    views/RegisterView.h
    views/RegisterView.ui
    views/SignalsView.cpp
    views/SignalsView.h
    views/SignalsView.ui
    views/SourcesView.cpp
    views/SourcesView.h
    views/SourcesView.ui
    views/StartStopButton.cpp
    views/StartStopButton.h
    views/VIAView.cpp
    views/VIAView.h
    views/VIAView.ui
    views/View.cpp
    views/View.h
    views/ViewFactory.cpp
    views/ViewFactory.h
    AboutDialog.cpp
    AboutDialog.h
    AboutDialog.ui
    BoardLoader.cpp
    BoardLoader.h
    DeviceConfigModel.cpp
    DeviceConfigModel.h
    DeviceViewCreator.cpp
    DeviceViewCreator.h
    M6502Disassembler.cpp
    M6502Disassembler.h
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    Program.cpp
    Program.h
    ProgramLoader.cpp
    ProgramLoader.h
    UserState.cpp
    UserState.h
    icons.qrc
)

target_compile_definitions(6502emu PRIVATE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

target_link_libraries(6502emu PUBLIC
    Qt5::Widgets
)

set(TS_FILES
    6502emu_en.ts
    6502emu_de.ts
)
target_sources(6502emu PRIVATE ${TS_FILES})
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(6502emu_app "")

target_sources(6502emu_app PRIVATE
    main.cpp
)

target_link_libraries(6502emu_app PRIVATE
    6502emu
)

set_target_properties(6502emu_app PROPERTIES
    OUTPUT_NAME "6502emu"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist"
)

set_target_properties(6502emu_app PROPERTIES
    WIN32_EXECUTABLE true
)

target_sources(6502emu_app PRIVATE
    6502emu.rc
)
