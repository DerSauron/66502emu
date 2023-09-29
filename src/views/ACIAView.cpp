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

#include "ACIAView.h"
#include "ui_ACIAView.h"

#include "LooseSignal.h"

ACIAView::ACIAView(ACIA* acia, MainWindow* parent) :
    DeviceView{acia, parent},
    ui{new Ui::ACIAView{}},
    acia_{acia}
{
    ui->setupUi(this);
    setup();
}

ACIAView::~ACIAView()
{
    delete ui;
}

void ACIAView::setup()
{
    ui->chipSelect->setBitCount(1);
    ui->txFlag->setBitCount(1);
    ui->txFlag->setEnableColor(BitsView::EnabledColor::Red);
    ui->rxFlag->setBitCount(1);
    ui->rxFlag->setEnableColor(BitsView::EnabledColor::Red);


    ui->statusRegister->setName(QLatin1String("STAT"));
    ui->statusRegister->setBitCount(8);
    ui->statusRegister->setBitNames({QStringLiteral("PE"), QStringLiteral("FE"), QStringLiteral("OVR"),
                                     QStringLiteral("RDF"), QStringLiteral("TRE"), QStringLiteral("DCD"),
                                     QStringLiteral("DSR"), QStringLiteral("IRQ")});

    ui->commandRegister->setName(QLatin1String("CMD"));
    ui->commandRegister->setBitCount(8);
    ui->commandRegister->setBitNames({QStringLiteral("DTR"), QStringLiteral("IRD"), QStringLiteral("TI0"),
                                      QStringLiteral("TI1"), QStringLiteral("REM"), QStringLiteral("PME"),
                                      QStringLiteral("PM0"), QStringLiteral("PM1")});

    ui->controlRegister->setName(QLatin1String("CTRL"));
    ui->controlRegister->setBitCount(8);
    ui->controlRegister->setBitNames({QStringLiteral("BD0"), QStringLiteral("BD1"), QStringLiteral("BD2"),
                                      QStringLiteral("BD3"), QStringLiteral("RCS"), QStringLiteral("WL0"),
                                      QStringLiteral("WL1"), QStringLiteral("SBN")});

    connect(acia_, &ACIA::sendByte, this, &ACIAView::onSendByte); // run synchronously
    LooseSignal::connect(acia_, &ACIA::selectedChanged, this, &ACIAView::onChipSelectedChanged);
    LooseSignal::connect(acia_, &ACIA::transmittingChanged, this, &ACIAView::onTransmittingChanged);
    LooseSignal::connect(acia_, &ACIA::receivingChanged, this, &ACIAView::onReceivingChanged);
    LooseSignal::connect(acia_, &ACIA::registerChanged, this, &ACIAView::onRegisterChanged);

    connect(ui->console, &Console::inputData, this, &ACIAView::onDataEntered);

    onRegisterChanged();
}

void ACIAView::onChipSelectedChanged()
{
    ui->chipSelect->setValue(acia_->isSelected() ? 1 : 0);
}

void ACIAView::onDataEntered(const QByteArray& data)
{
    for (const auto& byte : data)
    {
        QMetaObject::invokeMethod(acia_, "receiveByte", Q_ARG(uint8_t, static_cast<uint8_t>(byte)));
    }
}

void ACIAView::onSendByte(uint8_t byte)
{
    ui->console->outputData(QByteArray(1, static_cast<char>(byte)));
}

void ACIAView::onTransmittingChanged()
{
    ui->txFlag->setValue(acia_->isTransmitting() ? 1 : 0);
    ui->txData->setText(QStringLiteral("%1").arg(acia_->transmitterBuffer(), 2, 16, QLatin1Char('0')));
}

void ACIAView::onReceivingChanged()
{
    ui->rxFlag->setValue(acia_->isReceiving() ? 1 : 0);
    ui->rxData->setText(QStringLiteral("%1").arg(acia_->receiverBuffer(), 2, 16, QLatin1Char('0')));
}

void ACIAView::onRegisterChanged()
{
    ui->statusRegister->setValue(acia_->statusRegister());
    ui->commandRegister->setValue(acia_->commandRegister());
    ui->controlRegister->setValue(acia_->controlRegister());
}
