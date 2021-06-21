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

#include "Debugger.h"

#include "Board.h"
#include "Bus.h"
#include "Clock.h"
#include "M6502Disassembler.h"
#include <QThread>
#include <QTimer>

Debugger::Debugger(Board* board) :
    QObject(board),
    board_(board)
{
    jsrOpcode_ = M6502::searchOpcode(QStringLiteral("jsr"), M6502::AddressingMode::ABSOL);
    Q_ASSERT(jsrOpcode_ != 0xEA);
    rtsOpcode_ = M6502::searchOpcode(QStringLiteral("rts"), M6502::AddressingMode::IMPLI);
    Q_ASSERT(rtsOpcode_ != 0xEA);

    callStack_.reserve(1024);
}

Debugger::~Debugger()
{
}

bool Debugger::breakpointMatches(int address) const
{
    const auto pos = breakpoints_.find(address);
    return pos != breakpoints_.end();
}

void Debugger::stepInstruction()
{
    steppingMode_ = SteppingMode::Instruction;
    board_->clock()->start();
}

void Debugger::stepSubroutine()
{
    if (currentInstruction_ == jsrOpcode_)
    {
        steppingMode_ = SteppingMode::Subroutine;
        steppingSubroutineCallStackStart_ = callStack_.size();
        board_->clock()->start();
    }
    else
    {
        stepInstruction();
    }
}

void Debugger::addBreakpoint(int address)
{
    Q_ASSERT(QThread::currentThread() == thread());
    breakpoints_.insert(address);
}

void Debugger::removeBreakpoint(int address)
{
    Q_ASSERT(QThread::currentThread() == thread());
    breakpoints_.remove(address);
}

void Debugger::updateInstructionState(uint16_t address)
{
    lastInstruction_ = currentInstruction_;
    lastInstructionStart_ = currentInstructionStart_;

    currentInstruction_ = board_->dataBus()->typedData<uint8_t>();
    currentInstructionStart_ = address;
}

void Debugger::updateCallStack()
{
    if (lastInstruction_ == jsrOpcode_)
        callStack_.push_back(lastInstructionStart_);
    else if (lastInstruction_ == rtsOpcode_)
        callStack_.pop_back();
}

void Debugger::stopAtBreakpoint(uint16_t address)
{
    if (breakpointMatches(address))
    {
        board_->clock()->stop();
    }
}

void Debugger::stopAfterInstruction()
{
    if (steppingMode_ == SteppingMode::Instruction)
    {
        board_->clock()->stop();
        steppingMode_ = SteppingMode::None;
    }
}

void Debugger::stopAfterSubroutine()
{
    if (steppingMode_ != SteppingMode::Subroutine)
        return;

    if (lastInstruction_ == rtsOpcode_ && callStack_.size() == steppingSubroutineCallStackStart_)
    {
        board_->clock()->stop();
        steppingMode_ = SteppingMode::None;
    }
}

void Debugger::handleNewInstructionStart()
{
    uint16_t address = board_->addressBus()->typedData<uint16_t>();

    updateInstructionState(address);
    updateCallStack();

    stopAtBreakpoint(address);
    stopAfterInstruction();
    stopAfterSubroutine();

    emit newInstructionStart();
}

void Debugger::handleClockEdge(StateEdge edge)
{
    if (edge == StateEdge::Raising && board_->syncLine() == WireState::High)
    {
        handleNewInstructionStart();
    }
}
