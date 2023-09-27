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

#include "BoardLoader.h"

#include "board/ACIA.h"
#include "board/Board.h"
#include "board/Bus.h"
#include "board/BusConnection.h"
#include "board/LCD.h"
#include "board/Memory.h"
#include "board/VIA.h"
#include "BoardFile.h"
#include <QDebug>
#include <QIODevice>
#include <QThreadPool>
#include <optional>

namespace {

auto createBusses(const BoardInfo& boardInfo, Board* board)
{
    static int busAutoNameIndex{0};

    QVector<Bus*> busses;

    for (const auto& busInfo : boardInfo.busses)
    {
        auto busName = busInfo.name;
        if (busName.isEmpty())
            busName = QStringLiteral("Bus-%1").arg(busAutoNameIndex++);

        auto busWidth = busInfo.width;
        if (busWidth == 0 || busWidth > 64)
        {
            qWarning() << "Invalid bus width" << busWidth << "for" << busName;
            continue;
        }

        busses.append(new Bus{busName, busInfo.width, board});
    }

    return std::make_pair(true, busses);
}

void saveBusses(BoardInfo& boardInfo, const Board* board)
{
    boardInfo.busses.clear();

    for (const auto& bus : board->busses())
    {
        boardInfo.busses.append({
                                    bus->name(),
                                    bus->width()
                                });
    }
}

bool createBusConnections(Device* device, const DeviceInfo& deviceInfo, QVector<Bus*> busses, Board* board)
{
    auto baseInfo = deviceCommon(deviceInfo);

    for (const auto& conInfo : baseInfo.connections)
    {
        if (conInfo.busName.isEmpty() || conInfo.busMask == 0 || conInfo.portName.isEmpty() || conInfo.portMask == 0)
        {
            qWarning() << "Invalid bus connection definition";
            continue;
        }

        //Bus* bus = board->bus(conInfo.busName);
        auto bus = std::find_if(busses.begin(), busses.end(),
                                [&conInfo](const auto& bus) { return bus->name() == conInfo.busName; });
        if (bus == busses.end())
        {
            qWarning() << "No such bus" << conInfo.busName;
            continue;
        }

        device->addBusConnection(conInfo.portName, conInfo.portMask, *bus, conInfo.busMask);
    }

    return true;
}

void saveBusConnections(DeviceInfo& deviceInfo, const Device* device)
{
    auto baseInfo = deviceCommon(deviceInfo);

    baseInfo.connections.clear();

    for (const auto& con : device->busConnections())
    {
        baseInfo.connections.append({
                                         con.bus()->name(),
                                         con.busMask(),
                                         device->portTagName(con),
                                         con.portMask()
                                     });
    }
}

Device* createDevice(const DeviceInfo& deviceInfo, Board* board)
{
    static int deviceAutoNameIndex{0};

    auto baseInfo = deviceCommon(deviceInfo);

    auto deviceName = baseInfo.name;
    if (deviceName.isEmpty())
        deviceName = QStringLiteral("Device-%1").arg(deviceAutoNameIndex++);

    Device* device = nullptr;

    switch (baseInfo.type)
    {
        case DeviceType::Memory:
        {
            const auto& memInfo = std::get<MemoryInfo>(deviceInfo);
            device = new Memory(memInfo.memoryType, memInfo.memorySize, deviceName, board);
            break;
        }

        case DeviceType::VIA:
        {
            const auto& viaInfo = std::get<ViaInfo>(deviceInfo);
            auto* via = new VIA(deviceName, board);
            via->setUseNmi(viaInfo.useNMI);
            device = via;
            break;
        }

        case DeviceType::ACIA:
        {
            const auto& aciaInfo = std::get<AciaInfo>(deviceInfo);
            Q_UNUSED(aciaInfo)
            device = new ACIA(deviceName, board);
            break;
        }

        case DeviceType::LCD:
        {
            const auto& lcdInfo = std::get<LcdInfo>(deviceInfo);
            Q_UNUSED(lcdInfo)
            device = new LCD(deviceName, board);
            break;
        }
    }

    if (!device)
        return nullptr;

    device->setMapAddressStart(baseInfo.address);

    return device;
}

std::optional<DeviceInfo> saveDevice(const Device* device)
{
    std::optional<DeviceInfo> deviceInfo{};

    DeviceCommonInfo commonInfo{};
    commonInfo.name = device->name();
    commonInfo.address = device->mapAddressStart();

    if (const auto* memory = qobject_cast<const Memory*>(device))
    {
        commonInfo.type = DeviceType::Memory;
        deviceInfo = MemoryInfo{commonInfo, memory->type(), memory->size()};
    }
    else if (const auto* via = qobject_cast<const VIA*>(device))
    {
        commonInfo.type = DeviceType::VIA;
        deviceInfo = ViaInfo{commonInfo, via->isUseNmi()};
    }
    else if (const auto* acia = qobject_cast<const ACIA*>(device))
    {
        Q_UNUSED(acia)
        commonInfo.type = DeviceType::ACIA;
        deviceInfo = AciaInfo{commonInfo};
    }
    else if (const auto* lcd = qobject_cast<const LCD*>(device))
    {
        Q_UNUSED(lcd)
        commonInfo.type = DeviceType::LCD;
        deviceInfo = LcdInfo{commonInfo};
    }

    return deviceInfo;
}

auto createDevices(const BoardInfo& boardInfo, QVector<Bus*> busses, Board* board)
{
    QVector<Device*> devices;

    for (const auto& deviceInfo : boardInfo.devices)
    {
        Device* device = createDevice(deviceInfo, board);
        if (!device)
        {
            qWarning() << "Invalid device definition";
            continue;
        }

        createBusConnections(device, deviceInfo, busses, board);

        devices.append(device);
    }

    return std::make_pair(true, devices);
}

void saveDevices(BoardInfo& boardInfo, const Board* board)
{
    boardInfo.devices.clear();
    for (const auto& device : board->devices())
    {
        auto info = saveDevice(device);
        if (!info)
            continue;

        saveBusConnections(info.value(), device);

        boardInfo.devices.append(info.value());
    }
}

bool validateOverlapping(Board* board)
{
    bool result = true;
    const auto& devices = board->devices();
    for (int a = 0; a < devices.size(); a++)
    {
        if (devices[a]->mapAddressStart() > devices[a]->mapAddressEnd())
            continue;

        for (int b = a + 1; b < devices.size(); b++)
        {
            if (devices[b]->mapAddressStart() > devices[b]->mapAddressEnd())
                continue;

            if (devices[a]->mapAddressStart() <= devices[b]->mapAddressEnd() &&
                devices[b]->mapAddressStart() <= devices[a]->mapAddressEnd())
            {
                qWarning() << "Address map from" << devices[a]->name() << "and" << devices[b]->name() << "overlap";
                result = false;
            }
        }
    }

    return result;
}

} // namespace


