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

#include "Memory.h"

#include "Board.h"
#include "Bus.h"
#include "UserState.h"
#include "utils/ArrayView.h"

namespace {

const QString kStateContent = QStringLiteral("content");

} // namespace

Memory::Memory(Type type, uint32_t size, const QString& name, Board* board) :
    Device{name, board},
    type_{type},
    size_{size},
    data_(size)
{
    setup();
}

Memory::~Memory()
{
    if (isPersistant())
        saveContent();
}

void Memory::setup()
{
    if (isPersistant())
        loadContent();
}

void Memory::setData(uint32_t index, const ArrayView& data)
{
    for (int i = 0; i < data.size(); i++)
    {
        data_[static_cast<int>(index) + i] = data[i];
    }
}

uint16_t Memory::calcMapAddressEnd() const
{
    return (mapAddressStart() - 1) + size_;
}

void Memory::deviceClockEdge(StateEdge edge)
{
    if (!isSelected())
        return;

    if (isRaising(edge))
    {
        Board* brd = board();

        uint16_t addr = brd->addressBus()->typedData<uint16_t>() - mapAddressStart();

        bool accessed = false;
        if (isHigh(brd->rwLine()))
        {
            brd->dataBus()->setData(data_[addr]);
            accessed = true;
        }
        else if (isLow(brd->rwLine()) && isWriteable())
        {
            data_[addr] = brd->dataBus()->typedData<uint8_t>();
            accessed = true;
        }

        if (accessed)
            emit byteAccessed(addr, isLow(brd->rwLine()));
    }
}

void Memory::loadContent()
{
    QString dataString = board()->userState()->viewValue(name(), kStateContent).toString();
    QByteArray data = QByteArray::fromBase64(dataString.toUtf8());
    for (uint32_t i = 0; i < qMin(static_cast<uint32_t>(data.size()), size_); i++)
    {
        data_[i] = static_cast<uint8_t>(data[i]);
    }
}

void Memory::saveContent()
{
    QByteArray data(reinterpret_cast<char*>(data_.data()), size_);
    QString dataString = QString::fromUtf8(data.toBase64());
    board()->userState()->setViewValue(name(), kStateContent, dataString);
}
