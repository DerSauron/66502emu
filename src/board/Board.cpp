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

#include "Board.h"

#include "Bus.h"
#include "Clock.h"
#include "CPU.h"
#include "Debugger.h"
#include "Device.h"
#include <QChildEvent>
#include <QFile>
#include <QTimer>
#include <QThread>

namespace {

} // namespace

Board::Board(QObject* parent) :
    QObject{parent},
    addressBus_{new Bus{QStringLiteral("ADDRESS"), CPU::ADDRESS_BUS_WIDTH, this}},
    dataBus_{new Bus{QStringLiteral("DATA"), CPU::DATA_BUS_WIDTH, this}},
    busses_{},
    devices_{},
    resetLine_{WireState::High},
    rwLine_{WireState::Low},
    irqLine_{WireState::High},
    nmiLine_{WireState::High},
    syncLine_{WireState::Low},
    cpu_{new CPU{this}},
    clock_{new Clock{this}},
    debugger_{new Debugger(this)}
{
    connect(clock_, &Clock::clockCycleChanged, this, &Board::onClockCycleChanged);
}

Board::~Board()
{
}

void Board::setRwLine(WireState rwLine)
{
    if (rwLine == rwLine_)
        return;
    rwLine_ = rwLine;
    emit signalChanged();
}

void Board::setIrqLine(WireState irqLine)
{
    if (irqLine == irqLine_)
        return;
    irqLine_ = irqLine;
    emit signalChanged();
}

void Board::setNmiLine(WireState nmiLine)
{
    if (nmiLine == nmiLine_)
        return;
    nmiLine_ = nmiLine;
    emit signalChanged();
}

void Board::setResetLine(WireState resetLine)
{
    if (resetLine == resetLine_)
        return;
    resetLine_ = resetLine;
    emit signalChanged();
}

void Board::setSyncLine(WireState syncLine)
{
    if (syncLine == syncLine_)
        return;
    syncLine_ = syncLine;
    emit signalChanged();
}

const QVector<Device*>& Board::devices() const
{
    return devices_;
}

//Device* Board::device(const QString& name) const
//{
//    auto pos = std::find_if(devices_.begin(), devices_.end(), [&name](const auto& device) {
//        return device->name() == name;
//    });
//    if (pos == devices_.end())
//        return {};
//    return *pos;
//}

const QVector<Bus*>& Board::busses() const
{
    return busses_;
}

//Bus* Board::bus(const QString& name) const
//{
//    auto pos = std::find_if(busses_.begin(), busses_.end(), [&name](const auto& bus) {
//        return bus->name() == name;
//    });
//    if (pos == busses_.end())
//        return {};
//    return *pos;
//}

Device* Board::findDevice(int32_t address)
{
    for (auto& device : qAsConst(devices_))
    {
        if (address >= device->mapAddressStart() && address <= device->mapAddressEnd())
            return device;
    }
    return nullptr;
}

void Board::reset(QVector<Device*> devices, QVector<Bus*> busses)
{
    for (auto& d : devices)
    {
        d->moveToThread(thread());
        d->setParent(this);
    }

    for (auto& b : busses)
    {
        b->moveToThread(thread());
        b->setParent(this);
    }

    QMetaObject::invokeMethod(this, [this, devices, busses]() {
        qDeleteAll(devices_);
        devices_ = devices;

        qDeleteAll(busses_);
        busses_ = busses;

        emit resetted();
    });
}

void Board::onClockCycleChanged()
{
    StateEdge edge{StateEdge::Invalid};
    switch (clock_->state())
    {
        case WireState::High:
            edge = StateEdge::Raising;
            break;
        case WireState::Low:
            edge = StateEdge::Falling;
            break;
        default:
            break;
    }

    cpu_->clockEdge(edge);

    setIrqLine(WireState::High);
    setNmiLine(WireState::High);

    for (auto device : qAsConst(devices_))
    {
        device->clockEdge(edge);
    }

    debugger_->handleClockEdge(edge);
}
