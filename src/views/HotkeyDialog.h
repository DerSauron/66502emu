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

namespace Ui {
class HotkeyDialog;
}

class HotkeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HotkeyDialog(int bit, int keyCode, QWidget *parent = nullptr);
    ~HotkeyDialog() override;

    int bit() const { return bit_; }
    int keyCode() const { return keyCode_; }

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void on_clearButton_clicked();

private:
    void setKeyCodeText();

private:
    Ui::HotkeyDialog *ui;
    int bit_;
    int keyCode_;

    Q_DISABLE_COPY_MOVE(HotkeyDialog)
};
