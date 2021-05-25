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

#pragma once

#include <cinttypes>

class Bus;

class BusConnection
{
public:
    BusConnection(uint64_t portTag, uint64_t portMask, Bus* bus, uint64_t busMask) :
        portTag_{portTag},
        portMask_{portMask},
        bus_{bus},
        busMask_{busMask}
    {
    }

    uint64_t portTag() const { return portTag_; }
    uint64_t portMask() const { return portMask_; }
    Bus* bus() const { return bus_; }
    uint64_t busMask() const { return busMask_; }

private:
    uint64_t portTag_;
    uint64_t portMask_;
    Bus* bus_;
    uint64_t busMask_;
};
