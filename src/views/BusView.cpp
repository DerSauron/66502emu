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

#include "BusView.h"
#include "ui_BusView.h"

#include "board/Bus.h"

BusView::BusView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BusView),
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
    Q_ASSERT(bus);

    if (bus == bus_)
        return;

    if (bus_)
        disconnect(bus_, &Bus::dataChanged, this, &BusView::onDataChanged);

    bus_ = bus;

    connect(bus_, &Bus::dataChanged, this, &BusView::onDataChanged);

    ui->bitsView->setBitCount(bus_->width());

    onDataChanged();
}

void BusView::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void BusView::onDataChanged()
{
    auto data = bus_->data();
    ui->value->setText(QStringLiteral("%1").arg(data, bus_->width() / 8 * 2, 16, QLatin1Char('0')));
    ui->bitsView->setValue(data);
}
