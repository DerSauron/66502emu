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

#include "ClockView.h"
#include "ui_ClockView.h"

#include "LooseSignal.h"
#include "board/Clock.h"
#include <QIntValidator>

namespace {

const QVector<int32_t> Periods{ // clazy:exclude=non-pod-global-static}
    4000000,
    2000000,
    1000000,
    500000,
    200000,
    100000,
    10000,
    1000,
    100,
    0,
};

int findPeriodIndex(int32_t period)
{
    for (int i = 0; const auto& p : Periods)
    {
        if (p <= period)
            return i;
        ++i;
    }
    return 2;
}

} // namespace

ClockView::ClockView(QWidget* parent) :
    QWidget{parent},
    ui{new Ui::ClockView},
    clock_{}
{
    ui->setupUi(this);
    setup();
}

ClockView::~ClockView()
{
    delete ui;
}

void ClockView::setup()
{
    ui->clockState->setBitCount(1);
}

void ClockView::setClock(Clock* clock)
{
    Q_ASSERT(!clock_);
    Q_ASSERT(clock);

    clock_ = clock;

    LooseSignal::connect(clock_, &Clock::runningChanged, this, &ClockView::onClockRunningChanged);
    LooseSignal::connect(clock_, &Clock::clockCycleChanged, this, &ClockView::onClockCycleChanged);

    ui->frequency->setCurrentIndex(findPeriodIndex(clock_->period()));

    onClockRunningChanged();
    onClockCycleChanged();
}

void ClockView::onClockRunningChanged()
{
    const bool running = clock_->isRunning();
    if (running)
        ui->startStopButton->showStopMode();
    else
        ui->startStopButton->showStartMode();
    ui->singleStepButton->setEnabled(!running);
}

void ClockView::onClockCycleChanged()
{
    ui->clockState->setValue(toInt(clock_->state()));
}

void ClockView::on_startStopButton_clicked()
{
    if (clock_->isRunning())
        clock_->stop();
    else
        clock_->start();
}

void ClockView::on_singleStepButton_pressed()
{
    clock_->triggerEdge(StateEdge::Falling);
}

void ClockView::on_singleStepButton_released()
{
    clock_->triggerEdge(StateEdge::Raising);
}

void ClockView::on_frequency_currentIndexChanged(int index)
{
    if (!clock_)
        return;

    QMetaObject::invokeMethod(clock_, "setPeriod", Q_ARG(int32_t, Periods[ui->frequency->currentIndex()]));
}

