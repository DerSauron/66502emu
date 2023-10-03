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

#include "DeviceView.h"
#include "board/ACIA.h"

namespace Ui {
class ACIAView;
}

class ACIAView : public DeviceView
{
    Q_OBJECT

public:
    ACIAView(ACIA* acia, MainWindow* parent);
    ~ACIAView() override;

private:
    void setup();

private slots:
    void onChipSelectedChanged();
    void onDataEntered(const QByteArray& inputData);
    void onSendByte(uint8_t byte);
    void onTransmittingChanged();
    void onReceivingChanged();
    void onRegisterChanged();

private:
    Ui::ACIAView* ui;
    ACIA* acia_;

    Q_DISABLE_COPY_MOVE(ACIAView)
};
