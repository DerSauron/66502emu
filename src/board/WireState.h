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

#include <inttypes.h>
#include <limits>

enum class WireState
{
    None,
    Low,
    High,
};

enum class StateEdge
{
    Invalid,
    Raising,
    Falling,
};

inline bool isNone(WireState state)
{
    return state == WireState::None;
}

inline bool isHigh(WireState state)
{
    return state == WireState::High;
}

inline bool isLow(WireState state)
{
    return state == WireState::Low;
}

inline WireState negate(WireState state)
{
    switch (state)
    {
        case WireState::Low:
            return WireState::High;
        case WireState::High:
            return WireState::Low;
        default:
            return state;
    }
}

inline WireState toState(uint64_t value)
{
    return value != 0 ? WireState::High : WireState::Low;
}

inline uint64_t toInt(WireState state)
{
    switch (state)
    {
        case WireState::Low:
            return 0;
        case WireState::High:
            return 1;
        default:
            break;
    }

    return std::numeric_limits<uint64_t>::max();
}

inline bool isRaising(StateEdge edge)
{
    return edge == StateEdge::Raising;
}

inline bool isFalling(StateEdge edge)
{
    return edge == StateEdge::Falling;
}
