#include "ACIA.h"

#include "Board.h"
#include "Bus.h"
#include <QTimer>

namespace {

enum class Register : uint8_t
{
    Data = 0x00,
    Status = 0x01,
    Command = 0x02,
    Control = 0x03,
};

enum Command : uint8_t
{
    DataTerminalReady = 0x01,
    ReceiverIRQDisabled = 0x02,
    TransmitIRQMode_NI = 0x00,
    ReceiverEchoMode = 0x10,
    ParityEnabled = 0x20,
    ParityMode_NI = 0x00,
};

constexpr int BaudTimeMap[] = {
    9,
    20000,
    13333,
    9174,
    7463,
    6667,
    3333,
    1667,
    833,
    556,
    417,
    278,
    208,
    139,
    104,
    52
};

inline constexpr uint8_t baudRateConfig(uint8_t reg)
{
    return reg & 0x0F;
}

inline constexpr uint8_t externClockSource(uint8_t reg)
{
    return (reg & 0x10) == 0;
}
inline constexpr uint8_t wordLength(uint8_t reg)
{
    return 8 - ((reg & 0x60) >> 6);
}
inline constexpr bool stopBits(uint8_t reg)
{
    return ((reg & 0x0F) >> 7) + 1;
}

} // namespace

ACIA::ACIA(const QString& name, Board* board) :
    Device(name, board),
    transmitDelay_(new QTimer(this)),
    receiveDelay_(new QTimer(this))
{
    receiveBuffer_.resize(1024);

    transmitDelay_->setSingleShot(true);
    connect(transmitDelay_, &QTimer::timeout, this, &ACIA::transmitDelayTimeout);
    receiveDelay_->setSingleShot(true);
    connect(receiveDelay_, &QTimer::timeout, this, &ACIA::receiveDelayTimeout);

    resetChip(true);
}

ACIA::~ACIA()
{
}

uint8_t ACIA::status() const
{
    return status_;
}

uint8_t ACIA::baudRate() const
{
    return baudRateConfig(controlRegister_);
}

bool ACIA::isTransmitting() const
{
    return transmitDelay_->isActive();
}

bool ACIA::isReceiving() const
{
    return receiveDelay_->isActive();
}

void ACIA::setBaudDelayFactor(int factor)
{
    baudDelayFactor_ = factor;
}

void ACIA::receiveByte(uint8_t byte)
{
    receiveBuffer_.append(byte);
    if (!isReceiving())
        startReceive();
}

uint16_t ACIA::calcMapAddressEnd() const
{
    return mapAddressStart_ + 0x03;
}

void ACIA::deviceClockEdge(StateEdge edge)
{
    if (isLow(board()->resetLine()))
    {
        resetChip(true);
        return;
    }

    if (isRaising(edge) && isSelected())
    {
        if (isLow(board()->rwLine()))
            injectState();
        else // if (isHigh(board()->rwLine()))
            populateState();
    }
}

void ACIA::resetChip(bool hard)
{
    if (hard)
    {
        controlRegister_ = 0;
        commandRegister_ = 0;
    }
    else
    {
        commandRegister_ &= 0b11100000;
    }

}

void ACIA::injectState()
{
    Register rs = static_cast<Register>(board()->addressBus()->typedData<uint8_t>() & 0x03);
    uint8_t data = board()->dataBus()->typedData<uint8_t>();

    switch (rs)
    {
        case Register::Data:
            transmitData_ = data;
            status_ |= TransmitterEmpty; // WDC BUG
            if (!isTransmitting())
                startTransmit();
            break;
        case Register::Status:
            resetChip(false);
            break;
        case Register::Command:
            commandRegister_ = data;
            break;
        case Register::Control:
            controlRegister_ = data;
            break;
    }
}

void ACIA::populateState()
{
    Register rs = static_cast<Register>(board()->addressBus()->typedData<uint8_t>() & 0x03);

    uint8_t data;
    switch (rs)
    {
        case Register::Data:
            data = receiveData_;
            status_ &= ~(ParityError | FramingError | OverrunError | ReceiverFull);
            break;
        case Register::Status:
            data = status_;
            status_ &= ~(IRQ);
            break;
        case Register::Command:
            data = commandRegister_;
            break;
        case Register::Control:
            data = controlRegister_;
            break;
    }

    board()->dataBus()->setData(data);

    if (status_ & IRQ)
        board()->setIrqLine(WireState::Low);
}

void ACIA::startTransmit()
{
    transmitDelay_->start(baudDelay());
    emit transmittingChanged();
}

void ACIA::startReceive()
{
    receiveDelay_->start(baudDelay());
    emit receivingChanged();
}

int ACIA::baudDelay()
{
    return BaudTimeMap[baudRate()] * baudDelayFactor_;
}

void ACIA::transmitDelayTimeout()
{
    emit sendByte(transmitData_);
    emit transmittingChanged();

    // WDC BUG
    // - no transmitter empty
    // - no irq
}

void ACIA::receiveDelayTimeout()
{
    emit receivingChanged();

    uint8_t byte = receiveBuffer_.takeFirst();

    if (status_ & ReceiverFull)
    {
        status_ |= OverrunError;
    }
    else
    {
        receiveData_ = byte;
        status_ |= ReceiverFull;

        if ((commandRegister_ & ReceiverIRQDisabled) == 0)
        {
            status_ |= IRQ;
        }
    }

    if (!receiveBuffer_.isEmpty())
        startReceive();
}