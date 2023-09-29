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

#include "board/Memory.h"
#include "DeviceView.h"
#include "Program.h"

namespace Ui {
class MemoryView;
}

class ProgramFileWatcher;
class SourcesView;

class MemoryView : public DeviceView
{
    Q_OBJECT

public:
    explicit MemoryView(Memory* memory, MainWindow* parent);
    ~MemoryView() override;

    void initialize() override;

private slots:
    void onMemoryAccessed();
    void onMemorySelectedChanged();
    void onSourcesViewClosingEvent();
    void onProgramFileChanged();
    void on_loadButton_clicked();
    void on_followButton_toggled(bool checked);
    void on_page_valueChanged(int value);
    void on_showSourcesButton_clicked();

private:
    void setup();
    void maybeLoadProgram();
    void maybeShowSources();
    void loadProgram(const QString& fileName);
    void rememberProgram();
    void rememberShowSources();
    void showSources();
    void hideSources();
    QString sourcesName();

private:
    Ui::MemoryView* ui;
    Memory* memory_;
    bool pageAutomaticallyChanged_;
    QString programFileName_;
    Program program_;
    SourcesView* sourcesView_;
    ProgramFileWatcher* fileSystemWatcher_;
    bool reloadInProgress_;

    Q_DISABLE_COPY_MOVE(MemoryView)
};