BoardLoader::BoardLoader(BoardInfo& boardInfo, QObject* parent) :
   QObject{parent},
   boardInfo_{boardInfo}
{
}

void BoardLoader::load(Board* board)
{
    QThreadPool::globalInstance()->start([this, board]() -> void {
        auto result = loadImpl(boardInfo_, board);

        emit loaded(result);
    });
}

bool BoardLoader::loadImpl(const BoardInfo& boardInfo, Board* board)
{
    auto bussesResult = createBusses(boardInfo, board);
    if (!bussesResult.first)
    {
        qWarning() << "Could not create system busses";
        return false;
    }

    auto devicesResult = createDevices(boardInfo, bussesResult.second, board);
    if (!devicesResult.first)
    {
        qWarning() << "Could not create devices";
        return false;
    }

    if (!validate(board))
    {
        qWarning() << "Board addresses overlap";
        return false;
    }

    board->reset(devicesResult.second, bussesResult.second);

    return true;
}

void BoardLoader::save(const Board* board)
{
    QThreadPool::globalInstance()->start([this, board]() -> void {
        auto result = saveImpl(boardInfo_, board);

        emit saved(result);
    });
}

bool BoardLoader::saveImpl(BoardInfo& boardInfo, const Board* board)
{
    saveBusses(boardInfo, board);
    saveDevices(boardInfo, board);
    return true;
}

bool BoardLoader::validate(Board* board)
{
    return validateOverlapping(board);
}
