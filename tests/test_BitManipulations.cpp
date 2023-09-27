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

#include "utils/Bits.h"

#include <QtTest>

class TestBitManipulations : public QObject
{
    Q_OBJECT

private slots:
    void extract8_data()
    {
        QTest::addColumn<uint8_t>("value");
        QTest::addColumn<uint8_t>("mask");
        QTest::addColumn<uint8_t>("result");

        QTest::newRow("0") << uint8_t{0} << uint8_t{0} << uint8_t{0};
        QTest::newRow("1") << uint8_t{0b0001} << uint8_t{0b0001} << uint8_t{0b0001};
        QTest::newRow("2") << uint8_t{0b0010} << uint8_t{0b0001} << uint8_t{0b0000};
        QTest::newRow("3") << uint8_t{0b0100} << uint8_t{0b0100} << uint8_t{0b0001};
        QTest::newRow("4") << uint8_t{0b1000} << uint8_t{0b0100} << uint8_t{0b0000};
        QTest::newRow("5") << uint8_t{0b1000} << uint8_t{0b1100} << uint8_t{0b0010};
        QTest::newRow("6") << uint8_t{0b1100} << uint8_t{0b0100} << uint8_t{0b0001};
    }

    void extract8()
    {
        QFETCH(uint8_t, value);
        QFETCH(uint8_t, mask);
        QFETCH(uint8_t, result);

        QCOMPARE(extractBits(value, mask), result);
    }
};

#include "test_BitManipulations.moc"
QTEST_MAIN(TestBitManipulations)
