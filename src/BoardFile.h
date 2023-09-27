/*
 * Copyright (C) 2023 Daniel Volk <mail@volkarts.com>
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

#include "board/Memory.h"
#include <QObject>
#include <QSharedPointer>
#include <variant>

class QFile;

struct BusInfo
{
    QString name{};
    uint8_t width{};
};

struct BusConnectionInfo
{
    QString busName;
    uint64_t busMask;
    QString portName;
    uint64_t portMask;
};

enum class DeviceType
{
    Memory,
    VIA,
    ACIA,
    LCD,
};

struct DeviceCommonInfo
{
    DeviceType type{};
    QString name{};
    uint16_t address{};
    QVector<BusConnectionInfo> connections;
};

struct MemoryInfo : DeviceCommonInfo
{
    Memory::Type memoryType{};
    uint32_t memorySize{};
};

struct ViaInfo : DeviceCommonInfo
{
    bool useNMI{};
};

struct AciaInfo : DeviceCommonInfo
{
};

struct LcdInfo : DeviceCommonInfo
{
};

using DeviceInfo = std::variant<MemoryInfo, ViaInfo, AciaInfo, LcdInfo>;

[[maybe_unused]]
constexpr std::array DeviceTypeNames{"Memory", "VIA", "ACIA", "LCD"};

struct BoardInfo
{
    QVector<BusInfo> busses{};
    QVector<DeviceInfo> devices{};
};

inline constexpr DeviceCommonInfo& deviceCommon(DeviceInfo& deviceInfo)
{
    auto dc = std::visit([](auto&& e) -> DeviceCommonInfo* { return &e; }, deviceInfo);
    Q_ASSERT(dc);
    return *dc;
}

inline constexpr const DeviceCommonInfo& deviceCommon(const DeviceInfo& deviceInfo)
{
    auto dc = std::visit([](auto&& e) -> const DeviceCommonInfo* { return &e; }, deviceInfo);
    Q_ASSERT(dc);
    return *dc;
}

class BoardFile : public QObject
{
    Q_OBJECT

public:
    explicit BoardFile(QString fileName, QObject* parent = {});

    const QString& fileName() const { return fileName_; }
    BoardInfo& boardInfo() { return boardInfo_; }
    const BoardInfo& boardInfo() const { return boardInfo_; }

    void load();
    void save();

signals:
    void loaded(bool result);
    void saved(bool result);

private:
    QString fileName_;
    BoardInfo boardInfo_;
};
