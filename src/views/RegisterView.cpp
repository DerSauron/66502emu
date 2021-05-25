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

#include "RegisterView.h"
#include "ui_RegisterView.h"

RegisterView::RegisterView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterView),
    bitCount_{0}
{
    ui->setupUi(this);
    connect(ui->bitsView, &BitsView::valueChanged, this, &RegisterView::valueChanged);
}

RegisterView::~RegisterView()
{
    delete ui;
}

void RegisterView::setName(const QString& name)
{
    ui->name->setText(name);
}

void RegisterView::setBitCount(uint8_t bitCount)
{
    bitCount_ = bitCount;
    ui->bitsView->setBitCount(bitCount);
}

void RegisterView::setBitNames(const QStringList& names)
{
    ui->bitsView->setBitNames(names);
}

void RegisterView::setEditableMask(uint64_t mask)
{
    ui->bitsView->setEditableMask(mask);
}


uint64_t RegisterView::value() const
{
    return ui->bitsView->value();
}

void RegisterView::setValue(uint64_t value)
{
    ui->bitsView->setValue(value);
    ui->value->setText(QStringLiteral("%1").arg(value, bitCount_ / 8 * 2, 16, QLatin1Char('0')));
}
