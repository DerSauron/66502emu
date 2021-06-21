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

#include "LCDView.h"
#include "ui_LCDView.h"

#include "LooseSignal.h"
#include "utils/ArrayView.h"
#include <QTimer>

LCDView::LCDView(LCD* lcd, MainWindow* parent) :
    DeviceView(lcd, parent),
    ui(new Ui::LCDView),
    panelShift_{0}
{
    ui->setupUi(this);
    setup();
}

LCDView::~LCDView()
{
    delete ui;
}

void LCDView::setup()
{
    ui->busyFlag->setBitCount(1);
    ui->busyFlag->setEnableColor(BitsView::EnabledColor::Red);

    connect(lcd(), &LCD::characterChanged, this, &LCDView::onLCDCharaterChanged); // this must run in sync
    LooseSignal::connect(lcd(), &LCD::busyChanged, this, &LCDView::onLCDBusyChanged);
    LooseSignal::connect(lcd(), &LCD::cursorPosChanged, this, &LCDView::onLCDCursorPosChanged);
    LooseSignal::connect(lcd(), &LCD::cursorChanged, this, &LCDView::onLCDCursorChanged);
    LooseSignal::connect(lcd(), &LCD::displayShiftChanged, this, &LCDView::onLCDDisplayShiftChanged);
    LooseSignal::connect(lcd(), &LCD::displayChanged, this, &LCDView::onLCDDisplayChanged);
}

void LCDView::redrawCharacters()
{
    for (int y = 0; y < LCDCharPanel::height(); y++)
    {
        for (int x = 0; x < LCDCharPanel::width(); x++)
        {
            const uint8_t address = static_cast<uint8_t>(y * lcd()->bufferWidth() + x + panelShift_);
            ui->lcdPanel->setCharacterData(QPoint(x, y), lcd()->charMatrix(address));
        }
    }
}

void LCDView::onLCDCharaterChanged(uint8_t address)
{
    auto pos = panelPos(address);
    if (LCDCharPanel::outOfView(pos))
        return;
    ui->lcdPanel->setCharacterData(pos, lcd()->charMatrix(address));
}

void LCDView::onLCDBusyChanged()
{
    ui->busyFlag->setValue(lcd()->isBusy() ? 1 : 0);
}

void LCDView::onLCDCursorPosChanged()
{
    ui->lcdPanel->setCursorPos(panelPos(lcd()->cursorPos()));
}

void LCDView::onLCDCursorChanged()
{
    ui->lcdPanel->setCursorOn(lcd()->isCursorOn());
}

void LCDView::onLCDDisplayShiftChanged()
{
    panelShift_ = lcd()->displayShift();
    redrawCharacters();
}

void LCDView::onLCDDisplayChanged()
{
    ui->lcdPanel->setDisplayOn(lcd()->isDisplayOn());
}

QPoint LCDView::panelPos(uint16_t address) const
{
    const int y = address / lcd()->bufferWidth();
    const int x = address - y - panelShift_;
    return QPoint(x, y);
}
