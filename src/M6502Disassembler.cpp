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

namespace M6502 {

namespace {

// Exceptions for cycle counting
constexpr int CROSS_PAGE_ADDS_CYCLE = 1 << 0;
constexpr int BRANCH_TAKEN_ADDS_CYCLE = 1 << 1;

// The 6502's addressing modes
enum class AM
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

struct Opcode
{
    uint8_t number;             // Number of the opcode
    const char* mnemonic;       // Index in the name table
    AM addressing;              // Addressing mode
    uint8_t cycles;             // Number of cycles
    uint8_t cycles_exceptions;  // Mask of cycle-counting exceptions
};

class OpCodes
{
public:
    OpCodes();

    const Opcode* get(int index);

private:
    static const QVector<Opcode> _rawList;
    QVector<const Opcode*> map;
};

const QVector<Opcode> OpCodes::_rawList{
    {0x69, "ADC", AM::IMMED, 2, 0}, // ADC
    {0x65, "ADC", AM::ZEROP, 3, 0},
    {0x75, "ADC", AM::ZEPIX, 4, 0},
    {0x6D, "ADC", AM::ABSOL, 4, 0},
    {0x7D, "ADC", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x79, "ADC", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x61, "ADC", AM::INDIN, 6, 0},
    {0x71, "ADC", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x29, "AND", AM::IMMED, 2, 0}, // AND
    {0x25, "AND", AM::ZEROP, 3, 0},
    {0x35, "AND", AM::ZEPIX, 4, 0},
    {0x2D, "AND", AM::ABSOL, 4, 0},
    {0x3D, "AND", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x39, "AND", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x21, "AND", AM::INDIN, 6, 0},
    {0x31, "AND", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x0A, "ASL", AM::ACCUM, 2, 0}, // ASL
    {0x06, "ASL", AM::ZEROP, 5, 0},
    {0x16, "ASL", AM::ZEPIX, 6, 0},
    {0x0E, "ASL", AM::ABSOL, 6, 0},
    {0x1E, "ASL", AM::ABSIX, 7, 0},

    {0x90, "BCC", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BCC

    {0xB0, "BCS", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BCS

    {0xF0, "BEQ", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BEQ

    {0x24, "BIT", AM::ZEROP, 3, 0}, // BIT
    {0x2C, "BIT", AM::ABSOL, 4, 0},

    {0x30, "BMI", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BMI

    {0xD0, "BNE", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BNE

    {0x10, "BPL", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BPL

    {0x00, "BRK", AM::IMPLI, 7, 0}, // BRK

    {0x50, "BVC", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BVC

    {0x70, "BVS", AM::RELAT, 2, CROSS_PAGE_ADDS_CYCLE | BRANCH_TAKEN_ADDS_CYCLE}, // BVS

    {0x18, "CLC", AM::IMPLI, 2, 0}, // CLC

    {0xD8, "CLD", AM::IMPLI, 2, 0}, // CLD

    {0x58, "CLI", AM::IMPLI, 2, 0}, // CLI

    {0xB8, "CLV", AM::IMPLI, 2, 0}, // CLV

    {0xC9, "CMP", AM::IMMED, 2, 0}, // CMP
    {0xC5, "CMP", AM::ZEROP, 3, 0},
    {0xD5, "CMP", AM::ZEPIX, 4, 0},
    {0xCD, "CMP", AM::ABSOL, 4, 0},
    {0xDD, "CMP", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xD9, "CMP", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xC1, "CMP", AM::INDIN, 6, 0},
    {0xD1, "CMP", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xE0, "CPX", AM::IMMED, 2, 0}, // CPX
    {0xE4, "CPX", AM::ZEROP, 3, 0},
    {0xEC, "CPX", AM::ABSOL, 4, 0},

    {0xC0, "CPY", AM::IMMED, 2, 0}, // CPY
    {0xC4, "CPY", AM::ZEROP, 3, 0},
    {0xCC, "CPY", AM::ABSOL, 4, 0},

    {0xC6, "DEC", AM::ZEROP, 5, 0}, // DEC
    {0xD6, "DEC", AM::ZEPIX, 6, 0},
    {0xCE, "DEC", AM::ABSOL, 6, 0},
    {0xDE, "DEC", AM::ABSIX, 7, 0},

    {0xCA, "DEX", AM::IMPLI, 2, 0}, // DEX

    {0x88, "DEY", AM::IMPLI, 2, 0}, // DEY

    {0x49, "EOR", AM::IMMED, 2, 0}, // EOR
    {0x45, "EOR", AM::ZEROP, 3, 0},
    {0x55, "EOR", AM::ZEPIX, 4, 0},
    {0x4D, "EOR", AM::ABSOL, 4, 0},
    {0x5D, "EOR", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x59, "EOR", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x41, "EOR", AM::INDIN, 6, 1},
    {0x51, "EOR", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xE6, "INC", AM::ZEROP, 5, 0}, // INC
    {0xF6, "INC", AM::ZEPIX, 6, 0},
    {0xEE, "INC", AM::ABSOL, 6, 0},
    {0xFE, "INC", AM::ABSIX, 7, 0},

    {0xE8, "INX", AM::IMPLI, 2, 0}, // INX

    {0xC8, "INY", AM::IMPLI, 2, 0}, // INY

    {0x4C, "JMP", AM::ABSOL, 3, 0}, // JMP
    {0x6C, "JMP", AM::INDIA, 5, 0},

    {0x20, "JSR", AM::ABSOL, 6, 0}, // JSR

    {0xA9, "LDA", AM::IMMED, 2, 0}, // LDA
    {0xA5, "LDA", AM::ZEROP, 3, 0},
    {0xB5, "LDA", AM::ZEPIX, 4, 0},
    {0xAD, "LDA", AM::ABSOL, 4, 0},
    {0xBD, "LDA", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xB9, "LDA", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xA1, "LDA", AM::INDIN, 6, 0},
    {0xB1, "LDA", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0xA2, "LDX", AM::IMMED, 2, 0}, // LDX
    {0xA6, "LDX", AM::ZEROP, 3, 0},
    {0xB6, "LDX", AM::ZEPIY, 4, 0},
    {0xAE, "LDX", AM::ABSOL, 4, 0},
    {0xBE, "LDX", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},

    {0xA0, "LDY", AM::IMMED, 2, 0}, // LDY
    {0xA4, "LDY", AM::ZEROP, 3, 0},
    {0xB4, "LDY", AM::ZEPIX, 4, 0},
    {0xAC, "LDY", AM::ABSOL, 4, 0},
    {0xBC, "LDY", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},

    {0x4A, "LSR", AM::ACCUM, 2, 0}, // LSR
    {0x46, "LSR", AM::ZEROP, 5, 0},
    {0x56, "LSR", AM::ZEPIX, 6, 0},
    {0x4E, "LSR", AM::ABSOL, 6, 0},
    {0x5E, "LSR", AM::ABSIX, 7, 0},

    {0xEA, "NOP", AM::IMPLI, 2, 0}, // NOP

    {0x09, "ORA", AM::IMMED, 2, 0}, // ORA
    {0x05, "ORA", AM::ZEROP, 3, 0},
    {0x15, "ORA", AM::ZEPIX, 4, 0},
    {0x0D, "ORA", AM::ABSOL, 4, 0},
    {0x1D, "ORA", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x19, "ORA", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x01, "ORA", AM::INDIN, 6, 0},
    {0x11, "ORA", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x48, "PHA", AM::IMPLI, 3, 0}, // PHA

    {0x08, "PHP", AM::IMPLI, 3, 0}, // PHP

    {0x68, "PLA", AM::IMPLI, 4, 0}, // PLA

    {0x28, "PLP", AM::IMPLI, 4, 0}, // PLP

    {0x2A, "ROL", AM::ACCUM, 2, 0}, // ROL
    {0x26, "ROL", AM::ZEROP, 5, 0},
    {0x36, "ROL", AM::ZEPIX, 6, 0},
    {0x2E, "ROL", AM::ABSOL, 6, 0},
    {0x3E, "ROL", AM::ABSIX, 7, 0},

    {0x6A, "ROR", AM::ACCUM, 2, 0}, // ROR
    {0x66, "ROR", AM::ZEROP, 5, 0},
    {0x76, "ROR", AM::ZEPIX, 6, 0},
    {0x6E, "ROR", AM::ABSOL, 6, 0},
    {0x7E, "ROR", AM::ABSIX, 7, 0},

    {0x40, "RTI", AM::IMPLI, 6, 0}, // RTI

    {0x60, "RTS", AM::IMPLI, 6, 0}, // RTS

    {0xE9, "SBC", AM::IMMED, 2, 0}, // SBC
    {0xE5, "SBC", AM::ZEROP, 3, 0},
    {0xF5, "SBC", AM::ZEPIX, 4, 0},
    {0xED, "SBC", AM::ABSOL, 4, 0},
    {0xFD, "SBC", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xF9, "SBC", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0xE1, "SBC", AM::INDIN, 6, 0},
    {0xF1, "SBC", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},

    {0x38, "SEC", AM::IMPLI, 2, 0}, // SEC

    {0xF8, "SETableBuilderD", AM::IMPLI, 2, 0}, // SED

    {0x78, "SEI", AM::IMPLI, 2, 0}, // SEI

    {0x85, "STA", AM::ZEROP, 3, 0}, // STA
    {0x95, "STA", AM::ZEPIX, 4, 0},
    {0x8D, "STA", AM::ABSOL, 4, 0},
    {0x9D, "STA", AM::ABSIX, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x99, "STA", AM::ABSIY, 4, CROSS_PAGE_ADDS_CYCLE},
    {0x81, "STA", AM::INDIN, 6, 0},
    {0x91, "STA", AM::ININD, 5, CROSS_PAGE_ADDS_CYCLE},
    {0x92, "STA", AM::ZEPIN, 5, 0},

    {0x86, "STX", AM::ZEROP, 3, 0}, // STX
    {0x96, "STX", AM::ZEPIY, 4, 0},
    {0x8E, "STX", AM::ABSOL, 4, 0},

    {0x84, "STY", AM::ZEROP, 3, 0}, // STY
    {0x94, "STY", AM::ZEPIX, 4, 0},
    {0x8C, "STY", AM::ABSOL, 4, 0},

    {0xAA, "TAX", AM::IMPLI, 2, 0}, // TAX

    {0xA8, "TAY", AM::IMPLI, 2, 0}, // TAY

    {0xBA, "TSX", AM::IMPLI, 2, 0}, // TSX

    {0x8A, "TXA", AM::IMPLI, 2, 0}, // TXA

    {0x9A, "TXS", AM::IMPLI, 2, 0}, // TXS

    {0x98, "TYA", AM::IMPLI, 2, 0} // TYA
};

OpCodes::OpCodes() :
    map(256)
{
    for (const auto& def : _rawList)
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
            case AM::IMMED:
                return instruction(pc, op).operand("#$%1", byteOperand(memory, pc));

            case AM::ABSOL:
                return instruction(pc, op).operand("$%1", wordOperand(memory, pc));

            case AM::ZEROP:
                return instruction(pc, op).operand("$%1", byteOperand(memory, pc));

            case AM::IMPLI:
                return instruction(pc, op);

            case AM::INDIA:
                return instruction(pc, op).operand("($%1)", wordOperand(memory, pc));

            case AM::ABSIX:
                return instruction(pc, op).operand("$%1,X", wordOperand(memory, pc));

            case AM::ABSIY:
                return instruction(pc, op).operand("$%1,Y", wordOperand(memory, pc));

            case AM::ZEPIX:
                return instruction(pc, op).operand("$%1,X", byteOperand(memory, pc));

            case AM::ZEPIY:
                return instruction(pc, op).operand("$%1,Y", byteOperand(memory, pc));

            case AM::INDIN:
                return instruction(pc, op).operand("($%1,X)", byteOperand(memory, pc));

            case AM::ININD:
                return instruction(pc, op).operand("($%1),Y", byteOperand(memory, pc));

            case AM::RELAT:
            {
                auto operand = byteOperand(memory, pc);
                uint16_t addr = pc + 1;
                if (operand.value & 0x80U)
                    addr -= ((~operand.value & 0x7FU) + 1);
                else
                    addr += operand.value & 0x7FU;

                return instruction(pc, op).operand("$%1", Operand{addr, 2});
            }

            case AM::ACCUM:
                return instruction(pc, op).operand("A");

            // WDC's new modes
            case AM::ZEPIN:
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

} // namespace M6502
