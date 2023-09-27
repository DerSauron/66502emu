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

#pragma once

#include "utils/Bits.h"
#include <QScopedPointer>
#include <array>
#include <cinttypes>
#include <limits>

class ArrayView;
class QTimer;

class hd44780u
{
public:
    class listener
    {
    public:
        virtual ~listener() {}

        virtual void onCharacterChanged(uint8_t address) {}
        virtual void onBusyChanged() {}
        virtual void onCursorPosChanged() {}
        virtual void onCursorChanged() {}
        virtual void onShiftPosChanged() {}
        virtual void onDisplayChanged() {}
    };

    enum class MPUPinMask : uint16_t
    {
        Data = 0x00FF,
        RS = 0x0100,
        RW = 0x0200,
        EN = 0x0400,
    };

    inline static constexpr uint16_t extract(uint16_t pins, MPUPinMask mask)
    {
        return extractBits(pins, static_cast<uint16_t>(mask));
    }

    static constexpr uint16_t inject(uint16_t pins, MPUPinMask mask, uint16_t data)
    {
        return injectBits(pins, static_cast<uint16_t>(mask), data);
    }

public:
    hd44780u();

    void setListener(listener* listener);

    uint16_t busyDelay() const { return busyDelay_; }
    void setBusyDelay(uint16_t cycles) { busyDelay_ = cycles; }

    uint8_t cursorPos() const;
    uint8_t displayShift() const { return shift_; }
    bool isBusy() const { return busy_ > 0; }
    bool isDisplayOn() const { return displayOn_; }
    bool isCursorOn() const { return cursorOn_; }

    uint8_t charAt(uint8_t address) const;
    ArrayView charMatrix(uint8_t address) const;

    uint16_t cycle(uint16_t pins); // cycle is not necessarily a CPU cycle

    bool wasReadInstruction() const { return wasReadInstruction_; }

private:
    void onBlinkTick();
    void cursorControl();

    void handleInstruction(uint8_t cmd);
    uint8_t currentState();

    void setData(uint8_t data);
    uint8_t data();

    void clearInstruction();
    void returnHomeInstruction();
    void entryModeSetInstruction(uint8_t cmd);
    void displayControlInstruction(uint8_t cmd);
    void cursorInstruction(uint8_t cmd);
    void funtionSetInstruction(uint8_t cmd);
    void setCGRAMAddrInstruction(uint8_t cmd);
    void setDDRAMAddrInstruction(uint8_t cmd);

    void updateCursorPos();
    void moveDisplay(bool increment);
    void moveCursor(bool increment);

    void notifyChangedCharacters(uint8_t cgRamAddr);

private:
    listener* listener_;
    QScopedPointer<QTimer> blinker_;
    uint16_t mpuPins_{};
    uint16_t busyDelay_{20};
    uint16_t busy_{0};
    std::array<uint8_t, 80> ddram_{};
    std::array<uint8_t, 64> cgram_{};
    uint8_t ramAddr_{0};
    uint8_t cursorPos_{0};
    uint8_t shift_{0};
    uint8_t ramSelector_{0};
    bool increment_{true};
    bool shiftEnabled_{false};
    bool displayOn_{false};
    bool cursorEnabled_{true};
    bool cursorBlink_{false};
    bool readDataValid_{false};
    bool cursorOn_{true};

    bool wasReadInstruction_{false};
};
