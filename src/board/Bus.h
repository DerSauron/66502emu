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

#include "WireState.h"
#include <QObject>

class Board;

class Bus : public QObject
{
    Q_OBJECT

public:
    Bus(const QString& name, uint8_t width, Board* board);

    QString name() const { return objectName(); }
    uint8_t width() const { return width_; }

    WireState bit(uint8_t bit) const;
    void setBit(uint8_t bit, WireState state);

    template<typename T>
    T typedData() { return static_cast<T>(data_); }

    uint64_t data() const { return data_; }
    void setData(uint64_t data);

    uint64_t maskedData(uint64_t mask) const { return data_ & mask; }
    void setMaskedData(uint64_t data, uint64_t mask);

signals:
    void dataChanged();

private:
    uint8_t width_;
    uint64_t data_;
};
