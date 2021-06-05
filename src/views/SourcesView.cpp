/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "SourcesView.h"
#include "ui_SourcesView.h"

#include "MainWindow.h"
#include "Program.h"
#include "board/Board.h"
#include "board/Bus.h"
#include "board/Clock.h"
#include <QTextBlock>
#include <QTextCursor>

SourcesView::SourcesView(const QString& memoryName, Program* program, MainWindow* parent) :
    View(memoryName + QLatin1String(" - Source"), parent),
    ui(new Ui::SourcesView)
{
    ui->setupUi(this);
    setup();
    setProgram(program);
}

SourcesView::~SourcesView()
{
    delete ui;
}

void SourcesView::setProgram(Program* program)
{
    program_ = program;
    handleProgramChange();
}

void SourcesView::setup()
{
    connect(mainWindow()->board(), &Board::newInstructionStart, this, &SourcesView::onNewInstructionStart);
    connect(ui->codeView, &ce::CodeEditor::lineNumberDoubleClicked, this, &SourcesView::onLineNumberDoubleClicked);
}

void SourcesView::handleProgramChange()
{
    ui->codeView->clear();
    auto* codeDoc = ui->codeView->document();
    QTextCursor pos(codeDoc);
    int lineIdx = 0;
    for (const auto& line : program_->sourceLines())
    {
        pos.movePosition(QTextCursor::End);
        pos.insertText(line.text);

        addressMap_.insert(line.address, lineIdx);

        lineIdx++;
    }
}

int SourcesView::findLineForAddress(uint16_t address)
{
    auto pos = addressMap_.find(address);
    if (pos == addressMap_.end())
        return -1;
    return pos.value();
}

void SourcesView::highlightCurrentLine(uint16_t address)
{
    auto line = findLineForAddress(address);
    ui->codeView->highlightCurrentAddressLine(line);
}

void SourcesView::stopAtBreakpoint(uint16_t address)
{
    if (program_->breakpointMatches(address))
    {
        mainWindow()->board()->clock()->stop();
    }
}

void SourcesView::onNewInstructionStart()
{
    uint16_t address = mainWindow()->board()->addressBus()->typedData<uint16_t>();
    highlightCurrentLine(address);
    stopAtBreakpoint(address);
}

void SourcesView::onLineNumberDoubleClicked(int line)
{
    int address = -1;
    while (true)
    {
        if (line >= program_->sourceLines().size())
            return;
        address = program_->sourceLines()[line].address;
        if (address != -1)
            break;
        line++;
    }

    if (program_->toggleBreakpoint(address))
        ui->codeView->addBreakpoint(line);
    else
        ui->codeView->removeBreakpoint(line);
}
