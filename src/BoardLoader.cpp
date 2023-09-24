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
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <optional>

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

template<typename T>
inline QJsonValue toJsonValue(T&& val)
{
    return QJsonValue::fromVariant(QVariant::fromValue(std::forward<T>(val)));
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

bool saveJsonDocument(QIODevice* output, const QJsonDocument& doc)
{
    auto rawData = doc.toJson(QJsonDocument::Indented);
    auto bytesWritten = output->write(rawData);
    if (bytesWritten != rawData.size())
    {
        qWarning() << "Failed to save to output";
        return false;
    }
    return true;
}

bool createBus(Board* board, const QJsonObject& busDef)
{
    static int busAutoNameIndex{0};

    QString busName = busDef.value(QStringLiteral("name")).toString();
    if (busName.isEmpty())
        busName = QStringLiteral("Bus-%1").arg(busAutoNameIndex++);

    if (busName == QLatin1String("ADDRESS") ||
            busName == QLatin1String("DATA"))
        return true;

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

QJsonObject saveBus(const Bus* bus)
{
    QJsonObject busObj;

    busObj.insert(QStringLiteral("name"), bus->name());
    busObj.insert(QStringLiteral("width"), toJsonValue(static_cast<int>(bus->width())));

    return busObj;
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

void saveBusses(QJsonObject& doc, const Board* board)
{
    QJsonArray busList;

    for (const auto& bus : board->busses())
    {
        busList << saveBus(bus);
    }

    doc.insert(QLatin1String("busses"), busList);
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

QJsonObject saveBusConnection(const Device* device, const BusConnection& busConnection)
{
    QJsonObject conObj;

    conObj.insert(QLatin1String("bus"), toJsonValue(busConnection.bus()->name()));
    conObj.insert(QLatin1String("bus_mask"), toJsonValue(busConnection.busMask()));
    conObj.insert(QLatin1String("port"), toJsonValue(device->portTagName(busConnection)));
    conObj.insert(QLatin1String("port_mask"), toJsonValue(busConnection.portMask()));

    return conObj;
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

void saveBusConnections(QJsonObject& doc, const Device* device)
{
    QJsonArray busList;

    for (const auto& con : device->busConnections())
    {
        busList << saveBusConnection(device, con);
    }

    doc.insert(QLatin1String("connections"), busList);
}

LCD* newLCD(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    return new LCD(name, board);
}

void saveLCD(const LCD* lcd, QJsonObject& obj)
{
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

void saveMemory(const Memory* memory, QJsonObject& obj)
{
    obj.insert(QLatin1String("size"), toJsonValue(memory->size()));

    QString type;
    switch (memory->type())
    {
        using enum Memory::Type;
        case ROM:
            type = QLatin1String("rom");
            break;
        case RAM:
            type = QLatin1String("ram");
            break;
        case FLASH:
            type = QLatin1String("flash");
            break;
    }
    obj.insert(QLatin1String("memory_type"), type);
}

VIA* newVIA(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    return new VIA(name, board);
}

void saveVIA(const VIA* via, QJsonObject& obj)
{
}

ACIA* newACIA(const QString& name, Board* board, const QJsonObject& deviceDef)
{
    return new ACIA(name, board);
}

void saveACIA(const ACIA* acia, QJsonObject& obj)
{
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
    else if (deviceType == QLatin1String("acia"))
    {
        device = newACIA(deviceName, board, deviceDef);
    }

    if (!device)
        return false;

    device->setMapAddressStart(static_cast<uint16_t>(address.value()));

    createBusConnections(device, board, deviceDef);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    return true;
}

QJsonObject saveDevice(const Device* device)
{
    QJsonObject devObj;

    devObj.insert(QLatin1String("name"), toJsonValue(device->name()));
    devObj.insert(QLatin1String("address"), toJsonValue(device->mapAddressStart()));

    QString type;

    if (const auto* lcd = qobject_cast<const LCD*>(device))
    {
        type = QLatin1String("lcd");
        saveLCD(lcd, devObj);
    }
    else if (const auto* memory = qobject_cast<const Memory*>(device))
    {
        type = QLatin1String("memory");
        saveMemory(memory, devObj);
    }
    else if (const auto* via = qobject_cast<const VIA*>(device))
    {
        type = QLatin1String("via");
        saveVIA(via, devObj);
    }
    else if (const auto* acia = qobject_cast<const ACIA*>(device))
    {
        type = QLatin1String("acia");
        saveACIA(acia, devObj);
    }

    saveBusConnections(devObj, device);

    return devObj;
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

void saveDevices(QJsonObject& doc, const Board* board)
{
    QJsonArray deviceList;

    for (const auto& device : board->devices())
    {
        deviceList << saveDevice(device);
    }

    doc.insert(QLatin1String("devices"), deviceList);
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


BoardLoader::BoardLoader(QIODevice* io, QObject* parent) :
    QObject{parent},
    io_{io}
{
    io_->setParent(this);
}

bool BoardLoader::load(Board* board)
{
    return QMetaObject::invokeMethod(this, "loadImpl", Q_ARG(Board*, board));
}

bool BoardLoader::save(Board* board)
{
    return QMetaObject::invokeMethod(this, "saveImpl", Q_ARG(Board*, board));
}

void BoardLoader::loadImpl(Board* board)
{
    bool result = false;

    board->clearDevices();

    auto doc = openJsonDocument(io_);
    if (!doc.isNull() && doc.isObject())
    {
        if (createBusses(board, doc.object()))
        {
            if (createDevices(board, doc.object()))
            {
                if (validate(board))
                {
                    result = true;
                }
                else
                {
                    qWarning() << "Board addresses overlap";
                }
            }
            else
            {
                qWarning() << "Could not create devices";
            }
        }
        else
        {
            qWarning() << "Could not create system busses";
        }
    }
    else
    {
        qWarning() << "File is no valid json document";
    }

    emit loadingFinished(result);
}

void BoardLoader::saveImpl(Board* board)
{
    QJsonObject doc;

    saveBusses(doc, board);
    saveDevices(doc, board);

    auto result = saveJsonDocument(io_, QJsonDocument{doc});

    emit savingFinished(result);
}

bool BoardLoader::validate(Board* board)
{
    return validateOverlapping(board);
}
