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

#include "DeviceViewCreator.h"
#include "board/Device.h"
#include "views/DeviceView.h"
#include "views/DisassemblerView.h"

class MainWindow;

class ViewFactory
{
public:
    virtual ~ViewFactory() = default;

    const QString& viewName() { return viewName_; }
    View* view() const { return view_; }

    View* createView(MainWindow* mainWindow)
    {
        Q_ASSERT(!view_);
        view_ = createViewImpl(mainWindow);
        return view_;
    }

    void destroyView(MainWindow* mainWindow)
    {
        Q_ASSERT(view_);
        delete view_;
        view_ = nullptr;
    }

protected:
    ViewFactory(QString viewName) : viewName_(viewName) {}

    virtual View* createViewImpl(MainWindow* mainWindow) = 0;

    QString viewName_;
    View* view_{};
};

using ViewFactoryPointer = QSharedPointer<ViewFactory>;
Q_DECLARE_METATYPE(ViewFactoryPointer)

class DeviceViewFactory : public ViewFactory
{
public:
    static ViewFactoryPointer create(Device* device)
    {
        return ViewFactoryPointer{new DeviceViewFactory(device)};
    }

    Device* device() const { return device_; }

protected:
    DeviceViewFactory(Device* device) :
        ViewFactory(device->name()),
        device_(device)
    {
    }

    View* createViewImpl(MainWindow* mainWindow) override
    {
        auto* view = DeviceViewCreator::createViewForDevice(device_, mainWindow);
        view->initialize();
        return view;
    }

    Device* device_{};
};

using DeviceViewFactoryPointer = QSharedPointer<DeviceViewFactory>;
Q_DECLARE_METATYPE(DeviceViewFactoryPointer)

class DisassemblerViewFactory : public ViewFactory
{
public:
    static ViewFactoryPointer create(const QString& name)
    {
        return ViewFactoryPointer{new DisassemblerViewFactory(name)};
    }

private:
    DisassemblerViewFactory(const QString& name) :
        ViewFactory(name)
    {
    }

    View* createViewImpl(MainWindow* mainWindow) override
    {
        auto* view = new DisassemblerView(viewName_, mainWindow);
        view->initialize();
        return view;
    }
};

using DisassemblerViewFactoryPointer = QSharedPointer<DisassemblerViewFactory>;
Q_DECLARE_METATYPE(DisassemblerViewFactoryPointer)
