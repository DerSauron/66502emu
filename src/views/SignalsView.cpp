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

#include "SignalsView.h"
#include "ui_SignalsView.h"

#include "board/Board.h"
#include "BitsView.h"

SignalsView::SignalsView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SignalsView),
    board_{}
{
    ui->setupUi(this);
    setup();
}

SignalsView::~SignalsView()
{
    delete ui;
}

void SignalsView::setup()
{
    ui->rwLine->setBitCount(1);
    ui->irqLine->setBitCount(1);
    ui->nmiLine->setBitCount(1);
    ui->resetLine->setBitCount(1);
    ui->syncLine->setBitCount(1);
}

void SignalsView::setBoard(Board* board)
{
    board_ = board;

    connect(board_, &Board::signalChanged, this, &SignalsView::onBoardSignalChanged);
    onBoardSignalChanged();
}

void SignalsView::onBoardSignalChanged()
{
    ui->rwLine->setValue(toInt(board_->rwLine()));
    ui->irqLine->setValue(toInt(board_->irqLine()));
    ui->nmiLine->setValue(toInt(board_->nmiLine()));
    ui->resetLine->setValue(toInt(board_->resetLine()));
    ui->syncLine->setValue(toInt(board_->syncLine()));
}

void SignalsView::on_resetButton_clicked()
{
    board_->setResetLine(WireState::Low);
}
