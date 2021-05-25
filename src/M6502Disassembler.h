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

#pragma once

#include <QList>

class Memory;

namespace M6502 {

struct Instruction
{
    uint16_t position;
    QString instruction;
    uint8_t cycles;
    uint8_t length{0};
    uint8_t bytes[3];
    QString comment;
};

QList<Instruction> disassemble(Memory* memory, uint16_t start = 0, uint16_t end = std::numeric_limits<uint16_t>::max());
QList<Instruction> disassembleCount(Memory* memory, uint16_t count, uint16_t start = 0);

} // namespace M6502
