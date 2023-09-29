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

#include "DisassemblerView.h"
#include "ui_DisassemblerView.h"

#include "MainWindow.h"
#include "M6502Disassembler.h"
#include "board/Board.h"
#include "board/Bus.h"
#include "board/Clock.h"
#include "board/Debugger.h"
#include "board/Memory.h"
#include <QScrollBar>

namespace {

QString toString(int32_t baseAddress, const M6502::Instruction& instruction)
{
    QString bytes;
    for (uint8_t i = 0; i < instruction.length; i++)
    {
        bytes += QString(QLatin1String("%1 ")).arg(instruction.bytes[i], 2, 16, QLatin1Char('0'));
    }

    QString output = QStringLiteral("%1: %2 %3 ; %4");
    return output
            .arg(baseAddress + instruction.position, 4, 16, QLatin1Char('0'))
            .arg(bytes, -9)
            .arg(instruction.instruction, -10)
            .arg(instruction.comment);
}

} // namespace

const QFont DisassemblerView::normalFont(QLatin1String("Monospace"), 11);
const QFont DisassemblerView::boldFont(QLatin1String("Monospace"), 11, QFont::ExtraBold);
const QBrush DisassemblerView::normalColor(QColor(128, 128, 128));
const QBrush DisassemblerView::highlightColor(QColor(128, 128, 255));
const QBrush DisassemblerView::dimmColor(QColor(200, 200, 200));

DisassemblerView::DisassemblerView(const QString& name, MainWindow* mainWindow) :
    View{name, mainWindow},
    ui{new Ui::DisassemblerView{())}},
    instructionsLookAhead_{5},
    currentIndex_{0}
{
    ui->setupUi(this);
    setup();
}

DisassemblerView::~DisassemblerView()
{
    delete ui;
}

void DisassemblerView::setup()
{
    connect(mainWindow()->board()->clock(), &Clock::runningChanged, this, &DisassemblerView::onClockRunningChanged);
    onClockRunningChanged();
}

void DisassemblerView::onClockRunningChanged()
{
    // Run update in sync when clock is not running automatically

    auto board = mainWindow()->board();
    if (board->clock()->isRunning())
    {
        setEnabled(false);
        disconnect(board->debugger(), &Debugger::newInstructionStart, this, &DisassemblerView::onNewInstructionStart);
    }
    else
    {
        setEnabled(true);
        connect(board->debugger(), &Debugger::newInstructionStart, this, &DisassemblerView::onNewInstructionStart);
    }
}

void DisassemblerView::onNewInstructionStart()
{
    auto board = mainWindow()->board();

    uint16_t address = board->addressBus()->typedData<uint16_t>();

    auto mem = board->findDevice<Memory>(address);
    if (!mem)
        return;

    showAddress(mem, address);
}

void DisassemblerView::on_toolButton_triggered(QAction* action)
{
    ui->disassembly->clear();
    currentIndex_ = 0;
}

void DisassemblerView::showAddress(Memory* memory, int32_t address)
{
    bool wasScrollEnd = isScrollEnd();

    const int32_t baseAddress = memory->mapAddressStart();
    const auto instructions = M6502::disassembleCount(memory,
                                                      instructionsLookAhead_ + 1,
                                                      address - baseAddress);

    if (!instructions.isEmpty())
        showCurrent(baseAddress, instructions[0]);

    showLookAheads(baseAddress, instructions);

    if (wasScrollEnd)
        doScrollEnd();
}

void DisassemblerView::showCurrent(int32_t baseAddress, const M6502::Instruction& instruction)
{
    if (currentIndex_ < ui->disassembly->count())
    {
        auto currentItem = ui->disassembly->item(currentIndex_);
        currentItem->setFont(normalFont);
        currentItem->setForeground(normalColor);
        currentIndex_++;
    }

    auto item = new QListWidgetItem(toString(baseAddress, instruction));
    item->setFont(boldFont);
    item->setForeground(highlightColor);
    item->setSelected(true);
    ui->disassembly->insertItem(currentIndex_, item);
}

void DisassemblerView::showLookAheads(int32_t baseAddress, const QList<M6502::Instruction>& instructions)
{
    int row = currentIndex_ + 1;
    for (int i = 1; i < instructions.length(); i++, row++)
    {
        QListWidgetItem* item{};
        if (row >= ui->disassembly->count())
        {
            item = new QListWidgetItem(ui->disassembly);
            item->setFont(normalFont);
            item->setForeground(dimmColor);
        }
        else
            item = ui->disassembly->item(row);
        item->setText(toString(baseAddress, instructions[i]));
    }
    for (; row < ui->disassembly->count(); row++)
    {
        ui->disassembly->item(row)->setText(QString());
    }
}

bool DisassemblerView::isScrollEnd() const
{
    const auto* scrollBar = ui->disassembly->verticalScrollBar();
    return scrollBar->value() == scrollBar->maximum();
}

void DisassemblerView::doScrollEnd()
{
    ui->disassembly->scrollToBottom();
}
