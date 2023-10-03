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

#include "BusView.h"
#include "ui_BusView.h"

#include "LooseSignal.h"
#include "board/Bus.h"

BusView::BusView(QWidget* parent) :
    QWidget{parent},
    ui{new Ui::BusView{}},
    bus_{}
{
    ui->setupUi(this);
    setup();
}

BusView::~BusView()
{
    delete ui;
}

void BusView::setup()
{
}

void BusView::setBus(Bus* bus)
{
    Q_ASSERT(!bus_);
    Q_ASSERT(bus);

    bus_ = bus;

    LooseSignal::connect(bus_, &Bus::dataChanged, this, &BusView::onDataChanged);

    ui->bitsView->setBitCount(bus_->width());

    onDataChanged();
}
void BusView::onDataChanged()
{
    auto busData = bus_->data();
    ui->value->setText(QStringLiteral("%1").arg(busData, bus_->width() / 8 * 2, 16, QLatin1Char('0')));
    ui->bitsView->setValue(busData);
}
