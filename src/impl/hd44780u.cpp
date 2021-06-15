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

#include "hd44780u.h"

#include "hd44780u_data.h"
#include "utils/ArrayView.h"
#include <QTimer>

namespace {

hd44780u::listener DummyListener; // clazy:exclude=non-pod-global-static

bool enUp(uint16_t oldPins, uint16_t newPins)
{
    return ((hd44780u::extract(oldPins, hd44780u::MPUPinMask::EN)) == 0) &&
            ((hd44780u::extract(newPins, hd44780u::MPUPinMask::EN)) != 0);

}

bool enDown(uint16_t oldPins, uint16_t newPins)
{
    return ((hd44780u::extract(oldPins, hd44780u::MPUPinMask::EN)) != 0) &&
            ((hd44780u::extract(newPins, hd44780u::MPUPinMask::EN)) == 0);

}

}  // namespace

hd44780u::hd44780u() :
    listener_{&DummyListener},
    blinker_(new QTimer())
{
    blinker_->setSingleShot(false);
    blinker_->setInterval(600);
    QTimer::connect(blinker_.get(), &QTimer::timeout, [this]() { onBlinkTick(); });

    std::fill(ddram_.begin(), ddram_.end(), 0x20);
    std::fill(cgram_.begin(), cgram_.end(), 0);

    cursorControl();
}

void hd44780u::setListener(listener* listener)
{
    listener_ = listener;
}

uint16_t hd44780u::cursorPos() const
{
    return cursorPos_;
}

uint8_t hd44780u::charAt(uint8_t address) const
{
    return ddram_[address];
}

ArrayView hd44780u::charMatrix(uint8_t address) const
{
    uint8_t addrBase = charAt(address);

    const uint8_t* ptr;
    if (extractBits(addrBase, uint8_t{0xF0}) == 0)
    {
        uint8_t addr = static_cast<uint8_t>((addrBase & 0x7u) << 3u);
        ptr = &cgram_[addr];
    }
    else
    {
        uint16_t addr = static_cast<uint16_t>((((addrBase & 0xF0u) - 0x10u) | (addrBase & 0x0Fu)) << 3u);
        ptr = &characterRom[addr];
    }
    return ArrayView(ptr, 8);
}

uint16_t hd44780u::cycle(uint16_t pins)
{
    auto rs = extract(pins, MPUPinMask::RS);
    auto rw = extract(pins, MPUPinMask::RW);

    wasReadInstruction_ = false;

    if (busy_)
    {
        busy_--;
        if (!busy_)
            listener_->onBusyChanged();

        if (rs != 0 || rw != 1)
            return pins; // busy do nothing
    }

    if (enUp(mpuPins_, pins))
    {
        if (rw == 1) // reading
        {
            if (rs == 0) // IR
                pins = inject(pins, MPUPinMask::Data, currentState());
            else // (rs == 1) DR
                pins = inject(pins, MPUPinMask::Data, data());

            wasReadInstruction_ = true;
        }
    }
    else if (enDown(mpuPins_, pins))
    {
        if ((rs != extract(mpuPins_, MPUPinMask::RS)) || (rw != extract(mpuPins_, MPUPinMask::RW)))
            return pins; // invalid usage, ignore it

        if (rw == 0) // writing
        {
            auto data = static_cast<uint8_t>(extract(pins, MPUPinMask::Data));
            if (rs == 0) // IR
                handleInstruction(data);
            else // (rs == 1) DR
                setData(data);

            busy_ = busyDelay_;
            listener_->onBusyChanged();
        }
    }

    mpuPins_ = pins;

    return pins;
}

void hd44780u::onBlinkTick()
{
    cursorOn_ = !cursorOn_;
    listener_->onCursorChanged();
}

void hd44780u::cursorControl()
{
    bool cursorWasOn = cursorOn_;

    if (cursorEnabled_)
    {
        cursorOn_ = true;

        if (cursorBlink_)
            blinker_->start();
        else
            blinker_->stop();
    }
    else
    {
        cursorOn_ = false;
        blinker_->stop();
    }

    if (cursorWasOn != cursorOn_)
        listener_->onCursorChanged();
}

void hd44780u::handleInstruction(uint8_t cmd)
{
    readDataValid_ = false;

    if ((cmd & 0b11111111) == 0b00000001)
    {
        clearInstruction();
    }
    else if ((cmd & 0b11111110) == 0b00000010)
    {
        returnHomeInstruction();
    }
    else if ((cmd & 0b11111100) == 0b00000100)
    {
        entryModeSetInstruction(cmd);
    }
    else if ((cmd & 0b11111000) == 0b00001000)
    {
        displayControlInstruction(cmd);
    }
    else if ((cmd & 0b11110000) == 0b00010000)
    {
        cursorInstruction(cmd);
        readDataValid_ = true;
    }
    else if ((cmd & 0b11100000) == 0b00100000)
    {
        funtionSetInstruction(cmd);
    }
    else if ((cmd & 0b11000000) == 0b01000000)
    {
        setCGRAMAddrInstruction(cmd);
        readDataValid_ = true;
    }
    else if ((cmd & 0b10000000) == 0b10000000)
    {
        setDDRAMAddrInstruction(cmd);
        readDataValid_ = true;
    }
}

