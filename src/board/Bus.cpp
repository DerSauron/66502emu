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

#include "Bus.h"

#include "Board.h"

Bus::Bus(const QString& name, uint8_t width, Board* board) :
    QObject{nullptr},
    width_{width},
    data_{}
{
    assert(width_ == 8 || width_ == 16 || width_ == 32 || width_ == 64);
    setObjectName(name);
}

WireState Bus::bit(uint8_t bit) const
{
    assert(bit < width_);
    return toState(data_ & (1 << bit));
}

void Bus::setBit(uint8_t bit, WireState state)
{
    assert(bit < width_);
    uint64_t val = (isHigh(state) ? 1ULL : 0ULL) << bit;
    uint64_t mask = ~(1ULL << bit);
    data_ = (data_ & mask) | val;
}

void Bus::setData(uint64_t data)
{
    uint64_t widthMask = (1 << width_) - 1;
    uint64_t newData = data & widthMask;

    if (newData == data_)
        return;

    data_ = newData;

    emit dataChanged();
}

void Bus::setMaskedData(uint64_t data, uint64_t mask)
{
    uint64_t widthMask = (1 << width_) - 1;
    uint64_t newData = ((data_ & ~mask) | (data & mask)) & widthMask;

    if (newData == data_)
        return;

    data_ = newData;

    emit dataChanged();
}
