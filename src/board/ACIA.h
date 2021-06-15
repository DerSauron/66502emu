#pragma once

#include "Device.h"

class QTimer;

class ACIA : public Device
{
    Q_OBJECT

public:
    enum StatusFlag : uint8_t
    {
        ParityError = 0x01,
        FramingError = 0x02,
        OverrunError = 0x04,
        ReceiverFull = 0x08,
        TransmitterEmpty = 0x10,
        DataCarrierDetected = 0x20,
        DataSetReady = 0x40,
        IRQ = 0x80,
    };

public:
    ACIA(const QString& name, Board* board);
    ~ACIA() override;

    uint8_t status() const;
    uint8_t baudRate() const;
    bool isTransmitting() const;
    bool isReceiving() const;

    void setBaudDelayFactor(int factor);

signals:
    void sendByte(uint8_t byte);
    void transmittingChanged();
    void receivingChanged();

public slots:
    void receiveByte(uint8_t byte);

protected:
    uint16_t calcMapAddressEnd() const override;
    void deviceClockEdge(StateEdge edge) override;

private:
    void resetChip(bool hard);
    void injectState();
    void populateState();
    void startTransmit();
    void startReceive();
    int baudDelay();

private slots:
    void transmitDelayTimeout();
    void receiveDelayTimeout();

private:
    QVector<uint8_t> receiveBuffer_;
    QTimer* transmitDelay_;
    QTimer* receiveDelay_;
    int baudDelayFactor_{10};
    uint8_t controlRegister_{};
    uint8_t commandRegister_{};
    uint8_t transmitData_{};
    uint8_t receiveData_{};
    uint8_t status_{TransmitterEmpty};
};
