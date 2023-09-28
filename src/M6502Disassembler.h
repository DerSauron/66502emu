/*
 * Original code:
 * Copyright (c) 1998-2014 Tennessee Carmel-Veilleux <veilleux@tentech.ca>
 * https://github.com/tcarmelveilleux/dcc6502
 *
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

#pragma once

#include <QList>

class Memory;

namespace M6502 {

enum class AddressingMode
{
    IMMED, // Immediate
    ABSOL, // Absolute
    ZEROP, // Zero Page
    IMPLI, // Implied
    INDIA, // Indirect Absolute
    ABSIX, // Absolute indexed with X
    ABSIY, // Absolute indexed with Y
    ZEPIX, // Zero page indexed with X
    ZEPIY, // Zero page indexed with Y
    INDIN, // Indexed indirect (with X)
    ININD, // Indirect indexed (with Y)
    RELAT, // Relative
    ACCUM, // Accumulator
    // WDC's new modes
    ZEPIN, // Zero page indirect
};

struct Instruction
{
    int32_t position;
    QString instruction;
    uint8_t cycles;
    uint8_t length{0};
    std::array<uint8_t, 3> bytes;
    QString comment;
};

QList<Instruction> disassemble(Memory* memory, int32_t start = 0, int32_t end = std::numeric_limits<uint16_t>::max());
QList<Instruction> disassembleCount(Memory* memory, int32_t count, int32_t start = 0);
QList<QString> mnemonicList();
uint8_t searchOpcode(const QString& mnemonic, AddressingMode addressingMode);

} // namespace M6502
