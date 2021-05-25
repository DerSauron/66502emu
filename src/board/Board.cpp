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

#include "Board.h"

#include "BoardLoader.h"
#include "Bus.h"
#include "Clock.h"
#include "CPU.h"
#include "Device.h"
#include "UserState.h"
#include <QChildEvent>
#include <QFile>
#include <QTimer>

namespace {

} // namespace

Board::Board(QObject* parent) :
    QObject{parent},
    userState_(new UserState()),
    addressBus_{new Bus{QStringLiteral("ADDRESS"), CPU::ADDRESS_BUS_WIDTH, this}},
    dataBus_{new Bus{QStringLiteral("DATA"), CPU::DATA_BUS_WIDTH, this}},
    resetLine_{WireState::High},
    rwLine_{WireState::Low},
    irqLine_{WireState::High},
    nmiLine_{WireState::High},
    syncLine_{WireState::Low},
    cpu_{new CPU{this}},
    clock_{new Clock{this}},
    deviceListReloadTriggered_{false},
    dbgSingleInstructionMode_{false}
{
    connect(clock_, &Clock::clockEdge, this, &Board::onClockEdge);

    clock_->setPeriod(2);
}

Board::~Board()
{
    // delete devices prior to qt's children deletion function to keep a valid board state during device destruction
    clearDevices();
}

bool Board::load(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    clearDevices();

    QString stateFileName = fileName + QLatin1String(".state");
    userState_->setFileName(stateFileName);

    if (!BoardLoader::load(&file, this))
        return false;

    return true;
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

Device* Board::device(const QString& name)
{
    return findChild<Device*>(name);
}

Bus* Board::bus(const QString& name)
{
    return findChild<Bus*>(name);
}

Device* Board::findDevice(uint16_t address)
{
    for (auto& device : devices_)
    {
        if (address >= device->mapAddressStart() && address <= device->mapAddressEnd())
            return device;
    }

    return nullptr;
}

void Board::clearDevices()
{
    const auto children = findChildren<Device*>(QString(), Qt::FindDirectChildrenOnly);
    for (auto& child : children)
    {
        delete child;
    }
}

void Board::startSingleInstructionStep()
{
    dbgSingleInstructionMode_ = true;
    clock_->start();
}

void Board::stopSingleInstructionStep()
{
    clock_->stop();
    dbgSingleInstructionMode_ = false;
}

void Board::childEvent(QChildEvent* event)
{
    if (!deviceListReloadTriggered_ && (event->added() || event->removed()))
    {
        // defer device list update to ensure fully created devices
        deviceListReloadTriggered_ = true;
        QTimer::singleShot(0, this, &Board::recreateDeviceList);
    }
}

void Board::recreateDeviceList()
{
    devices_ = findChildren<Device*>(QString(), Qt::FindDirectChildrenOnly);
    BoardLoader::validate(this);
    emit deviceListChanged();
    deviceListReloadTriggered_ = false;
}

void Board::onClockEdge(StateEdge edge)
{
    cpu_->clockEdge(edge);

    if (edge == StateEdge::Raising && syncLine_ == WireState::High && dbgSingleInstructionMode_)
        stopSingleInstructionStep();

    setIrqLine(WireState::High);
    setNmiLine(WireState::High);

    for (auto& device : devices_)
    {
        device->clockEdge(edge);
    }

    emit clockEdge(edge);
}
