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

extern "C" {
struct _m6522_t;
typedef _m6522_t m6522_t;
}

class Bus;

class VIA : public Device
{
    Q_OBJECT

public:
    VIA(const QString& name, Board* board);
    ~VIA() override;

    void setPinOffset(uint8_t offset);
    bool isUseNmi() const { return useNmi_; };
    void setUseNmi(bool useNmi) { useNmi_ = useNmi; };

    uint8_t pa() const;
    void setPa(uint8_t pa);

    uint8_t pb() const;
    void setPb(uint8_t pb);

    uint8_t paDir() const;
    uint8_t pbDir() const;
    uint16_t t1() const;
    uint16_t t1l() const;
    uint16_t t2() const;
    uint8_t t2l() const;
    uint8_t ifr() const;
    uint8_t ier() const;
    uint8_t acr() const;
    uint8_t pcr() const;

signals:
    void paChanged();
    void pbChanged();
    void t1Changed();
    void t2Changed();
    void ifrChanged();
    void acrChanged();
    void pcrChanged();

protected:
    uint64_t mapPortTag(const QString& portTagName) const override;
    QString mapPortTagName(uint64_t portTag) const override;
    int32_t calcMapAddressEnd() const override;
    void deviceClockEdge(StateEdge edge) override;

private:
    void setPin(uint64_t pin, WireState state);
    uint8_t registerAddress();
    void injectState();
    void populateState();
    void injectPBusses();
    uint8_t injectPBusImpl(const BusConnection& bc, uint8_t pPins, uint8_t pDirs);
    void populatePBusses();
    void populatePBusImpl(const BusConnection& bc, uint8_t pPins, uint8_t pDirs);

private:
    m6522_t* chip_;
    uint64_t pinState_;
    uint16_t previouseT1State_;
    uint16_t previouseT2State_;
    uint16_t previouseIFRState_;
    uint8_t rsPinOffset_;
    bool useNmi_;
};
