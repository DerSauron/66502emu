/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include "views/View.h"
#include <QMap>

namespace Ui {
class SourcesView;
}

class Program;

class SourcesView : public View
{
    Q_OBJECT

public:
    SourcesView(const QString& name, Program* program, MainWindow* mainWindow, QWidget* parent);
    ~SourcesView() override;

    void setProgram(Program* program);

signals:
    void addBreakpoint(int32_t address);
    void removeBreakpoint(int32_t address);
    void breakpointChanged(int32_t line);

private slots:
    void onClockRunningChanged();
    void onNewInstructionStart();
    void onDebuggerFailStateChanged();
    void onLineNumberDoubleClicked(int32_t line);
    void on_startStopButton_clicked();

private:
    struct Labels
    {
        QList<QString> eqLabels;
        QList<QString> absLabels;
    };

private:
    void setup();
    void handleProgramChange();
    int findLineForAddress(int32_t address);
    void highlightCurrentLine(int32_t address);
    void setProgramText();
    void buildAddressIndex();
    Labels scanLabels();
    void setHighlighterRules();
    void setButtonStates();

private:
    Ui::SourcesView* ui;
    Program* program_;
    QMap<int32_t, int32_t> addressMap_;
    bool clockRunning_;

    Q_DISABLE_COPY_MOVE(SourcesView)
};
