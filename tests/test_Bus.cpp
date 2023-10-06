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

#include "board/Bus.h"
#include <QtTest>

class TestBus : public QObject
{
    Q_OBJECT

private:
    Bus* bus;

private slots:
    void init()
    {
        bus = new Bus(QStringLiteral("TEST"), 8, nullptr);
    }

    void cleanup()
    {
        delete bus;
    }

    void new_created_empty()
    {
        QCOMPARE(bus->data(), 0);
    }

    void set_fifths_bit()
    {
        bus->setBit(5, WireState::High);
        QCOMPARE(bus->bit(5), WireState::High);
        QCOMPARE(bus->bit(4), WireState::Low);
        QCOMPARE(bus->data(), 0b00100000);
        bus->setBit(5, WireState::Low);
        QCOMPARE(bus->bit(5), WireState::Low);
        QCOMPARE(bus->bit(4), WireState::Low);
        QCOMPARE(bus->data(), 0b00000000);
    }

    void set_masked()
    {
        bus->setMaskedData(0b11111111, 0b01100000);
        QCOMPARE(bus->bit(7), WireState::Low);
        QCOMPARE(bus->bit(6), WireState::High);
        QCOMPARE(bus->bit(5), WireState::High);
        QCOMPARE(bus->bit(4), WireState::Low);
        QCOMPARE(bus->data(), 0b01100000);
    }
};

#include "test_Bus.moc"
QTEST_MAIN(TestBus)
