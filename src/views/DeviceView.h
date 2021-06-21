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

#include "View.h"

class Device;
class MainWindow;

class DeviceView : public View
{
    Q_OBJECT

public:
    DeviceView(Device* device, MainWindow* parent);
    ~DeviceView() override;

    Device* device() const { return device_; };

private:
    Device* device_;
};
