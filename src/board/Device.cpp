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

#include "Device.h"

#include "Board.h"
#include "Bus.h"
#include "BusConnection.h"
#include "CPU.h"

Device::Device(const QString& name, Board* board) :
    QObject{board},
    mapAddressStart_{std::numeric_limits<uint16_t>::max()},
    chipWasSelected_{},
    chipSelected_{}
{
    setObjectName(name);
}

Device::~Device()
{
}

Board* Device::board() const
{
    auto b = dynamic_cast<Board*>(parent());
    Q_ASSERT(b);
    return b;
}

void Device::addBusConnection(const QString& portTagName, uint64_t portMask, Bus* bus, uint64_t busMask)
{
    const uint64_t portTag = mapPortTag(portTagName);
    busConnections_.append(BusConnection{portTag, portMask, bus, busMask});

}

//void Device::setName(const QString& name)
//{
//    if (name == name_)
//        return;

//    name_ = name;

//    emit nameChanged();
//}

void Device::clockEdge(StateEdge edge)
{
    // TODO move chip selecting to board
    chipWasSelected_ = chipSelected_;

    auto address = board()->addressBus()->typedData<uint16_t>();

    chipSelected_ = (address >= mapAddressStart()) && (address <= mapAddressEnd());

    if (chipSelected_ != chipWasSelected_)
        emit selectedChanged();

    deviceClockEdge(edge);
}
