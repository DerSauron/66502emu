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

#include "DeviceViewCreator.h"

#include "MainWindow.h"
#include "board/LCD.h"
#include "board/Memory.h"
#include "board/VIA.h"
#include "views/LCDView.h"
#include "views/MemoryView.h"
#include "views/VIAView.h"
#include <QDebug>

DeviceView* DeviceViewCreator::createViewForDevice(Device* device, MainWindow* mainWindow)
{
    if (LCD* lcd = qobject_cast<LCD*>(device))
    {
        return new LCDView(lcd, mainWindow);
    }
    else if (Memory* mem = qobject_cast<Memory*>(device))
    {
        return new MemoryView(mem, mainWindow);
    }
    else if (VIA* via = qobject_cast<VIA*>(device))
    {
        return new VIAView(via, mainWindow);
    }

    qWarning() << "Cannot create view for device" << device->name();
    return nullptr;
}

void DeviceViewCreator::destroyViewForDevice(Device* device, MainWindow* mainWindow)
{
    auto views = mainWindow->findChildren<DeviceView*>();
    for (const auto view : views)
    {
        if (view->name() == device->name())
            destroyView(view);
    }
}

void DeviceViewCreator::destroyView(DeviceView* view)
{

}
