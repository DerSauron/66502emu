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

#include "VIAView.h"
#include "ui_VIAView.h"

#include "impl/m6522.h"

namespace {

QStringList ioStates(uint64_t dir)
{
    QStringList bitNames;
    for (int i = 0; i < 8; i++)
    {
        bitNames += (dir & 1) ? QStringLiteral("O") : QStringLiteral("I");
        dir >>= 1;
    }
    return bitNames;
}

#if 0
QStringList ierStates(uint64_t ier)
{
    QStringList bitNames;
    for (int i = 0; i < 8; i++)
    {
        bitNames += (ier & 1) ? QStringLiteral("E") : QStringLiteral();
        ier >>= 1;
    }
    return bitNames;
}
#endif

} // namespace

VIAView::VIAView(VIA* via, MainWindow* parent) :
    DeviceView{via, parent},
    ui(new Ui::VIAView)
{
    ui->setupUi(this);
    setup();
}

VIAView::~VIAView()
{
    delete ui;
}

void VIAView::setup()
{
    ui->chipSelected->setBitCount(1);
    connect(via(), &VIA::selectedChanged, this, &VIAView::onVIASelectedChanged);
    connect(via(), &VIA::paChanged, this, &VIAView::onPaChanged);
    connect(via(), &VIA::pbChanged, this, &VIAView::onPbChanged);
    connect(via(), &VIA::t1Changed, this, &VIAView::onT1Changed);
    connect(via(), &VIA::t2Changed, this, &VIAView::onT2Changed);
    connect(via(), &VIA::ifrChanged, this, &VIAView::onIFRChanged);
    connect(via(), &VIA::registerChanged, this, &VIAView::onRegisterChanged);

    ui->paView->setName(QStringLiteral("IOA"));
    ui->paView->setBitCount(8);
    ui->pbView->setName(QStringLiteral("IOB"));
    ui->pbView->setBitCount(8);
    ui->ierView->setName(QStringLiteral("IER"));
    ui->ierView->setBitCount(8);
    ui->ierView->setBitNames(makeBitNames("CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", ""));
    ui->ifrView->setName(QStringLiteral("IFR"));
    ui->ifrView->setBitCount(8);
    ui->ifrView->setBitNames(makeBitNames("CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", "IRQ"));

    connect(ui->paView, &RegisterView::valueChanged, this, &VIAView::onSetPa);
    connect(ui->pbView, &RegisterView::valueChanged, this, &VIAView::onSetPb);

    onPaChanged();
    onRegisterChanged(M6522_REG_DDRA);
    onPbChanged();
    onRegisterChanged(M6522_REG_DDRB);
}

void VIAView::onVIASelectedChanged()
{
    ui->chipSelected->setValue(via()->isSelected());
}

void VIAView::onPaChanged()
{
    ui->paView->setValue(via()->pa());
}

void VIAView::onPbChanged()
{
    ui->pbView->setValue(via()->pb());
}

void VIAView::onT1Changed()
{
    ui->timer1->setText(QStringLiteral("T1: %1").arg(via()->t1(), 4, 16, QLatin1Char('0')));
}

void VIAView::onT2Changed()
{
    ui->timer2->setText(QStringLiteral("T2: %1").arg(via()->t2(), 4, 16, QLatin1Char('0')));
}

void VIAView::onIFRChanged()
{
    ui->ifrView->setValue(via()->ifr());
}

void VIAView::onRegisterChanged(uint8_t reg)
{
    switch (reg)
    {
        case M6522_REG_RB:
        case M6522_REG_RA:
            break;

        case M6522_REG_DDRB:
            ui->pbView->setEditableMask(~via()->pbDir() & 0xFF);
            ui->pbView->setBitNames(ioStates(via()->pbDir()));
            break;

        case M6522_REG_DDRA:
            ui->paView->setEditableMask(~via()->paDir() & 0xFF);
            ui->paView->setBitNames(ioStates(via()->paDir()));
            break;

        case M6522_REG_T1CL:
        case M6522_REG_T1CH:
            break;

        case M6522_REG_T1LL:
        case M6522_REG_T1LH:
            ui->timer1Latch->setText(QStringLiteral("L: %1").arg(via()->t1l(), 4, 16, QLatin1Char('0')));
            break;

        case M6522_REG_T2CL:
            ui->timer2Latch->setText(QStringLiteral("L:   %1").arg(via()->t2l(), 2, 16, QLatin1Char('0')));
            break;

        case M6522_REG_T2CH:
        case M6522_REG_SR:
        case M6522_REG_ACR:
        case M6522_REG_PCR:
        case M6522_REG_IFR:
            break;

        case M6522_REG_IER:
            ui->ierView->setValue(via()->ier());
            break;

        case M6522_REG_RA_NOH:
        case M6522_NUM_REGS:
            break;
    }
}

void VIAView::onSetPa()
{
    via()->setPa(ui->paView->value());
}

void VIAView::onSetPb()
{
    via()->setPb(ui->pbView->value());
}
