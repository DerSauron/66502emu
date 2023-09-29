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
    Memory(Type type, int32_t size, const QString& name, Board* board);
    ~Memory() override;

    bool isWriteable() const { return type_ == Type::RAM || type_ == Type::FLASH; }
    bool isPersistant() const { return type_ == Type::ROM || type_ == Type::FLASH; }

    int32_t lastAccessAddress() const { return lastAccessAddress_; }
    bool lastAccessWasWrite() const { return lastAccessWasWrite_; }

    Type type() const { return type_; }
    int32_t size() const { return data_.size(); }
    QVector<uint8_t>& data() { return data_; };
    void setData(int32_t index, const ArrayView& data);

    uint8_t byte(int32_t address) const { return data_[address]; }

signals:
    void accessed();

protected:
    void setup();
    int32_t calcMapAddressEnd() const override;
    void deviceClockEdge(StateEdge edge) override;

private:
    Type type_;
    QVector<uint8_t> data_;
    int32_t lastAccessAddress_;
    bool lastAccessWasWrite_;

    Q_DISABLE_COPY_MOVE(Memory)
};
