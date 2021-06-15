#include "Debugger.h"

#include "Board.h"
#include "Bus.h"
#include "Clock.h"
#include <QThread>
#include <QTimer>

Debugger::Debugger(Board* board) :
    QObject(board),
    board_(board)
{
}

Debugger::~Debugger()
{
}

bool Debugger::breakpointMatches(int address) const
{
    const auto pos = breakpoints_.find(address);
    return pos != breakpoints_.end();
}

void Debugger::addBreakpoint(int address)
{
    Q_ASSERT(QThread::currentThreadId() == thread());
    breakpoints_.insert(address);
}

void Debugger::removeBreakpoint(int address)
{
    Q_ASSERT(QThread::currentThreadId() == thread());
    breakpoints_.remove(address);
}

void Debugger::stopAtBreakpoint(uint16_t address)
{
    if (breakpointMatches(address))
    {
        board_->clock()->stop();
    }
}

void Debugger::onNewInstructionStart()
{
    uint16_t address = board_->addressBus()->typedData<uint16_t>();
    stopAtBreakpoint(address);
}
