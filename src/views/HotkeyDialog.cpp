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

#include "HotkeyDialog.h"
#include "ui_HotkeyDialog.h"

#include <KeySequence.h>
#include <QKeyEvent>

HotkeyDialog::HotkeyDialog(int bit, int keyCode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HotkeyDialog),
    bit_{bit},
    keyCode_{keyCode}
{
    ui->setupUi(this);
    ui->keyGrabber->installEventFilter(this);
    setKeyCodeText();
}

HotkeyDialog::~HotkeyDialog()
{
    delete ui;
}

bool HotkeyDialog::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        keyCode_ = KeySequence::toKeyCode(keyEvent);
        setKeyCodeText();

        return true;
    }

    return QDialog::eventFilter(object, event);
}

void HotkeyDialog::on_clearButton_clicked()
{
    keyCode_ = Qt::Key_unknown;
    setKeyCodeText();
}

void HotkeyDialog::setKeyCodeText()
{
    if (keyCode_ == Qt::Key_unknown)
        ui->keyGrabber->setText(QString{});
    else
        ui->keyGrabber->setText(KeySequence::toString(keyCode_));
}
