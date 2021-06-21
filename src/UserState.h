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

#include <QJsonDocument>
#include <QString>

class Device;
class View;
class QJsonValue;

class UserState
{
public:
    UserState();

    void setFileName(const QString& fileName);

    void loadViewState(View* view);
    void saveViewState(View* view);

    bool viewVisible(const QString& viewName, bool def = false);
    void setViewVisible(const QString& viewName, bool visible);

    QJsonValue viewValue(const QString& viewName, const QString& valueName);
    void setViewValue(const QString& viewName, const QString& valueName, const QJsonValue& value);

private:
    void loadFile();
    void saveFile();

private:
    QString fileName_;
    QJsonDocument document_;
};
