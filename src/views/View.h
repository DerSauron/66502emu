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

#include <QDialog>

class MainWindow;

class View : public QDialog
{
    Q_OBJECT

public:
    View(const QString& name, MainWindow* mainWindow);
    View(const QString& name, MainWindow* mainWindow, QWidget* parent);
    ~View() override;
    virtual void initialize();

    MainWindow* mainWindow() const { return mainWindow_; }
    const QString& name() const { return name_; }

signals:
    void closingEvent();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void loadWindowState();
    void saveWindowState();

private:
    MainWindow* mainWindow_;
    QString name_;
};
