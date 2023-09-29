/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle accurate emulator gui.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

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

    uint8_t statusRegister() const { return statusRegister_; }
    uint8_t baudRate() const;
    uint8_t commandRegister() const { return commandRegister_; }
    uint8_t controlRegister() const { return controlRegister_; }
    bool isTransmitting() const;
    bool isReceiving() const;
    uint8_t transmitterBuffer() const { return transmitData_; }
    uint8_t receiverBuffer() const { return receiveData_; }

    void setBaudDelayFactor(int factor);

signals:
    void sendByte(uint8_t byte);
    void registerChanged();
    void transmittingChanged();
    void receivingChanged();

public slots:
    void receiveByte(uint8_t byte);

protected:
    int32_t calcMapAddressEnd() const override;
    void deviceClockEdge(StateEdge edge) override;

private:
    void resetChip(bool hard);
    void injectState();
    void populateState();
    void populateGlobalState();
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
    uint8_t statusRegister_{TransmitterEmpty};

    Q_DISABLE_COPY_MOVE(ACIA)
};
