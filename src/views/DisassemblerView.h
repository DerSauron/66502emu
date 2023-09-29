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

#pragma once

#include "View.h"
#include "board/WireState.h"

class Board;
class Memory;

namespace M6502 {
struct Instruction;
}

namespace Ui {
class DisassemblerView;
}

class DisassemblerView : public View
{
    Q_OBJECT

public:
    DisassemblerView(const QString& name, MainWindow* mainWindow);
    ~DisassemblerView() override;

    void setLookAhead(int instructions) { instructionsLookAhead_ = instructions; }

signals:

private slots:
    void onClockRunningChanged();
    void onNewInstructionStart();
    void on_toolButton_triggered(QAction* action);

private:
    void setup();
    void showAddress(Memory* memory, int32_t address);
    void showCurrent(int32_t baseAddress, const M6502::Instruction& instruction);
    void showLookAheads(int32_t baseAddress, const QList<M6502::Instruction>& instructions);
    bool isScrollEnd() const;
    void doScrollEnd();

private:
    static const QFont normalFont;
    static const QFont boldFont;
    static const QBrush normalColor;
    static const QBrush highlightColor;
    static const QBrush dimmColor;

private:
    Ui::DisassemblerView* ui;
    int32_t instructionsLookAhead_;
    int32_t currentIndex_;

    Q_DISABLE_COPY_MOVE(DisassemblerView)
};
