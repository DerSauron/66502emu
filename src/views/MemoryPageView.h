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

class Memory;

class MemoryPageView : public QWidget
{
    Q_OBJECT
public:
    MemoryPageView(QWidget *parent = nullptr);
    ~MemoryPageView() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    Memory*  memory() { return memory_; }
    void setMemory(Memory* memory);

    uint16_t addressOffset() const { return addressOffset_; }
    void setAddressOffset(uint16_t addressOffset);

    uint8_t page() const { return page_; }
    void setPage(uint8_t page);

    void highlight(uint8_t byte, bool write);
    void resetHighlight();

signals:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QFont font_;
    Memory* memory_;
    uint8_t page_;
    uint16_t addressOffset_;
    int charWidth_;
    int charHeight_;
    int charAscent_;
    uint16_t highLightByte_;
    bool highlightWrite_;
};

