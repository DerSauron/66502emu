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
#include <QMessageBox>
#include <QTextBlock>
#include <QTextCursor>

SourcesView::SourcesView(const QString& name, Program* program, MainWindow* mainWindow, QWidget* parent) :
    View(name, mainWindow, parent),
    ui(new Ui::SourcesView),
    program_{},
    clockRunning_{}
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
    connect(mainWindow()->board()->debugger(), &Debugger::newInstructionStart, this, &SourcesView::onNewInstructionStart);
    connect(mainWindow()->board()->debugger(), &Debugger::failStateChanged, this, &SourcesView::onDebuggerFailStateChanged);
    connect(ui->stepInstructionButton, &QPushButton::clicked, mainWindow()->board()->debugger(), &Debugger::stepInstruction);
    connect(ui->stepSubroutineButton, &QPushButton::clicked, mainWindow()->board()->debugger(), &Debugger::stepSubroutine);
    connect(this, &SourcesView::addBreakpoint, mainWindow()->board()->debugger(), &Debugger::addBreakpoint);
    connect(this, &SourcesView::removeBreakpoint, mainWindow()->board()->debugger(), &Debugger::removeBreakpoint);

    connect(ui->codeView, &ce::CodeEditor::lineNumberDoubleClicked, this, &SourcesView::onLineNumberDoubleClicked);

    onClockRunningChanged();
}

void SourcesView::handleProgramChange()
{
    setProgramText();
    buildAddressIndex();
    setHighlighterRules();
}

int SourcesView::findLineForAddress(int32_t address)
{
    auto pos = addressMap_.find(address);
    if (pos == addressMap_.end())
        return -1;
    return pos.value();
}

void SourcesView::highlightCurrentLine(int32_t address)
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

void SourcesView::setButtonStates()
{
    if (clockRunning_)
        ui->startStopButton->showStopMode();
    else
        ui->startStopButton->showStartMode();
    ui->stepInstructionButton->setEnabled(!clockRunning_);
    ui->stepSubroutineButton->setEnabled(!clockRunning_ && !mainWindow()->board()->debugger()->isFailState());
}

void SourcesView::onClockRunningChanged()
{
    clockRunning_ = mainWindow()->board()->clock()->isRunning();

    onNewInstructionStart();
}

void SourcesView::onNewInstructionStart()
{
    if (!clockRunning_)
    {
        int32_t address = mainWindow()->board()->addressBus()->typedData<int32_t>();
        highlightCurrentLine(address);
    }
}

void SourcesView::onDebuggerFailStateChanged()
{
    setButtonStates();

    if (mainWindow()->board()->debugger()->isFailState())
    {
        const QString reason = QStringLiteral("JSR-RTS calls mismatch");
        QMessageBox::warning(this,
                             QStringLiteral("Malformed program"),
                             QStringLiteral("The program is malformed: %1").arg(reason),
                             QMessageBox::Ok, QMessageBox::Ok);
    }
}

void SourcesView::onLineNumberDoubleClicked(int line)
{
    int address = -1;
    while (line < program_->sourceLines().size())
    {
        if (line >= program_->sourceLines().size())
            return;
        address = program_->sourceLines()[line].address;
        if (address != -1)
            break;
        line++;
    }
    if (address == -1)
        return;

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
