/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle correct emulator.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#include "VIA.h"

#include "Board.h"
#include "Bus.h"
#include "BusConnection.h"
#include "impl/m6522.h"
#include "utils/BitManipulations.h"
#include "utils/Strings.h"

namespace {

enum Tags
{
    PA,
    PB
};


} // namespace

VIA::VIA(const QString& name, Board* board) :
    Device{name, board},
    chip_{new m6522_t},
    pinState_{},
    previouseT1State_{},
    previouseT2State_{},
    previouseIFRState_{},
    rsPinOffset_{},
    useNmi_{}
{
    m6522_init(chip_);
}

VIA::~VIA()
{
    delete chip_;
}

uint64_t VIA::mapPortTag(const QString& portTagName) const
{
    if (portTagName == "PA"_lat1)
        return Tags::PA;
    else if (portTagName == "PB"_lat1)
        return Tags::PB;
    else
        return Device::mapPortTag(portTagName);
}

void VIA::deviceClockEdge(StateEdge edge)
{
    if (isLow(board()->resetLine()))
    {
        m6522_reset(chip_);
        pinState_ = 0;
        return;
    }

    if (edge == StateEdge::Raising)
    {
        injectState();
        injectPBusses();

        pinState_ = m6522_tick(chip_, pinState_);

        populateState();
        populatePBusses();
    }
}

uint8_t VIA::pa() const
{
    return M6522_GET_PA(pinState_);
}

void VIA::setPa(uint8_t _pa)
{
    if (_pa == pa())
        return;

    M6522_SET_PA(pinState_, _pa);
    emit paChanged();
}

uint8_t VIA::pb() const
{
    return M6522_GET_PB(pinState_);
}

void VIA::setPb(uint8_t _pb)
{
    if (_pb == pb())
        return;

    M6522_SET_PB(pinState_, _pb);
    emit pbChanged();
}

uint8_t VIA::paDir() const
{
    return chip_->pa.ddr;
}

uint8_t VIA::pbDir() const
{
    return chip_->pb.ddr;
}

uint16_t VIA::t1() const
{
    return chip_->t1.counter;
}

uint16_t VIA::t1l() const
{
    return chip_->t1.latch;
}

uint16_t VIA::t2() const
{
    return chip_->t2.counter;
}

uint8_t VIA::t2l() const
{
    return chip_->t2.latch;
}

uint8_t VIA::ifr() const
{
    return chip_->intr.ifr;
}

uint8_t VIA::ier() const
{
    return chip_->intr.ier;
}

uint16_t VIA::calcMapAddressEnd() const
{
    return mapAddressStart_ + 0xF;
}

void VIA::setPin(uint64_t pin, WireState state)
{
    if (isHigh(state))
        pinState_ |= pin;
    else if (isLow(state))
        pinState_ &= ~pin;
}

uint8_t VIA::registerAddress()
{
    uint16_t addr = board()->addressBus()->data();
    addr >>= rsPinOffset_;
    return addr;
}

void VIA::injectState()
{
    previouseT1State_ = chip_->t1.counter;
    previouseT2State_ = chip_->t2.counter;
    previouseIFRState_ = chip_->intr.ifr;

    // unconditionally set all pins. Let the "chip" handle the rw,cs,rs flags

    setPin(M6522_RW, board()->rwLine());

    setPin(M6522_CS1, toState(isSelected()));
    setPin(M6522_CS2, negate(toState(isSelected())));

    uint8_t regAddr = registerAddress();
    setPin(M6522_RS0, toState(regAddr & 0x1));
    setPin(M6522_RS1, toState(regAddr & 0x2));
    setPin(M6522_RS2, toState(regAddr & 0x4));
    setPin(M6522_RS3, toState(regAddr & 0x8));

    M6522_SET_DATA(pinState_, board()->dataBus()->data());
}

void VIA::populateState()
{
    if (pinState_ & M6522_IRQ)
    {
        if (useNmi_)
            board()->setNmiLine(WireState::Low);
        else
            board()->setIrqLine(WireState::Low);
    }

    if (isSelected())
    {
        if (isHigh(board()->rwLine()))
        {
            board()->dataBus()->setData(M6522_GET_DATA(pinState_));
        }
        else
        {
            uint8_t regNo = registerAddress() & M6522_RS_PINS;
            if (regNo == M6522_REG_RA)
                emit paChanged();
            else if (regNo == M6522_REG_RB)
                emit pbChanged();
            else
                emit registerChanged(regNo);
        }
    }

    if (previouseT1State_ != chip_->t1.counter)
        emit t1Changed();

    if (previouseT2State_ != chip_->t2.counter)
        emit t2Changed();

    if (previouseIFRState_ != chip_->intr.ifr)
        emit ifrChanged();
}

void VIA::injectPBusses()
{
    for (const auto& bc : qAsConst(busConnections_))
    {
        if (bc.portTag() == Tags::PA)
            setPa(injectPBusImpl(bc, pa(), paDir()));
        else if (bc.portTag() == Tags::PB)
            setPb(injectPBusImpl(bc, pb(), pbDir()));
    }
}

uint8_t VIA::injectPBusImpl(const BusConnection& bc, uint8_t pPins, uint8_t pDirs)
{
    const uint8_t inMask = static_cast<uint8_t>(bc.portMask()) & ~pDirs; // only use the bits from the mask that are inputs
    if (inMask == 0)
        return pPins;
     // shift inMask right to get rid of trailing zeros
    const uint8_t normInMask = extractBits(inMask, static_cast<uint8_t>(bc.portMask()));
    // map inMask to the bus (shift left to fit the bus position)
    const uint64_t busInMask = injectBits(uint64_t{}, bc.busMask(), uint64_t{normInMask});
     // get data from bus
    const uint64_t busValue = extractBits(bc.bus()->data(), busInMask);
    // fill bus data into port
    return injectBits(pPins, inMask, static_cast<uint8_t>(busValue));
}

void VIA::populatePBusses()
{
    for (const auto& bc : qAsConst(busConnections_))
    {
        if (bc.portTag() == Tags::PA)
            populatePBusImpl(bc, pa(), paDir());
        else if (bc.portTag() == Tags::PB)
            populatePBusImpl(bc, pb(), pbDir());
    }
}

void VIA::populatePBusImpl(const BusConnection& bc, uint8_t pPins, uint8_t pDirs)
{
    const uint8_t outMask = static_cast<uint8_t>(bc.portMask()) & pDirs; // only use the bits from the mask that are outputs
    if (outMask == 0)
        return;
     // shift outMask right to get rid of trailing zeros
    const uint8_t normOutMask = extractBits(outMask, static_cast<uint8_t>(bc.portMask()));
    // map outMask to the bus (shift left to fit the bus position)
    const uint64_t busOutMask = injectBits(uint64_t{}, bc.busMask(), uint64_t{normOutMask});
    // fill bus with pin data
    uint8_t pinValue = extractBits(pPins, outMask);
    bc.bus()->setData(injectBits(bc.bus()->data(), busOutMask, uint64_t{pinValue}));
}