uint8_t hd44780u::currentState()
{
    return ((busy_) ? 0x8 : 0x0) | (ramAddr_ & 0x7F);
}

void hd44780u::setData(uint8_t data)
{
    if (ramSelector_ == 0) // ddram
    {
        ddram_[ramAddr_] = data;
        listener_->onCharacterChanged(static_cast<uint8_t>(ramAddr_));
    }
    else // (ramSelector_ == 1) cgram
    {
        cgram_[ramAddr_] = data;
        notifyChangedCharacters(ramAddr_);
    }

    readDataValid_ = false;

    if (ramSelector_ == 0 && shiftEnabled_)
        moveDisplay(increment_);
    moveCursor(increment_);
}

uint8_t hd44780u::data()
{
    if (!readDataValid_)
        return 0xFF;

    uint8_t d = 0;

    if (ramSelector_ == 0) // ddram
    {
        d = ddram_[ramAddr_];
    }
    else // (ramSelector_ == 1) cgram
    {
        d = cgram_[ramAddr_];
    }

    moveCursor(increment_);

    return d;
}

void hd44780u::clearInstruction()
{
    for (size_t i = 0; i < ddram_.size(); i++)
    {
        ddram_[i] = 0x20;
        listener_->onCharacterChanged(static_cast<uint8_t>(i));
    }
    increment_ = true;
    returnHomeInstruction();
}

void hd44780u::returnHomeInstruction()
{
    setDDRAMAddrInstruction(0);
    shift_ = 0;
    listener_->onShiftPosChanged();
}

void hd44780u::entryModeSetInstruction(uint8_t cmd)
{
    increment_ = extractBits(cmd, uint8_t{0b10});
    shiftEnabled_ = extractBits(cmd, uint8_t{0b1});
}

void hd44780u::displayControlInstruction(uint8_t cmd)
{
    displayOn_ = extractBits(cmd, uint8_t{0b100});
    cursorEnabled_ = extractBits(cmd, uint8_t{0b10});
    cursorBlink_ = extractBits(cmd, uint8_t{0b1});
    cursorControl();
    listener_->onDisplayChanged();
}

void hd44780u::cursorInstruction(uint8_t cmd)
{
    const bool inc = extractBits(cmd, uint8_t{0b0100}) == 1;

    ramSelector_ = 0;

    if (extractBits(cmd, uint8_t{0b1000}) == 0) // move cursor
    {
        moveCursor(inc);
    }
    else // shift display
    {
        moveDisplay(inc);
    }
}

void hd44780u::funtionSetInstruction(uint8_t cmd)
{
    // TODO implement funtionSetInstruction()
}

void hd44780u::setCGRAMAddrInstruction(uint8_t cmd)
{
    ramSelector_ = 1;
    ramAddr_ = cmd & 0b00111111;
}

void hd44780u::setDDRAMAddrInstruction(uint8_t cmd)
{
    ramSelector_ = 0;
    ramAddr_ = cmd & 0b01111111;
    updateCursorPos();
    listener_->onCursorPosChanged();
}

void hd44780u::updateCursorPos()
{
    cursorPos_ = ramAddr_;
}

void hd44780u::moveDisplay(bool increment)
{
    if (increment) // right
    {
        if (shift_ == 39)
            shift_ = 0;
        else
            shift_++;
    }
    else //right
    {
        if (shift_ == 0)
            shift_ = 39;
        else
            shift_--;
    }
    listener_->onShiftPosChanged();
}

void hd44780u::moveCursor(bool increment)
{
    if (increment) // right
    {
        if (ramAddr_ == 79)
            ramAddr_ = 0;
        else
            ramAddr_++;
    }
    else //right
    {
        if (ramAddr_ == 0)
            ramAddr_ = 79;
        else
            ramAddr_--;
    }
    if (ramSelector_ == 0)
        updateCursorPos();
    listener_->onCursorPosChanged();
}

void hd44780u::notifyChangedCharacters(uint8_t cgRamAddr)
{
    uint8_t cgRamChar = cgRamAddr >> 3u;
    for (uint8_t i = 0; i < ddram_.size(); i++)
    {
        if (ddram_[i] == cgRamChar)
            listener_->onCharacterChanged(i);
    }
}
