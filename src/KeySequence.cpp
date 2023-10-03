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

#include "KeySequence.h"

#include <QKeyEvent>
#include <QString>

int KeySequence::toKeyCode(QKeyEvent* event)
{
    int keyCode = event->key();
    Qt::Key key = static_cast<Qt::Key>(keyCode);

    if (key == Qt::Key_unknown)
        return Qt::Key_unknown;

    if (key == Qt::Key_unknown ||
        key == Qt::Key_Control ||  key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta)
        return keyCode;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (modifiers & Qt::ShiftModifier)
        keyCode += static_cast<int>(Qt::SHIFT);
    if (modifiers & Qt::ControlModifier)
        keyCode += static_cast<int>(Qt::CTRL);
    if (modifiers & Qt::AltModifier)
        keyCode += static_cast<int>(Qt::ALT);
    if (modifiers & Qt::MetaModifier)
        keyCode += static_cast<int>(Qt::META);

    return keyCode;
}

QString KeySequence::toString(int keyCode)
{
    return QKeySequence(keyCode).toString(QKeySequence::NativeText);
}
