#pragma once

#include <QObject>
#include <QSet>

class Board;

class Debugger : public QObject
{
    Q_OBJECT

public:
    explicit Debugger(Board* board);
    ~Debugger() override;

    bool breakpointMatches(int address) const;

public slots:
    void addBreakpoint(int address);
    void removeBreakpoint(int address);

private:
    void stopAtBreakpoint(uint16_t address);

private slots:
    void onNewInstructionStart();

private:
    Board* board_;
    QSet<int> breakpoints_;
};
