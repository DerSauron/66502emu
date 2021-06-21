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

#include "Bus.h"
#include "WireState.h"
#include <QObject>

class Board;

extern "C" {
struct _m6502_t;
typedef _m6502_t m6502_t;
}

class CPU : public QObject
{
    Q_OBJECT

public:
    static constexpr uint8_t ADDRESS_BUS_WIDTH = 16;
    static constexpr uint8_t DATA_BUS_WIDTH = 8;

public:
    explicit CPU(Board* board);
    ~CPU();

    void clockEdge(StateEdge edge);

    uint64_t registerA() const;
    uint64_t registerX() const;
    uint64_t registerY() const;
    uint64_t registerS() const;
    uint64_t registerPC() const;
    uint64_t registerIR() const;
    uint64_t flags() const;

signals:
    void stepped();

private:
    void setPin(uint64_t pin, WireState state);

    void injectState();
    void populateState();

private:
    Board* board_;
    m6502_t* chip_;
    uint64_t pinState_;
};
