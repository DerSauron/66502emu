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

#include "BoardFile.h"

#include <glaze/glaze.hpp>
#include <QDebug>
#include <QFile>
#include <QThreadPool>

namespace glz::detail {

template<>
struct from_json<QString>
{
    template<auto Opts>
    static void op(QString& str, auto&&... args)
    {
        std::string buf;
        read<json>::op<Opts>(buf, args...);
        str = QString::fromStdString(buf);
    }
};

template <>
struct to_json<QString>
{
    template<auto Opts>
    static void op(const QString& str, auto&&... args) noexcept
    {
        auto buf = str.toStdString();
        write<json>::op<Opts>(buf, args...);
    }
};

} // namespace glz::detail

template <>
struct glz::meta<BusInfo>
{
   using T = BusInfo;
   static constexpr auto value = object(
               "name", &T::name,
               "width", &T::width
               );
};

template <>
struct glz::meta<BusConnectionInfo>
{
    using T = BusConnectionInfo;
    static constexpr auto value = object(
                "bus_name", &T::busName,
                "bus_mask", &T::busMask,
                "port_name", &T::portName,
                "port_mask", &T::portMask
                );
};

template <>
struct glz::meta<DeviceType>
{
   using enum DeviceType;
   static constexpr auto value = enumerate(
               "Memory", Memory,
               "VIA", VIA,
               "ACIA", ACIA,
               "LCD", LCD
                );
};

template <>
struct glz::meta<MemoryInfo>
{
   using T = MemoryInfo;
   static constexpr auto value = object(
               "type", &T::type,
               "name", &T::name,
               "address", &T::address,
               "connections", &T::connections,
               "memory_type", &T::memoryType,
               "memory_size", &T::memorySize
                );
};

template <>
struct glz::meta<Memory::Type>
{
   using enum Memory::Type;
   static constexpr auto value = enumerate(
               "ROM", ROM,
               "RAM", RAM,
               "Flash", FLASH
                );
};

template <>
struct glz::meta<ViaInfo>
{
    using T = ViaInfo;
    static constexpr auto value = object(
                "type", &T::type,
                "name", &T::name,
                "address", &T::address,
                "connections", &T::connections,
                "use_nmi", &T::useNMI
                );
};

template <>
struct glz::meta<AciaInfo>
{
    using T = AciaInfo;
    static constexpr auto value = object(
                "type", &T::type,
                "name", &T::name,
                "address", &T::address,
                "connections", &T::connections
                );
};

template <>
struct glz::meta<LcdInfo>
{
    using T = LcdInfo;
    static constexpr auto value = object(
                "type", &T::type,
                "name", &T::name,
                "address", &T::address,
                "connections", &T::connections
                );
};

template <>
struct glz::meta<DeviceInfo>
{
    static constexpr std::string_view tag = "type";
    static constexpr auto ids = DeviceTypeNames;
};

template <>
struct glz::meta<BoardInfo>
{
    using T = BoardInfo;
    static constexpr auto value = object(
                "busses", &T::busses,
                "devices", &T::devices
                );
};

BoardFile::BoardFile(QString fileName, QObject* parent) :
    QObject{parent},
    fileName_{std::move(fileName)}
{
}

void BoardFile::load()
{
    QThreadPool::globalInstance()->start([this]() -> void {
        bool result = false;

        if (QFile input{fileName_}; !input.open(QFile::ReadOnly))
        {
            qWarning() << "Could not open file" << fileName_;
        }
        else
        {
            QByteArray jsonData = input.readAll();
            if (jsonData.isEmpty())
            {
                qWarning() << "Failed to load from input" ;
            }
            else
            {
                std::string_view bufferView{
                    jsonData.constData(), static_cast<std::string_view::size_type>(jsonData.length())};
                BoardInfo boardInfo{};
                auto ec = glz::read_json(boardInfo, bufferView);
                if (ec)
                {
                    qWarning() << "Failed to parse input json:" <<
                                  QString::fromStdString(glz::format_error(ec, bufferView));
                }
                else
                {
                    boardInfo_ = boardInfo;
                    result = true;
                }
            }
        }

        emit loaded(result);
    });
}

void BoardFile::save()
{
    QThreadPool::globalInstance()->start([this]() -> void {
        bool result = false;

        std::string buffer{};
        constexpr glz::opts options{
            .format = glz::json,
            .prettify = true,
            .indentation_width = 4,
            .write_type_info = false, // we do this manually
        };
        glz::write<options>(boardInfo_, buffer);

        if (QFile output{fileName_}; !output.open(QFile::WriteOnly))
        {
            qWarning() << "Could not open file" << fileName_;
        }
        else
        {
            output.write(buffer.data(), static_cast<qint64>(buffer.size()));
            result = true;
        }

        emit saved(result);
    });
}
