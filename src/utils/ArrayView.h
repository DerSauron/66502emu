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

#include <cinttypes>

class ArrayView
{
public:
    class Iterator
    {
    public:
        Iterator(const uint8_t* p) : p_{p} {}
        ~Iterator() = default;

        Iterator(const Iterator&) = default;
        Iterator& operator=(const Iterator&) = default;
        Iterator(Iterator&&) = default;
        Iterator& operator=(Iterator&&) = default;

        const uint8_t& operator*() { return *p_; }
        bool operator!=(const Iterator& rhs) const { return p_ != rhs.p_; }
        void operator++() { ++p_; }

    private:
          const uint8_t* p_;
    };

public:
    ArrayView(const uint8_t* data, int size) :
        data_(data),
        size_(size)
    {
    }

    template<typename T>
    ArrayView(const T& container) :
        data_(container.data()),
        size_(container.size())
    {
    }

    uint8_t operator[](int index) const { return data_[index]; }
    int size() const { return size_; }

    Iterator begin() const { return Iterator{data_}; }
    Iterator end() const { return Iterator{data_ + size_}; }

private:
    const uint8_t* data_;
    int size_;
};

