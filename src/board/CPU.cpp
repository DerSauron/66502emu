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

#include "CPU.h"

#include "Board.h"
#include "impl/m6502.h"

CPU::CPU(Board* board) :
    QObject{board},
    board_{board},
    chip_{new m6502_t}
{
    m6502_desc_t init;
    pinState_ = m6502_init(chip_, &init);
    populateState();
}

CPU::~CPU()
{
    delete chip_;
}

void CPU::clockEdge(StateEdge edge)
{
    if (isFalling(edge))
    {
        injectState();

        pinState_ = m6502_tick(chip_, pinState_);

        populateState();

        emit stepped();
    }
}

uint64_t CPU::registerA() const
{
    return m6502_a(chip_);
}

uint64_t CPU::registerX() const
{
    return m6502_x(chip_);
}

uint64_t CPU::registerY() const
{
    return m6502_y(chip_);
}

uint64_t CPU::registerS() const
{
    return m6502_s(chip_);
}

uint64_t CPU::registerPC() const
{
    return m6502_pc(chip_);
}

uint64_t CPU::registerIR() const
{
    return chip_->IR;
}

uint64_t CPU::flags() const
{
    return m6502_p(chip_);
}

void CPU::setPin(uint64_t pin, WireState state)
{
    if (isHigh(state))
        pinState_ |= pin;
    else if (isLow(state))
        pinState_ &= ~pin;
}

void CPU::injectState()
{
    setPin(M6502_RES, negate(board_->resetLine()));
    setPin(M6502_IRQ, negate(board_->irqLine()));
    setPin(M6502_NMI, negate(board_->nmiLine()));

    if (isHigh(toState(pinState_ & M6502_RW)))
        M6502_SET_DATA(pinState_, board_->dataBus()->data());
}

void CPU::populateState()
{
    board_->setResetLine(negate(toState(pinState_ & M6502_RES)));
    board_->setRwLine(toState(pinState_ & M6502_RW));
    board_->setSyncLine(toState(pinState_ & M6502_SYNC));

    board_->addressBus()->setData(M6502_GET_ADDR(pinState_));

    if (isLow(toState(pinState_ & M6502_RW)))
        board_->dataBus()->setData(M6502_GET_DATA(pinState_));
}
