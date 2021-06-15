/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "SourcesView.h"
#include "ui_SourcesView.h"

#include "M6502Disassembler.h"
#include "MainWindow.h"
#include "Program.h"
#include "board/Board.h"
#include "board/Bus.h"
#include "board/Clock.h"
#include "board/Debugger.h"
#include "codeeditor/Highlighter.h"
#include <QTextBlock>
#include <QTextCursor>

SourcesView::SourcesView(const QString& name, Program* program, MainWindow* mainWindow, QWidget* parent) :
    View(name, mainWindow, parent),
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
    connect(mainWindow()->board()->clock(), &Clock::runningChanged, this, &SourcesView::onClockRunningChanged);
    connect(mainWindow()->board(), &Board::newInstructionStart, this, &SourcesView::onNewInstructionStart);
    connect(ui->stepButton, &QPushButton::clicked, mainWindow()->board(), &Board::startSingleInstructionStep);
    connect(ui->codeView, &ce::CodeEditor::lineNumberDoubleClicked, this, &SourcesView::onLineNumberDoubleClicked);
}

void SourcesView::handleProgramChange()
{
    setProgramText();
    buildAddressIndex();
    setHighlighterRules();
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

void SourcesView::setProgramText()
{
    ui->codeView->clear();
    auto* codeDoc = ui->codeView->document();
    QTextCursor pos(codeDoc);
    for (const auto& line : program_->sourceLines())
    {
        pos.insertText(line.text);
    }
    ui->codeView->moveCursor(QTextCursor::Start);
}

void SourcesView::buildAddressIndex()
{
    int lineIdx = 0;
    for (const auto& line : program_->sourceLines())
    {
        addressMap_.insert(line.address, lineIdx++);
    }
}

SourcesView::Labels SourcesView::scanLabels()
{
    static QRegularExpression labelExpr(QStringLiteral("^(\\.?[a-z0-9_]+\\$?)\\s*([:=])"),
                                 QRegularExpression::CaseInsensitiveOption);

    Labels labels;
    for (const auto& line : program_->sourceLines())
    {
        auto match = labelExpr.match(line.text);
        if (!match.hasMatch())
            continue;
        if (match.captured(2) == QLatin1String("="))
            labels.eqLabels.append(match.captured(1));
        else
            labels.absLabels.append(match.captured(1));
    }

    return labels;
}

void SourcesView::setHighlighterRules()
{
    auto labels = scanLabels();
    ui->codeView->highlighter()->setRules(
                M6502::mnemonicList(),
                labels.absLabels,
                labels.eqLabels,
                {QStringLiteral("$"), QStringLiteral("%")},
                {QStringLiteral(";")}
                );
}

void SourcesView::onClockRunningChanged()
{
    bool running = mainWindow()->board()->clock()->isRunning();
    if (running)
        ui->startStopButton->showStopMode();
    else
        ui->startStopButton->showStartMode();
    ui->stepButton->setEnabled(!running);
}

void SourcesView::onNewInstructionStart()
{
    uint16_t address = mainWindow()->board()->addressBus()->typedData<uint16_t>();
    highlightCurrentLine(address);
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

    if (!ui->codeView->hasBreakpoint(line))
    {
        ui->codeView->addBreakpoint(line);
        emit addBreakpoint(address);
    }
    else
    {
        ui->codeView->removeBreakpoint(line);
        emit removeBreakpoint(address);
    }
}

void SourcesView::on_startStopButton_clicked()
{
    auto* clock = mainWindow()->board()->clock();
    if (clock->isRunning())
        clock->stop();
    else
        clock->start();
}
