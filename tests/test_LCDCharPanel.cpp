/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle correct emulator.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#include "views/LCDCharPanel.h"
#include "utils/ArrayView.h"
#include <QApplication>

namespace {

constexpr uint8_t LetterA[] = {
    0b00100,
    0b01010,
    0b10001,
    0b10001,
    0b11111,
    0b10001,
    0b10001,
    0b00000,
};

}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    LCDCharPanel panel;
    panel.setCharacterData(QPoint(), ArrayView(LetterA, 8));
    panel.show();
    return QApplication::exec();
}
