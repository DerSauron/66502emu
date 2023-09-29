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

#include "VIAView.h"
#include "ui_VIAView.h"

#include "LooseSignal.h"
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
    ui{new Ui::VIAView{}}
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
    LooseSignal::connect(via(), &VIA::selectedChanged, this, &VIAView::onSelectedChanged);
    LooseSignal::connect(via(), &VIA::paChanged, this, &VIAView::onPaChanged);
    LooseSignal::connect(via(), &VIA::pbChanged, this, &VIAView::onPbChanged);
    LooseSignal::connect(via(), &VIA::t1Changed, this, &VIAView::onT1Changed);
    LooseSignal::connect(via(), &VIA::t2Changed, this, &VIAView::onT2Changed);
    LooseSignal::connect(via(), &VIA::ifrChanged, this, &VIAView::onIFRChanged);

    ui->paView->setName(QStringLiteral("PA"));
    ui->paView->setBitCount(8);
    ui->paView->setHotkeysEnabled(true);

    ui->pbView->setName(QStringLiteral("PB"));
    ui->pbView->setBitCount(8);
    ui->pbView->setHotkeysEnabled(true);

    ui->ierView->setName(QStringLiteral("IER"));
    ui->ierView->setBitCount(8);
    ui->ierView->setBitNames(makeBitNames("CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", ""));

    ui->ifrView->setName(QStringLiteral("IFR"));
    ui->ifrView->setBitCount(8);
    ui->ifrView->setBitNames(makeBitNames("CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", "IRQ"));

    ui->acrView->setName(QStringLiteral("ACR"));
    ui->acrView->setBitCount(8);
    ui->acrView->setBitNames(makeBitNames("PA", "PB", "SR0", "SR1", "SR2", "T2", "T10", "T11"));

    ui->pcrView->setName(QStringLiteral("PCR"));
    ui->pcrView->setBitCount(8);
    ui->pcrView->setBitNames(makeBitNames("CA1", "CA2", "CA2", "CA2", "CB1", "CB2", "CB2", "CB2"));

    connect(ui->paView, &RegisterView::valueChanged, this, &VIAView::onSetPa);
    connect(ui->pbView, &RegisterView::valueChanged, this, &VIAView::onSetPb);

    onPaChanged();
    onPbChanged();
}

void VIAView::onSelectedChanged()
{
    ui->chipSelected->setValue(via()->isSelected());
}

void VIAView::onPaChanged()
{
    ui->paView->setValue(via()->pa());
    ui->paView->setEditableMask(~via()->paDir() & 0xFF);
    ui->paView->setBitNames(ioStates(via()->paDir()));
}

void VIAView::onPbChanged()
{
    ui->pbView->setValue(via()->pb());
    ui->pbView->setEditableMask(~via()->pbDir() & 0xFF);
    ui->pbView->setBitNames(ioStates(via()->pbDir()));
}

void VIAView::onT1Changed()
{
    ui->timer1->setText(QStringLiteral("T1: %1").arg(via()->t1(), 4, 16, QLatin1Char('0')));
    ui->timer1Latch->setText(QStringLiteral("L: %1").arg(via()->t1l(), 4, 16, QLatin1Char('0')));
}

void VIAView::onT2Changed()
{
    ui->timer2->setText(QStringLiteral("T2: %1").arg(via()->t2(), 4, 16, QLatin1Char('0')));
    ui->timer2Latch->setText(QStringLiteral("L:   %1").arg(via()->t2l(), 2, 16, QLatin1Char('0')));
}

void VIAView::onIFRChanged()
{
    ui->ifrView->setValue(via()->ifr());
    ui->ierView->setValue(via()->ier());
}

void VIAView::onACRChanged()
{
    ui->acrView->setValue(via()->acr());
}

void VIAView::onPCRChanged()
{
    ui->pcrView->setValue(via()->pcr());
}

void VIAView::onSetPa()
{
    via()->setPa(static_cast<uint8_t>(ui->paView->value()));
}

void VIAView::onSetPb()
{
    via()->setPb(static_cast<uint8_t>(ui->pbView->value()));
}
