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

#include "board/Memory.h"
#include "DeviceView.h"

namespace Ui {
class MemoryView;
}

class MemoryView : public DeviceView
{
    Q_OBJECT

public:
    explicit MemoryView(Memory* memory, MainWindow* parent);
    ~MemoryView() override;

    Memory* memory() const { return static_cast<Memory*>(device()); }

private slots:
    void onMemoryAccessed(uint16_t address, bool write);
    void onMemorySelectedChanged();
    void on_loadButton_clicked();
    void on_followButton_toggled(bool checked);
    void on_page_valueChanged(int value);

private:
    void setup();
    void loadFileToMemory(const QString& fileName);

private:
    Ui::MemoryView* ui;
    bool pageAutomaticallyChanged_;
};
