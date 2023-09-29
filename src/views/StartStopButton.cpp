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

#include "StartStopButton.h"

StartStopButton::StartStopButton(QWidget* parent) :
    QPushButton{parent}
{
}

StartStopButton::~StartStopButton()
{
}

void StartStopButton::showStartMode()
{
    setText(tr("Start"));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/start.png"), QSize(), QIcon::Normal, QIcon::Off);
    setIcon(icon);
}

void StartStopButton::showStopMode()
{
    setText(tr("Stop"));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
    setIcon(icon);
}
