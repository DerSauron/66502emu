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

#include "View.h"

#include "MainWindow.h"
#include "UserState.h"
#include "board/Board.h"
#include <QCloseEvent>

View::View(const QString& name, MainWindow* mainWindow) :
    View(name, mainWindow, mainWindow)
{
}

View::View(const QString& name, MainWindow* mainWindow, QWidget* parent) :
    QDialog(parent),
    mainWindow_(mainWindow),
    name_(name)
{
    setup();
}

View::~View()
{
    saveWindowState();
}

void View::closeEvent(QCloseEvent* event)
{
    event->ignore();
    emit closingEvent();
}

void View::setup()
{
    setWindowTitle(name_);
    loadWindowState();
}

void View::loadWindowState()
{
    mainWindow()->board()->userState()->loadViewState(this);
}

void View::saveWindowState()
{
    mainWindow()->board()->userState()->saveViewState(this);
}
