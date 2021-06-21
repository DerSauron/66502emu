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

#include "Device.h"
#include <QVector>

class ArrayView;

class Memory : public Device
{
    Q_OBJECT

public:
    enum class Type
    {
        ROM,
        RAM,
        FLASH,
    };

public:
    Memory(Type type, uint32_t size, const QString& name, Board* board);
    ~Memory() override;

    bool isWriteable() const { return type_ == Type::RAM || type_ == Type::FLASH; }
    bool isPersistant() const { return type_ == Type::ROM || type_ == Type::FLASH; }

    Type type() const { return type_; }
    uint32_t size() const { return static_cast<uint32_t>(data_.size()); }
    QVector<uint8_t>& data() { return data_; };
    void setData(uint32_t index, const ArrayView& data);

    uint8_t byte(uint16_t address) const { return data_[address]; }

signals:
    void byteAccessed(uint16_t address, bool write);

protected:
    void setup();
    uint16_t calcMapAddressEnd() const override;
    void deviceClockEdge(StateEdge edge) override;

private:
    Type type_;
    QVector<uint8_t> data_;
};
