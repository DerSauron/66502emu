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

#include "BoardLoader.h"
#include "Bus.h"
#include "Clock.h"
#include "CPU.h"
#include "Debugger.h"
#include "Device.h"
#include "UserState.h"
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

void Board::load(const QString& fileName)
{
    if (QThread::currentThread() == thread())
        loadImpl(fileName);
    QMetaObject::invokeMethod(this, "loadImpl", Q_ARG(QString, fileName));
}

void Board::loadImpl(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        emit loadingFinished(false);
        return;
    }

    clearDevices();

    if (!BoardLoader::load(&file, this))
    {
        emit loadingFinished(false);
        return;
    }

    BoardLoader::validate(this);

    emit loadingFinished(true);
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

QList<Device*> Board::devices() const
{
    return findChildren<Device*>(QString(), Qt::FindDirectChildrenOnly);
}

Device* Board::device(const QString& name)
{
    return findChild<Device*>(name, Qt::FindDirectChildrenOnly);
}

Bus* Board::bus(const QString& name)
{
    return findChild<Bus*>(name, Qt::FindDirectChildrenOnly);
}

Device* Board::findDevice(uint16_t address)
{
    const QList<Device*> _devices = devices();
    for (auto& device : _devices)
    {
        if (address >= device->mapAddressStart() && address <= device->mapAddressEnd())
            return device;
    }
    return nullptr;
}

void Board::clearDevices()
{
    const QList<Device*> _devices = devices();
    for (auto device : _devices)
    {
        delete device;
    }
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

    const QList<Device*> _devices = devices();
    for (auto device : _devices)
    {
        device->clockEdge(edge);
    }

    debugger_->handleClockEdge(edge);
}
