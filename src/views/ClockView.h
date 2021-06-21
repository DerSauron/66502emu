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

#include "board/WireState.h"
#include <QWidget>

class Clock;

namespace Ui {
class ClockView;
}

class ClockView : public QWidget
{
    Q_OBJECT

public:
    explicit ClockView(QWidget* parent = nullptr);
    ~ClockView() override;

    void setClock(Clock* clock);

private slots:
    void onClockRunningChanged();
    void onClockCycleChanged();
    void on_startStopButton_clicked();
    void on_singleStepButton_pressed();
    void on_singleStepButton_released();
    void on_frequency_currentIndexChanged(int index);

private:
    void setup();

private:
    Ui::ClockView* ui;
    Clock* clock_;
};
