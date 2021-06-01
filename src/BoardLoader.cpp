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

#include "BoardLoader.h"

#include "board/Board.h"
#include "board/Bus.h"
#include "board/LCD.h"
#include "board/Memory.h"
#include "board/VIA.h"
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

int numberBase(const QString& string, int& offset)
{
    if ((string.length() < 2) || (string[0] != QLatin1Char('0')))
    {
        offset = 0;
        return 10;
    }
    if (string[1] == QLatin1Char('b'))
    {
        offset = 2;
        return 2;
    }
    if (string[1] == QLatin1Char('x'))
    {
        offset = 2;
        return 16;
    }
    offset = 1;
    return 8;
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
std::optional<T> parseNumber(const QString& string)
{
    int offset;
    int base = numberBase(string, offset);
    bool ok;
    T number = static_cast<T>(string.midRef(offset).toULongLong(&ok, base));
    if (!ok)
        return {};
    return number;
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
std::optional<T> extractNumber(const QJsonValue& value)
{
    if (value.isUndefined())
        return {};

    if (value.isDouble())
        return static_cast<T>(value.toInt());

    if (value.isString())
        return parseNumber<T>(value.toString());

    return {};
}

QJsonDocument openJsonDocument(QIODevice* input)
{
    QByteArray jsonData = input->readAll();
    if (jsonData.isEmpty())
    {
        qWarning() << "Failed to load from input";
        return {};
    }

    QJsonParseError error{};
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (doc.isNull())
    {
        qWarning() << "Failed to parse input json" << error.errorString() << "at" << error.offset;
        return {};
    }

    return doc;
}

bool createBus(Board* board, const QJsonObject& busDef)
{
    static int busAutoNameIndex{0};

    QString busName = busDef.value(QStringLiteral("name")).toString();
    if (busName.isEmpty())
        busName = QStringLiteral("Bus-%1").arg(busAutoNameIndex++);

    auto width = extractNumber<uint8_t>(busDef.value(QStringLiteral("width")));

    if (!width.has_value())
    {
        qWarning() << "Bus" << busName << "has no width";
        return false;
    }

    Bus* bus = new Bus(busName, static_cast<uint8_t>(width.value()), board);
    Q_UNUSED(bus);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return true;
}

bool createBusses(Board* board, const QJsonObject& doc)
{
    auto busListIt = doc.find(QLatin1String("busses"));
    if (busListIt == doc.end())
        return true; // its ok, the computer has still a cpu

    if (!busListIt.value().isArray())
        return false;

    QJsonArray busList = busListIt.value().toArray();
    for (const auto& busDef : busList)
    {
        if (!busDef.isObject())
        {
            qWarning() << "Invalid bus definition" << busDef;
            continue;
        }

        createBus(board, busDef.toObject());
    }

    return true;
}

bool createBusConnection(Device* device, Board* board, const QJsonObject& connDef)
{
    QString busName = connDef.value(QLatin1String("bus")).toString();
    auto busMask = extractNumber<uint64_t>(connDef.value(QLatin1String("bus_mask")));
    QString portName = connDef.value(QLatin1String("port")).toString();
    auto portMask = extractNumber<uint64_t>(connDef.value(QLatin1String("port_mask")));

    if (busName.isEmpty() || !busMask.has_value() || portName.isEmpty() || !portMask.has_value())
    {
        qWarning() << "Invalid bus connection definition";
        return false;
    }

    Bus* bus = board->bus(busName);
    if (!bus)
    {
        qWarning() << "No such bus" << busName;
        return false;
    }

    device->addBusConnection(portName, portMask.value(), bus, busMask.value());
    return true;
}

bool createBusConnections(Device* device, Board* board, const QJsonObject& deviceDef)
{
    auto connDefsIt = deviceDef.find(QLatin1String("connections"));
    if (connDefsIt == deviceDef.end())
        return true;

    if (!connDefsIt.value().isArray())
    {
        qWarning() << "Invalid connection definition for device" << device->name();
        return false;
    }

    QJsonArray connDefs = connDefsIt.value().toArray();
    for (const auto& def : connDefs)
    {
        if (!def.isObject())
        {
            qWarning() << "Invalid device definition" << def;
            continue;
        }

        createBusConnection(device, board, def.toObject());
    }

    return true;
}

LCD* newLCD(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    return new LCD(name, board);
}

Memory* newMemory(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    QString typeString = deviceDef.value(QStringLiteral("memory_type")).toString();
    Memory::Type type;
    if (typeString.compare(QLatin1String("rom"), Qt::CaseInsensitive) == 0)
        type = Memory::Type::ROM;
    else if (typeString.compare(QLatin1String("flash"), Qt::CaseInsensitive) == 0)
        type = Memory::Type::FLASH;
    else
        type = Memory::Type::RAM;

    auto size = extractNumber<int>(deviceDef.value(QStringLiteral("size")));
    if (!size.has_value() && size.value() < 0)
    {
        qWarning() << "Memory definition for" << name << "must contain a size";
        return nullptr;
    }

    return new Memory(type, static_cast<uint32_t>(size.value()), name, board);
}

VIA* newVIA(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    return new VIA(name, board);
}

bool createDevice(Board* board, const QJsonObject& deviceDef)
{
    static int deviceAutoNameIndex{0};

    QString deviceType = deviceDef.value(QStringLiteral("type")).toString();
    if (deviceType.isEmpty())
    {
        qWarning() << "Device definition without a type";
        return false;
    }

    QString deviceName = deviceDef.value(QStringLiteral("name")).toString();
    if (deviceName.isEmpty())
        deviceName = QStringLiteral("Device-%1").arg(deviceAutoNameIndex++);

    auto address = extractNumber<int>(deviceDef.value(QLatin1String("address")));
    if (!address.has_value())
    {
        qWarning() << "Device" << deviceName << "has no valid map address";
        return false;
    }
    if (address < 0)
        address = std::numeric_limits<uint16_t>::max();

    Device* device = nullptr;
    if (deviceType == QLatin1String("lcd"))
    {
        device = newLCD(deviceName, board, deviceDef);
    }
    else if (deviceType == QLatin1String("memory"))
    {
        device = newMemory(deviceName, board, deviceDef);
    }
    else if (deviceType == QLatin1String("via"))
    {
        device = newVIA(deviceName, board, deviceDef);
    }

    if (!device)
        return false;

    device->setMapAddressStart(static_cast<uint16_t>(address.value()));

    createBusConnections(device, board, deviceDef);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return true;
}

bool createDevices(Board* board, const QJsonObject& doc)
{
    auto devListIt = doc.find(QLatin1String("devices"));
    if (devListIt == doc.end())
        return true; // its ok, the computer has still a cpu

    if (!devListIt.value().isArray())
        return false;

    QJsonArray devList = devListIt.value().toArray();
    for (const auto& deviceDef : devList)
    {
        if (!deviceDef.isObject())
        {
            qWarning() << "Invalid device definition" << deviceDef;
            continue;
        }

        createDevice(board, deviceDef.toObject());
    }

    return true;
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

bool BoardLoader::load(QIODevice* input, Board* board)
{
    auto doc = openJsonDocument(input);
    if (doc.isNull() || !doc.isObject())
        return false;

    if (!createBusses(board, doc.object()))
        return false;

    if (!createDevices(board, doc.object()))
        return false;

    return true;
}

bool BoardLoader::validate(Board* board)
{
    if (!validateOverlapping(board))
        return false;
    return true;
}
