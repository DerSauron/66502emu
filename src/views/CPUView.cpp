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

#include "CPUView.h"
#include "ui_CPUView.h"

#include "board/CPU.h"

CPUView::CPUView(QWidget* parent) :
    QWidget{parent},
    ui{new Ui::CPUView},
    cpu_{}
{
    ui->setupUi(this);
    setup();
}

CPUView::~CPUView()
{
    delete ui;
}

void CPUView::setup()
{
    ui->registerA->setName(QStringLiteral("A"));
    ui->registerA->setBitCount(8);

    ui->registerX->setName(QStringLiteral("X"));
    ui->registerX->setBitCount(8);

    ui->registerY->setName(QStringLiteral("Y"));
    ui->registerY->setBitCount(8);

    ui->registerS->setName(QStringLiteral("S"));
    ui->registerS->setBitCount(8);

    ui->flags->setName("Flags");
    ui->flags->setBitCount(8);
    ui->flags->setBitNames({QStringLiteral("C"), QStringLiteral("Z"), QStringLiteral("I"), QStringLiteral("D"),
                           QStringLiteral("B"), QStringLiteral("-"), QStringLiteral("V"), QStringLiteral("N")});
}

void CPUView::setCPU(CPU* cpu)
{
    if (cpu == cpu_)
        return;

    if (cpu_)
    {
        disconnect(cpu_, &CPU::stepped, this, &CPUView::onCPUStepped);
    }

    cpu_ = cpu;

    connect(cpu_, &CPU::stepped, this, &CPUView::onCPUStepped);

    onCPUStepped();
}

void CPUView::onCPUStepped()
{
    populateRegisters();
}

void CPUView::populateRegisters()
{
    ui->registerA->setValue(cpu_->registerA());
    ui->registerX->setValue(cpu_->registerX());
    ui->registerY->setValue(cpu_->registerY());
    ui->registerS->setValue(cpu_->registerS());
    ui->registerPC->setText(QStringLiteral("PC: %1").arg(cpu_->registerPC(), 4, 16, QLatin1Char('0')));
    ui->registerIR->setText(QStringLiteral("IR: %1/%2")
                            .arg(cpu_->registerIR() >> 3, 2, 16, QLatin1Char('0'))
                            .arg(cpu_->registerIR() & 0x7, 1, 16, QLatin1Char('0')));
    ui->flags->setValue(cpu_->flags());
}
