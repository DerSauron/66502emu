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

#include "WireState.h"
#include <QObject>

class QTimer;

class Clock : public QObject
{
    Q_OBJECT

public:
    explicit Clock(QObject* parent = nullptr);
    ~Clock() override;

    int period() const;
    void setPeriod(int period);

    bool isRunning() const;

    WireState state() const { return state_; }

public slots:
    void start();
    void stop();
    void triggerEdge(StateEdge edge);

signals:
    void runningChanged();
    void clockEdge(StateEdge edge);

private slots:
    void tick();

private:
    QTimer* timer_;
    WireState state_;
    bool shouldStop_;
};

