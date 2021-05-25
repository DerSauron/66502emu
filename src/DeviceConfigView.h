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

#pragma once

#include <QWidget>

namespace Ui {
class DeviceConfigView;
}

class DeviceConfigView : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigView(QWidget *parent = nullptr);
    ~DeviceConfigView();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DeviceConfigView *ui;
};

