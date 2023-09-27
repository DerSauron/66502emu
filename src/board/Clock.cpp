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

#include "Clock.h"

#include <QTimer>
#include <QThread>
#include <QDebug>

Clock::Clock(QObject* parent) :
    QObject{parent},
    period_{0},
    busyWaiter_{},
    busyWaitTimeout_{0},
    timer_{new QTimer{this}},
    state_{true},
    shouldStop_{0},
    statsTimer_(new QTimer(this)),
    statsCycleCounter_{}
{
    connect(timer_, &QTimer::timeout, this, &Clock::tick);

    connect(statsTimer_, &QTimer::timeout, this, &Clock::collectStats);
    statsTimer_->start(1000);

    setPeriod(1000);
}

Clock::~Clock()
{
}

void Clock::setPeriod(int64_t period)
{
    if (period == period_)
        return;

    period_ = period;
    if (period_ < 2000)
    {
        busyWaitTimeout_ = period_ / 2;
        busyWaiter_.start();
        timer_->setInterval(0);
    }
    else
    {
        busyWaitTimeout_ = 0;
        timer_->setInterval(static_cast<int>(period_ / 2000));
    }
}

bool Clock::isRunning() const
{
    return timer_->isActive();
}

void Clock::start()
{
    if (QThread::currentThread() != thread())
    {
        QMetaObject::invokeMethod(this, "start");
        return;
    }

    if (!isRunning())
    {
        timer_->start();
        emit runningChanged();
    }

    shouldStop_ = 0;
}

void Clock::stop()
{
    shouldStop_ = 1;
}

void Clock::triggerEdge(StateEdge edge)
{
    if (isRunning())
        return;

    bool extraTick{};

    switch (edge)
    {
        case StateEdge::Raising:
            extraTick = isHigh(state_);
            break;
        case StateEdge::Falling:
            extraTick = isLow(state_);
            break;
        default:
            return;
    }

    if (extraTick)
        tick();

    tick();
}

void Clock::tick()
{
    if (isRunning() && busyWaitTimeout_ > 0)
    {
        while (busyWaiter_.nsecsElapsed() / 1000 <= busyWaitTimeout_);
        busyWaiter_.start();
    }

    state_ = isLow(state_) ? WireState::High : WireState::Low;

    if (isHigh(state_))
        statsCycleCounter_++;

    emit clockCycleChanged();

    if (shouldStop_ && isHigh(state_))
    {
        timer_->stop();
        emit runningChanged();
    }
}

void Clock::collectStats()
{
    emit statsUpdatedClockCycles(statsCycleCounter_);

    statsCycleCounter_ = 0;
}
