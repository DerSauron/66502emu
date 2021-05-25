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

#include <cassert>
#include <cinttypes>
#include <type_traits>

#define COUNT_ZERO_BITS(x) __builtin_ctz(x)

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T extractBits(T value, T mask)
{
    if (mask == 0)
        return T{};
    const auto shift = COUNT_ZERO_BITS(mask);
    return (value & mask) >> shift;
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T injectBits(T value, T mask, T data)
{
    if (mask == 0)
        return T{};
    const auto shift = COUNT_ZERO_BITS(mask);
    return (value & ~mask) | ((data << shift) & mask);
}

constexpr uint8_t bitCount(unsigned long long value)
{
    return static_cast<uint8_t>(__builtin_popcountll(value));
}
