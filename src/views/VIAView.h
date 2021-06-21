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

#include "board/VIA.h"
#include "DeviceView.h"

namespace Ui {
class VIAView;
}

class VIAView : public DeviceView
{
    Q_OBJECT

public:
    VIAView(VIA* via, MainWindow* parent);
    ~VIAView();

    VIA* via() const { return static_cast<VIA*>(device()); }

private slots:
    void onVIASelectedChanged();
    void onPaChanged();
    void onPbChanged();
    void onT1Changed();
    void onT2Changed();
    void onIFRChanged();
    void onRegisterChanged(uint8_t reg);
    void onSetPa();
    void onSetPb();

private:
    void setup();

private:
    Ui::VIAView *ui;
};
