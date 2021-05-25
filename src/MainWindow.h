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

#include "views/ViewFactory.h"
#include <QMainWindow>

class Board;
class Device;
class View;
class UserState;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Board* board, QWidget* parent = nullptr);
    ~MainWindow() override;

    Board* board() const { return board_; }

    void closeView(View* view);
    void closeAllViews();

private slots:
    void onClockRunningChanged();
    void onBoardDevicesChanged();
    void onBoardViewAction();
    void on_actionManageDevices_triggered();
    void on_actionQuit_triggered();
    void on_actionNewBoard_triggered();
    void on_actionOpenBoard_triggered();

private:
    void setup();
    void maybeLoadBoard();
    void loadWindowState();
    void saveWindowState();
    void createBoardMenu();
    QAction* createDeviceViewAction(int index, Device* device, const ViewFactoryPointer& factory);
    void rebuildBoardMenuDevices();
    bool conditionallyShowView(const ViewFactoryPointer& factory, bool defaultShow);
    void showView(const ViewFactoryPointer& factory);
    void loadBoard(const QString& fileName);
    void saveBoard(const QString& fileName);
    void handleBoardLoaded();

private:
    Ui::MainWindow* ui;
    Board* board_;
    QString loadedFile_;
};
