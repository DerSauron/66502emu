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

#include "LCD.h"

#include "Board.h"
#include "Bus.h"
#include "BusConnection.h"
#include "impl/hd44780u.h"
#include "utils/ArrayView.h"
#include <QTimer>

using PinMask = hd44780u::MPUPinMask;

namespace {

enum Tags
{
    DATA,
    RS,
    RW,
    EN
};

} // namespace

LCD::LCD(const QString& name, Board* board) :
    Device(name, board),
    chip_(new hd44780u()),
    pins_{},
    cursorPos_{},
    cursorOn_{true}
{
    setup();
}

LCD::~LCD()
{
}

void LCD::setup()
{
    chip_->setListener(this);
}

ArrayView LCD::charMatrix(uint8_t address) const
{
    return chip_->charMatrix(address);
}

uint64_t LCD::mapPortTag(const QString& portTagName) const
{
    if (portTagName == QLatin1String{"DATA"})
        return Tags::DATA;
    else if (portTagName == QLatin1String{"RS"})
        return Tags::RS;
    else if (portTagName == QLatin1String{"RW"})
        return Tags::RW;
    else if (portTagName == QLatin1String{"EN"})
        return Tags::EN;
    else
        return Device::mapPortTag(portTagName);
}

void LCD::deviceClockEdge(StateEdge edge)
{
    // interact on every edge

    injectState();

    pins_ = chip_->cycle(pins_);

    populateState();
}

void LCD::injectState()
{
    for (const auto& bc : qAsConst(busConnections_))
    {
        const uint16_t busData = static_cast<uint16_t>(extractBits(bc.bus()->data(), bc.busMask()));
        if (bc.portTag() == Tags::DATA)
            pins_ = hd44780u::inject(pins_, PinMask::Data, busData);
        else if (bc.portTag() == Tags::RS)
            pins_ = hd44780u::inject(pins_, PinMask::RS, busData);
        else if (bc.portTag() == Tags::RW)
            pins_ = hd44780u::inject(pins_, PinMask::RW, busData);
        else if (bc.portTag() == Tags::EN)
            pins_ = hd44780u::inject(pins_, PinMask::EN, busData);
    }
}

void LCD::populateState()
{
    if (chip_->wasReadInstruction())
    {
        for (const auto& bc : qAsConst(busConnections_))
        {
            if (bc.portTag() == Tags::DATA)
            {
                const uint16_t pinData = hd44780u::extract(pins_, PinMask::Data);
                const uint64_t busData = injectBits(uint16_t{}, static_cast<uint16_t>(bc.busMask()), pinData);
                bc.bus()->setMaskedData(busData, bc.busMask());
                break;
            }
        }
    }
}

void LCD::onCharacterChanged(uint8_t address)
{
    emit characterChanged(address);
}

void LCD::onBusyChanged()
{
    emit busyChanged();
}

void LCD::onCursorPosChanged()
{
    emit cursorPosChanged();
}

void LCD::onCursorChanged()
{
    emit cursorChanged();
}

void LCD::onShiftPosChanged()
{
    emit displayShiftChanged();
}

void LCD::onDisplayChanged()
{
    emit displayChanged();
}

uint8_t LCD::bufferWidth() const
{
    return 40; // QFS
}

bool LCD::isBusy() const
{
    return chip_->isBusy();
}

uint16_t LCD::cursorPos() const
{
    return chip_->cursorPos();
}

uint8_t LCD::displayShift() const
{
    return chip_->displayShift();
}

bool LCD::isDisplayOn() const
{
    return chip_->isDisplayOn();
}

bool LCD::isCursorOn() const
{
    return chip_->isCursorOn();
}
