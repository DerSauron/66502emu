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

    bool isFailState() const { return failState_; }

    uint8_t lastInstruction() const { return lastInstruction_; }
    int32_t lastInstructionStart() const { return lastInstructionStart_; }
    uint8_t currentInstruction() const { return currentInstruction_; }
    int32_t currentInstructionStart() const { return currentInstructionStart_; }

    bool breakpointMatches(int address) const;

    void handleClockEdge(StateEdge edge);

signals:
    void newInstructionStart();
    void failStateChanged();

public slots:
    void stepInstruction();
    void stepSubroutine();

    void addBreakpoint(qint32 address);
    void removeBreakpoint(qint32 address);

private:
    void reset();
    void updateInstructionState(int32_t address);
    void updateCallStack();
    void stopAtBreakpoint(int32_t address);
    void stopAfterInstruction();
    void stopAfterSubroutine();
    void handleNewInstructionStart();
    void enterFailState();

private:
    enum class SteppingMode
    {
        None,
        Instruction,
        Subroutine,
    };

private:
    Board* board_;
    uint8_t brkOpcode_{};
    uint8_t jsrOpcode_{};
    uint8_t rtsOpcode_{};
    bool failState_{false};
    uint8_t lastInstruction_{};
    int32_t lastInstructionStart_{};
    uint8_t currentInstruction_{};
    int32_t currentInstructionStart_{};
    SteppingMode steppingMode_{SteppingMode::None};
    QSet<int32_t> breakpoints_;
    QVector<int32_t> callStack_;
    int steppingSubroutineCallStackStart_{};

    Q_DISABLE_COPY_MOVE(Debugger)
};
