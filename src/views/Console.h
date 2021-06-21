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

#include <QPlainTextEdit>

class Console : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console(QWidget* parent = nullptr);
    ~Console() override;

public:
    void setScrollBackBuffer(int scrollBackBuffer);
    void setLocalEcho(bool localEcho);

public slots:
    void outputData(const QByteArray& data);

signals:
    void inputData(const QByteArray& data);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setup();

private:
    bool localEcho_{false};
};
