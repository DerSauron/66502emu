/*
 * Original code:
 * Copyright (c) 1998-2014 Tennessee Carmel-Veilleux <veilleux@tentech.ca>
 * https://github.com/tcarmelveilleux/dcc6502
 *
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

#include "M6502Disassembler.h"

#include "board/Memory.h"
#include <QSet>

namespace M6502 {

namespace {

// Exceptions for cycle counting
constexpr int CROSS_PAGE_ADDS_CYCLE = 1 << 0;
constexpr int BRANCH_TAKEN_ADDS_CYCLE = 1 << 1;

struct Opcode
{
    uint8_t number;             // Number of the opcode
    const char* mnemonic;       // Index in the name table
    AddressingMode addressing;              // Addressing mode
    uint8_t cycles;             // Number of cycles
    uint8_t cycles_exceptions;  // Mask of cycle-counting exceptions
};

class OpCodes
{
public:
    OpCodes();

    const Opcode* get(int index);

private:
    QVector<const Opcode*> map;
};

const QVector<Opcode> rawOpCodeList{ // clazy:exclude=non-pod-global-static
    {0x69, "ADC", AddressingMode::IMMED, 2, 0}, // ADC
    {0x65, "ADC", AddressingMode::ZEROP, 3, 0},
    {0x75, "ADC", AddressingMode::ZEPIX, 4, 0},
    {0x6D, "ADC", AddressingMode::ABSOL, 4, 0},
    {0x7D, "ADC", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x79, "ADC", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x61, "ADC", AddressingMode::INDIN, 6, 0},
    {0x71, "ADC", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x29, "AND", AddressingMode::IMMED, 2, 0}, // AND
    {0x25, "AND", AddressingMode::ZEROP, 3, 0},
    {0x35, "AND", AddressingMode::ZEPIX, 4, 0},
    {0x2D, "AND", AddressingMode::ABSOL, 4, 0},
    {0x3D, "AND", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x39, "AND", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x21, "AND", AddressingMode::INDIN, 6, 0},
    {0x31, "AND", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x0A, "ASL", AddressingMode::ACCUM, 2, 0}, // ASL
    {0x06, "ASL", AddressingMode::ZEROP, 5, 0},
    {0x16, "ASL", AddressingMode::ZEPIX, 6, 0},
    {0x0E, "ASL", AddressingMode::ABSOL, 6, 0},
    {0x1E, "ASL", AddressingMode::ABSIX, 7, 0},

    {0x90, "BCC", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BCC

    {0xB0, "BCS", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BCS

    {0xF0, "BEQ", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BEQ

    {0x24, "BIT", AddressingMode::ZEROP, 3, 0}, // BIT
    {0x2C, "BIT", AddressingMode::ABSOL, 4, 0},

    {0x30, "BMI", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BMI

    {0xD0, "BNE", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BNE

    {0x10, "BPL", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BPL

    {0x00, "BRK", AddressingMode::IMPLI, 7, 0}, // BRK

    {0x50, "BVC", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BVC

    {0x70, "BVS", AddressingMode::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BVS

    {0x18, "CLC", AddressingMode::IMPLI, 2, 0}, // CLC

    {0xD8, "CLD", AddressingMode::IMPLI, 2, 0}, // CLD

    {0x58, "CLI", AddressingMode::IMPLI, 2, 0}, // CLI

    {0xB8, "CLV", AddressingMode::IMPLI, 2, 0}, // CLV

    {0xC9, "CMP", AddressingMode::IMMED, 2, 0}, // CMP
    {0xC5, "CMP", AddressingMode::ZEROP, 3, 0},
    {0xD5, "CMP", AddressingMode::ZEPIX, 4, 0},
    {0xCD, "CMP", AddressingMode::ABSOL, 4, 0},
    {0xDD, "CMP", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xD9, "CMP", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xC1, "CMP", AddressingMode::INDIN, 6, 0},
    {0xD1, "CMP", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xE0, "CPX", AddressingMode::IMMED, 2, 0}, // CPX
    {0xE4, "CPX", AddressingMode::ZEROP, 3, 0},
    {0xEC, "CPX", AddressingMode::ABSOL, 4, 0},

    {0xC0, "CPY", AddressingMode::IMMED, 2, 0}, // CPY
    {0xC4, "CPY", AddressingMode::ZEROP, 3, 0},
    {0xCC, "CPY", AddressingMode::ABSOL, 4, 0},

    {0xC6, "DEC", AddressingMode::ZEROP, 5, 0}, // DEC
    {0xD6, "DEC", AddressingMode::ZEPIX, 6, 0},
    {0xCE, "DEC", AddressingMode::ABSOL, 6, 0},
    {0xDE, "DEC", AddressingMode::ABSIX, 7, 0},

    {0xCA, "DEX", AddressingMode::IMPLI, 2, 0}, // DEX

    {0x88, "DEY", AddressingMode::IMPLI, 2, 0}, // DEY

    {0x49, "EOR", AddressingMode::IMMED, 2, 0}, // EOR
    {0x45, "EOR", AddressingMode::ZEROP, 3, 0},
    {0x55, "EOR", AddressingMode::ZEPIX, 4, 0},
    {0x4D, "EOR", AddressingMode::ABSOL, 4, 0},
    {0x5D, "EOR", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x59, "EOR", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x41, "EOR", AddressingMode::INDIN, 6, 1},
    {0x51, "EOR", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xE6, "INC", AddressingMode::ZEROP, 5, 0}, // INC
    {0xF6, "INC", AddressingMode::ZEPIX, 6, 0},
    {0xEE, "INC", AddressingMode::ABSOL, 6, 0},
    {0xFE, "INC", AddressingMode::ABSIX, 7, 0},

    {0xE8, "INX", AddressingMode::IMPLI, 2, 0}, // INX

    {0xC8, "INY", AddressingMode::IMPLI, 2, 0}, // INY

    {0x4C, "JMP", AddressingMode::ABSOL, 3, 0}, // JMP
    {0x6C, "JMP", AddressingMode::INDIA, 5, 0},

    {0x20, "JSR", AddressingMode::ABSOL, 6, 0}, // JSR

    {0xA9, "LDA", AddressingMode::IMMED, 2, 0}, // LDA
    {0xA5, "LDA", AddressingMode::ZEROP, 3, 0},
    {0xB5, "LDA", AddressingMode::ZEPIX, 4, 0},
    {0xAD, "LDA", AddressingMode::ABSOL, 4, 0},
    {0xBD, "LDA", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xB9, "LDA", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xA1, "LDA", AddressingMode::INDIN, 6, 0},
    {0xB1, "LDA", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xA2, "LDX", AddressingMode::IMMED, 2, 0}, // LDX
    {0xA6, "LDX", AddressingMode::ZEROP, 3, 0},
    {0xB6, "LDX", AddressingMode::ZEPIY, 4, 0},
    {0xAE, "LDX", AddressingMode::ABSOL, 4, 0},
    {0xBE, "LDX", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},

    {0xA0, "LDY", AddressingMode::IMMED, 2, 0}, // LDY
    {0xA4, "LDY", AddressingMode::ZEROP, 3, 0},
    {0xB4, "LDY", AddressingMode::ZEPIX, 4, 0},
    {0xAC, "LDY", AddressingMode::ABSOL, 4, 0},
    {0xBC, "LDY", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},

    {0x4A, "LSR", AddressingMode::ACCUM, 2, 0}, // LSR
    {0x46, "LSR", AddressingMode::ZEROP, 5, 0},
    {0x56, "LSR", AddressingMode::ZEPIX, 6, 0},
    {0x4E, "LSR", AddressingMode::ABSOL, 6, 0},
    {0x5E, "LSR", AddressingMode::ABSIX, 7, 0},

    {0xEA, "NOP", AddressingMode::IMPLI, 2, 0}, // NOP

    {0x09, "ORA", AddressingMode::IMMED, 2, 0}, // ORA
    {0x05, "ORA", AddressingMode::ZEROP, 3, 0},
    {0x15, "ORA", AddressingMode::ZEPIX, 4, 0},
    {0x0D, "ORA", AddressingMode::ABSOL, 4, 0},
    {0x1D, "ORA", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x19, "ORA", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x01, "ORA", AddressingMode::INDIN, 6, 0},
    {0x11, "ORA", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x48, "PHA", AddressingMode::IMPLI, 3, 0}, // PHA

    {0x08, "PHP", AddressingMode::IMPLI, 3, 0}, // PHP

    {0x68, "PLA", AddressingMode::IMPLI, 4, 0}, // PLA

    {0x28, "PLP", AddressingMode::IMPLI, 4, 0}, // PLP

    {0x2A, "ROL", AddressingMode::ACCUM, 2, 0}, // ROL
    {0x26, "ROL", AddressingMode::ZEROP, 5, 0},
    {0x36, "ROL", AddressingMode::ZEPIX, 6, 0},
    {0x2E, "ROL", AddressingMode::ABSOL, 6, 0},
    {0x3E, "ROL", AddressingMode::ABSIX, 7, 0},

    {0x6A, "ROR", AddressingMode::ACCUM, 2, 0}, // ROR
    {0x66, "ROR", AddressingMode::ZEROP, 5, 0},
    {0x76, "ROR", AddressingMode::ZEPIX, 6, 0},
    {0x6E, "ROR", AddressingMode::ABSOL, 6, 0},
    {0x7E, "ROR", AddressingMode::ABSIX, 7, 0},

    {0x40, "RTI", AddressingMode::IMPLI, 6, 0}, // RTI

    {0x60, "RTS", AddressingMode::IMPLI, 6, 0}, // RTS

    {0xE9, "SBC", AddressingMode::IMMED, 2, 0}, // SBC
    {0xE5, "SBC", AddressingMode::ZEROP, 3, 0},
    {0xF5, "SBC", AddressingMode::ZEPIX, 4, 0},
    {0xED, "SBC", AddressingMode::ABSOL, 4, 0},
    {0xFD, "SBC", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xF9, "SBC", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xE1, "SBC", AddressingMode::INDIN, 6, 0},
    {0xF1, "SBC", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x38, "SEC", AddressingMode::IMPLI, 2, 0}, // SEC

    {0xF8, "SETableBuilderD", AddressingMode::IMPLI, 2, 0}, // SED

    {0x78, "SEI", AddressingMode::IMPLI, 2, 0}, // SEI

    {0x85, "STA", AddressingMode::ZEROP, 3, 0}, // STA
    {0x95, "STA", AddressingMode::ZEPIX, 4, 0},
    {0x8D, "STA", AddressingMode::ABSOL, 4, 0},
    {0x9D, "STA", AddressingMode::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x99, "STA", AddressingMode::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x81, "STA", AddressingMode::INDIN, 6, 0},
    {0x91, "STA", AddressingMode::ININD, 5, CROSS_PAGE_ADDS_CYCLE},
    {0x92, "STA", AddressingMode::ZEPIN, 5, 0},

    {0x86, "STX", AddressingMode::ZEROP, 3, 0}, // STX
    {0x96, "STX", AddressingMode::ZEPIY, 4, 0},
    {0x8E, "STX", AddressingMode::ABSOL, 4, 0},

    {0x84, "STY", AddressingMode::ZEROP, 3, 0}, // STY
    {0x94, "STY", AddressingMode::ZEPIX, 4, 0},
    {0x8C, "STY", AddressingMode::ABSOL, 4, 0},

    {0xAA, "TAX", AddressingMode::IMPLI, 2, 0}, // TAX

    {0xA8, "TAY", AddressingMode::IMPLI, 2, 0}, // TAY

    {0xBA, "TSX", AddressingMode::IMPLI, 2, 0}, // TSX

    {0x8A, "TXA", AddressingMode::IMPLI, 2, 0}, // TXA

    {0x9A, "TXS", AddressingMode::IMPLI, 2, 0}, // TXS

    {0x98, "TYA", AddressingMode::IMPLI, 2, 0} // TYA
};

OpCodes::OpCodes() :
    map(256)
{
    for (const auto& def : rawOpCodeList)
    {
        map[def.number] = &def;
    }
};

const Opcode* OpCodes::get(int index)
{
    return map[index];
}

Q_GLOBAL_STATIC(OpCodes, opCodes);

struct Operand
{
    uint16_t value;
    uint8_t length;
};

struct InstructionBuilder
{
    Instruction instruction;

    InstructionBuilder& operand(const char* opstr)
    {
        instruction.instruction += QLatin1Char(' ') + QLatin1String(opstr);
        return *this;
    }

    InstructionBuilder& operand(const char* templ, const Operand& operand)
    {
        uint32_t opValue = operand.value;
        instruction.instruction += QLatin1Char(' ') +
                QString(QLatin1String(templ)).arg(opValue, static_cast<int>(operand.length * 2), 16, QLatin1Char('0'));
        for (uint8_t i = 0; i < operand.length; i++)
        {
            instruction.bytes[instruction.length++] = opValue & 0xFF;
            opValue >>= 8;
        }
        return *this;
    }

    InstructionBuilder& comment(const char* com)
    {
        instruction.comment = QLatin1String(com);
        return *this;
    }

    operator Instruction() const { return instruction; }
};

inline InstructionBuilder instruction(uint16_t& pc, const Opcode* opcode)
{
    InstructionBuilder builder;
    builder.instruction.position = pc++;
    if (opcode)
    {
        builder.instruction.instruction = QString::fromUtf8(opcode->mnemonic);
        builder.instruction.bytes[0] = opcode->number;
        builder.instruction.length++;
    }
    else
    {
        builder.instruction.instruction = QStringLiteral(".byte");
    }
    return builder;
}

inline Operand byteOperand(const Memory* mem, uint16_t& pc)
{
    return {mem->byte(pc++), 1};
}

inline Operand wordOperand(const Memory* mem, uint16_t& pc)
{
    auto operand = static_cast<uint16_t>(mem->byte(pc++));
    operand += (static_cast<uint16_t>(mem->byte(pc++)) << 8);
    return {operand, 2};
}

Instruction decodeInstruction(const Memory* memory, uint16_t& pc)
{
    uint8_t byte = memory->byte(pc);

    const Opcode* op = opCodes->get(byte);

    if (op)
    {
        switch (op->addressing)
        {
            case AddressingMode::IMMED:
                return instruction(pc, op).operand("#$%1", byteOperand(memory, pc));

            case AddressingMode::ABSOL:
                return instruction(pc, op).operand("$%1", wordOperand(memory, pc));

            case AddressingMode::ZEROP:
                return instruction(pc, op).operand("$%1", byteOperand(memory, pc));

            case AddressingMode::IMPLI:
                return instruction(pc, op);

            case AddressingMode::INDIA:
                return instruction(pc, op).operand("($%1)", wordOperand(memory, pc));

            case AddressingMode::ABSIX:
                return instruction(pc, op).operand("$%1,X", wordOperand(memory, pc));

            case AddressingMode::ABSIY:
                return instruction(pc, op).operand("$%1,Y", wordOperand(memory, pc));

            case AddressingMode::ZEPIX:
                return instruction(pc, op).operand("$%1,X", byteOperand(memory, pc));

            case AddressingMode::ZEPIY:
                return instruction(pc, op).operand("$%1,Y", byteOperand(memory, pc));

            case AddressingMode::INDIN:
                return instruction(pc, op).operand("($%1,X)", byteOperand(memory, pc));

            case AddressingMode::ININD:
                return instruction(pc, op).operand("($%1),Y", byteOperand(memory, pc));

            case AddressingMode::RELAT:
            {
                auto operand = byteOperand(memory, pc);
                uint16_t addr = pc + 1;
                if (operand.value & 0x80U)
                    addr -= ((~operand.value & 0x7FU) + 1);
                else
                    addr += operand.value & 0x7FU;

                return instruction(pc, op).operand("$%1", Operand{addr, 2});
            }

            case AddressingMode::ACCUM:
                return instruction(pc, op).operand("A");

            // WDC's new modes
            case AddressingMode::ZEPIN:
                return instruction(pc, op).operand("$%1)", byteOperand(memory, pc)).comment("WDC's new mode");
        }
    }

    return instruction(pc, nullptr).operand("$%1", Operand{byte, 1}).comment("Invalid opcode");
}

} // namespace

QList<Instruction> disassemble(Memory* memory, uint16_t start, uint16_t end)
{
    QList<Instruction> instructions;
    uint16_t pc = start;
    while (pc < end)
    {
        instructions << decodeInstruction(memory, pc);
    }
    return instructions;
}

QList<Instruction> disassembleCount(Memory* memory, uint16_t count, uint16_t start)
{
    QList<Instruction> instructions;
    uint16_t pc = start;
    for (uint16_t i = 0; i < count && pc < memory->size() - 3; i++)
    {
        instructions << decodeInstruction(memory, pc);
    }
    return instructions;
}

QList<QString> mnemonicList()
{
    QSet<QString> unique;
    for (const auto& opcode : rawOpCodeList)
    {
        unique.insert(QLatin1String(opcode.mnemonic));
    }
    return unique.values();
}

uint8_t searchOpcode(const QString& mnemonic, AddressingMode addressingMode)
{
    for (const auto& opcode : rawOpCodeList)
    {
        if ((QLatin1String(opcode.mnemonic).compare(mnemonic, Qt::CaseInsensitive) == 0) &&
            (opcode.addressing == addressingMode))
            return opcode.number;
    }
    return 0xEA;
}

} // namespace M6502
