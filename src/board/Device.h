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
#include <QUuid>
#include <QVector>

class Board;
class Bus;
class BusConnection;

class Device : public QObject
{
    Q_OBJECT

public:
    Device(const QString& name, Board* board);
    ~Device() override;

    QString name() const { return objectName(); }
    Board* board() const { return board_; }

    void setMapAddressStart(uint16_t address) { mapAddressStart_ = address; }
    uint16_t mapAddressStart() const { return mapAddressStart_; }
    uint16_t mapAddressEnd() const { return calcMapAddressEnd(); }

    const QVector<BusConnection>& busConnections() const;
    QString portTagName(const BusConnection& busConnection) const;
    void addBusConnection(const QString& portTagName, uint64_t portMask, Bus* bus, uint64_t busMask);

    bool isSelected() const { return chipSelected_; };

    void clockEdge(StateEdge edge);

signals:
    void selectedChanged();

protected:
    virtual uint64_t mapPortTag(const QString& portTagName) const { return std::numeric_limits<uint64_t>::max(); }
    virtual QString mapPortTagName(uint64_t portTag) const { return {}; }
    virtual uint16_t calcMapAddressEnd() const { return mapAddressStart_ - 1; }
    virtual void deviceClockEdge(StateEdge edge) {}

protected:
    Board* board_;
    uint16_t mapAddressStart_;
    bool chipWasSelected_;
    bool chipSelected_;
    QVector<BusConnection> busConnections_;
};

