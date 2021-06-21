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

#include "Device.h"
#include "impl/hd44780u.h"

class Board;
class Bus;
class QTimer;

class LCD : public Device, public hd44780u::listener
{
    Q_OBJECT

public:
    LCD(const QString& name, Board* board);
    ~LCD() override;

    ArrayView charMatrix(uint8_t address) const;

    void onCharacterChanged(uint8_t address) override;
    void onBusyChanged() override;
    void onCursorPosChanged() override;
    void onCursorChanged() override;
    void onShiftPosChanged() override;
    void onDisplayChanged() override;

    uint8_t bufferWidth() const;

    bool isBusy() const;
    uint16_t cursorPos() const;
    uint8_t displayShift() const;
    bool isDisplayOn() const;
    bool isCursorOn() const;

signals:
    void characterChanged(uint8_t address);
    void busyChanged();
    void cursorPosChanged();
    void cursorChanged();
    void displayShiftChanged();
    void displayChanged();

protected:
    uint64_t mapPortTag(const QString& portTagName) const override;
    // update the hd44780u with the system clock to keep state when stepping
    void deviceClockEdge(StateEdge edge) override;

private:
    void setup();
    void injectState();
    void populateState();

private:
    QScopedPointer<hd44780u> chip_;
    uint16_t pins_;
    uint16_t cursorPos_;
    bool cursorOn_;
};
