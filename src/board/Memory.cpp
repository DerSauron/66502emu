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

#include "Memory.h"

#include "Board.h"
#include "Bus.h"
#include "UserState.h"
#include "utils/ArrayView.h"

namespace {

} // namespace

Memory::Memory(Type type, int32_t size, const QString& name, Board* board) :
    Device{name, board},
    type_{type},
    data_(size),
    lastAccessAddress_{0},
    lastAccessWasWrite_{false}
{
    setup();
}

Memory::~Memory()
{
}

void Memory::setup()
{
}

void Memory::setData(int32_t index, const ArrayView& data)
{
    for (int i = 0; i < data.size(); i++)
    {
        data_[index + i] = data[i];
    }
}

int32_t Memory::calcMapAddressEnd() const
{
    return (mapAddressStart() - 1) + data_.size();
}

void Memory::deviceClockEdge(StateEdge edge)
{
    if (!isSelected())
        return;

    if (isRaising(edge))
    {
        Board* brd = board();

        int32_t addr = brd->addressBus()->typedData<int32_t>() - mapAddressStart();

        bool wasAccessed = false;
        if (isHigh(brd->rwLine()))
        {
            brd->dataBus()->setData(data_[addr]);
            wasAccessed = true;
        }
        else if (isLow(brd->rwLine()) && isWriteable())
        {
            data_[addr] = brd->dataBus()->typedData<uint8_t>();
            wasAccessed = true;
        }

        if (wasAccessed)
        {
            lastAccessAddress_ = addr;
            lastAccessWasWrite_ = isLow(brd->rwLine());
            emit accessed();
        }
    }
}
