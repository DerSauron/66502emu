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

#pragma once

#include "WireState.h"
#include <QObject>
#include <QSet>

class Board;

class Debugger : public QObject
{
    Q_OBJECT

public:
    explicit Debugger(Board* board);
    ~Debugger() override;

    uint8_t lastInstruction() const { return lastInstruction_; }
    uint16_t lastInstructionStart() const { return lastInstructionStart_; }
    uint8_t currentInstruction() const { return currentInstruction_; }
    uint16_t currentInstructionStart() const { return currentInstructionStart_; }

    bool breakpointMatches(int address) const;

    void handleClockEdge(StateEdge edge);

signals:
    void newInstructionStart();

public slots:
    void stepInstruction();
    void stepSubroutine();

    void addBreakpoint(int address);
    void removeBreakpoint(int address);

private:
    void updateInstructionState(uint16_t address);
    void updateCallStack();
    void stopAtBreakpoint(uint16_t address);
    void stopAfterInstruction();
    void stopAfterSubroutine();
    void handleNewInstructionStart();

private:
    enum class SteppingMode
    {
        None,
        Instruction,
        Subroutine,
    };

private:
    Board* board_;
    uint8_t jsrOpcode_{};
    uint8_t rtsOpcode_{};
    uint8_t lastInstruction_{};
    uint16_t lastInstructionStart_{};
    uint8_t currentInstruction_{};
    uint16_t currentInstructionStart_{};
    SteppingMode steppingMode_{SteppingMode::None};
    QSet<int> breakpoints_;
    QVector<int> callStack_;
    int steppingSubroutineCallStackStart_{};
};
