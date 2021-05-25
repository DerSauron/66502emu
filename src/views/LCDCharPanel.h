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

#include <QWidget>

class ArrayView;

class LCDCharPanel : public QWidget
{
    Q_OBJECT

public:
    static int width();
    static int height();
    static bool outOfView(const QPoint& panelPos);

public:
    explicit LCDCharPanel(QWidget *parent = nullptr);
    ~LCDCharPanel() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setCharacterData(const QPoint& pos, const ArrayView& data);
    void setCursorPos(const QPoint& cursorPos);
    void setCursorOn(bool cursorOn);
    void setDisplayOn(bool displayOn);

    const QBrush& backgroundBrush() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateCharacterData(const QPoint& pos, const ArrayView& data);

    void clearBackground(QPainter& p, int startX, int startY, int endX, int endY);
    void renderCharMatrix(QPainter& p, int charX, int charY);
    void renderCharRow(QPainter& p, int charX, int charY, int row, uint8_t rowValue);

private:
    QVector<uint8_t> data_;
    QPoint cursorPos_;
};
