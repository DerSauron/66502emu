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
    ~SourcesView();

    void setProgram(Program* program);

signals:
    void addBreakpoint(uint16_t address);
    void removeBreakpoint(uint16_t address);
    void breakpointChanged(int line);

private slots:
    void onClockRunningChanged();
    void onNewInstructionStart();
    void onLineNumberDoubleClicked(int line);
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
    int findLineForAddress(uint16_t address);
    void highlightCurrentLine(uint16_t address);
    void setProgramText();
    void buildAddressIndex();
    Labels scanLabels();
    void setHighlighterRules();

private:
    Ui::SourcesView* ui;
    Program* program_;
    QMap<int, int> addressMap_;
};
