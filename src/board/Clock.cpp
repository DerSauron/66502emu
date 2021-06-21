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

Clock::Clock(QObject* parent) :
    QObject{parent},
    timer_{new QTimer{this}},
    state_{true},
    shouldStop_{false}
    statsTimer_(new QTimer(this)),
    statsCycleCounter_{}
{
    connect(timer_, &QTimer::timeout, this, &Clock::tick);

    connect(statsTimer_, &QTimer::timeout, this, &Clock::collectStats);
    statsTimer_->start(1000);
}

Clock::~Clock()
{
}

int Clock::period() const
{
    return timer_->interval() * 2;
}

void Clock::setPeriod(int period)
{
    timer_->setInterval(period / 2 + period % 2);
}

bool Clock::isRunning() const
{
    return timer_->isActive();
}

void Clock::start()
{
    if (!isRunning())
    {
        timer_->start();
        emit runningChanged();
    }

    shouldStop_ = false;
}

void Clock::stop()
{
    shouldStop_ = true;
}

void Clock::triggerEdge(StateEdge edge)
{
    if (isRunning())
        return;

    bool extraTick;

    switch (edge)
    {
        case StateEdge::Raising:
            extraTick = isHigh(state_);
            break;
        case StateEdge::Falling:
            extraTick = isLow(state_);
            break;
    }

    if (extraTick)
        tick();

    tick();
}

void Clock::tick()
{
    state_ = isLow(state_) ? WireState::High : WireState::Low;

    if (isHigh(state_))
        statsCycleCounter_++;

    emit clockEdge(isHigh(state_) ? StateEdge::Raising : StateEdge::Falling);

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
