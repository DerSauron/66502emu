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

#include "ClockView.h"
#include "ui_ClockView.h"

#include "board/Clock.h"
#include <QIntValidator>

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
    ui->period->setValidator(new QIntValidator(1, 5000, this));
    ui->period->setText(QStringLiteral("1"));
}

void ClockView::setClock(Clock* clock)
{
    Q_ASSERT(clock);

    if (clock == clock_)
        return;

    if (clock_)
    {
        disconnect(clock_, &Clock::runningChanged, this, &ClockView::onClockRunningChanged);
        disconnect(clock_, &Clock::clockEdge, this, &ClockView::onClockEdge);
    }

    clock_ = clock;

    connect(clock_, &Clock::runningChanged, this, &ClockView::onClockRunningChanged);
    connect(clock_, &Clock::clockEdge, this, &ClockView::onClockEdge);

    ui->period->setText(QString::number(clock_->period()));

    onClockRunningChanged();
    onClockEdge(StateEdge::Raising);
}

void ClockView::onClockRunningChanged()
{
    const bool running = clock_->isRunning();
    if (running)
    {
        ui->startStopButton->setText(tr("Stop"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->startStopButton->setIcon(icon);
    }
    else
    {
        ui->startStopButton->setText(tr("Start"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/start.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->startStopButton->setIcon(icon);

    }
    ui->period->setEnabled(!running);
    ui->singleStepButton->setEnabled(!running);
}

void ClockView::onClockEdge(StateEdge edge)
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

void ClockView::on_period_textChanged(const QString& text)
{
    if (clock_)
        clock_->setPeriod(ui->period->text().toInt());
}
