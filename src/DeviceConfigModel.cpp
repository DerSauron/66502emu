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

#include "DeviceConfigModel.h"

DeviceConfigModel::DeviceConfigModel(QObject* parent) :
    QAbstractItemModel{parent}
{
}

QModelIndex DeviceConfigModel::index(int row, int column, const QModelIndex &parent) const
{
    return {};
}

QModelIndex DeviceConfigModel::parent(const QModelIndex &index) const
{
    return {};
}

int DeviceConfigModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return {};
}

int DeviceConfigModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return {};
}

QVariant DeviceConfigModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
