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
    explicit SourcesView(const QString& memoryName, Program* program, MainWindow* parent = nullptr);
    ~SourcesView();

    void setProgram(Program* program);

signals:
    void breakpointChanged(int line);

private slots:
    void onNewInstructionStart();
    void onLineNumberDoubleClicked(int line);

private:
    void setup();
    void handleProgramChange();
    int findLineForAddress(uint16_t address);
    void highlightCurrentLine(uint16_t address);
    void stopAtBreakpoint(uint16_t address);

private:
    Ui::SourcesView *ui;
    Program* program_;
    QMap<int, int> addressMap_;
};
