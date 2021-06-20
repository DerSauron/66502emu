#pragma once

#include "WireState.h"
#include <QObject>
#include <QSet>

class Board;

class Debugger : public QObject
{
    Q_OBJECT

public:
    explicit Debugger(Board* board);
    ~Debugger() override;

    uint8_t lastInstruction() const { return lastInstruction_; }
    uint16_t lastInstructionStart() const { return lastInstructionStart_; }
    uint8_t currentInstruction() const { return currentInstruction_; }
    uint16_t currentInstructionStart() const { return currentInstructionStart_; }

    bool breakpointMatches(int address) const;

signals:
    void newInstructionStart();

public slots:
    void stepInstruction();
    void stepSubroutine();

    void addBreakpoint(int address);
    void removeBreakpoint(int address);

private:
    void updateInstructionState(uint16_t address);
    void updateCallStack();
    void stopAtBreakpoint(uint16_t address);
    void stopAfterInstruction();
    void stopAfterSubroutine();
    void handleNewInstructionStart();

private slots:
    void onClockEdge(StateEdge edge);

private:
    enum class SteppingMode
    {
        None,
        Instruction,
        Subroutine,
    };

private:
    Board* board_;
    uint8_t jsrOpcode_{};
    uint8_t rtsOpcode_{};
    uint8_t lastInstruction_{};
    uint16_t lastInstructionStart_{};
    uint8_t currentInstruction_{};
    uint16_t currentInstructionStart_{};
    SteppingMode steppingMode_{SteppingMode::None};
    QSet<int> breakpoints_;
    QVector<int> callStack_;
    int steppingSubroutineCallStackStart_{};
};
