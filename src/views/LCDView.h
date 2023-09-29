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

#include "board/LCD.h"
#include "DeviceView.h"

class LCD;

namespace Ui {
class LCDView;
}

class LCDView : public DeviceView
{
    Q_OBJECT

public:
    explicit LCDView(LCD* lcd, MainWindow* parent);
    ~LCDView() override;

private slots:
    void onLCDCharaterChanged(uint8_t address);
    void onLCDBusyChanged();
    void onLCDCursorPosChanged();
    void onLCDCursorChanged();
    void onLCDDisplayShiftChanged();
    void onLCDDisplayChanged();

private:
    QPoint panelPos(int32_t address) const;
    void setup();
    void redrawCharacters();

private:
    Ui::LCDView* ui;
    LCD* lcd_;
    uint8_t panelShift_;

    Q_DISABLE_COPY_MOVE(LCDView)
};
