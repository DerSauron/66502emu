/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle accurate emulator gui.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BoardExecutor.h"
#include "MainWindow.h"
#include "board/Board.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(icons);

    QApplication::setApplicationName(QStringLiteral("6502emu"));
    QApplication::setOrganizationName(QStringLiteral("volkarts.com"));

    QApplication a(argc, argv);

    Board board{};

    BoardExecutor boardExecutor{&board};
    boardExecutor.start();

    MainWindow mainWindow{&board};
    mainWindow.show();

    int returnCode = QApplication::exec();

    boardExecutor.shutdown();

    return returnCode;
}
