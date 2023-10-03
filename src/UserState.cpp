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

#include "UserState.h"

#include "views/View.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

const QString kGeometry = QStringLiteral("geometry");
const QString kVisible = QStringLiteral("visible");

} // namespace

UserState::UserState()
{
}

void UserState::setFileName(const QString& fileName)
{
    Q_ASSERT(!fileName.isEmpty());
    fileName_ = fileName;
    loadFile();
}

void UserState::loadViewState(View* view)
{
    const QJsonValue geometryValue = viewValue(view->name(), kGeometry);
    const auto geometry = QByteArray::fromBase64(geometryValue.toString().toUtf8());
    view->restoreGeometry(geometry);
}

void UserState::saveViewState(View* view)
{
    const QByteArray geometry = view->saveGeometry();
    const QString geometryString = QString::fromUtf8(geometry.toBase64());
    setViewValue(view->name(), kGeometry, geometryString);
}

bool UserState::viewVisible(const QString& viewName, bool def)
{
    return viewValue(viewName, kVisible).toBool(def);
}

void UserState::setViewVisible(const QString& viewName, bool visible)
{
    setViewValue(viewName, kVisible, visible);
}

QJsonValue UserState::viewValue(const QString& viewName, const QString& valueName)
{
    const QJsonObject obj = document_.object();
    const QJsonObject viewState = obj[viewName].toObject();
    return viewState[valueName];
}

void UserState::setViewValue(const QString& viewName, const QString& valueName, const QJsonValue& value)
{
    QJsonObject obj = document_.object();
    QJsonObject viewState = obj[viewName].toObject();
    viewState[valueName] = value;
    obj[viewName] = viewState;
    document_.setObject(obj);
    saveFile();
}

void UserState::loadFile()
{
    QFile file(fileName_);
    if (file.exists() && !file.open(QFile::ReadOnly))
    {
        qWarning() << "Could not open state file for reading" << fileName_;
    }

    QByteArray data;
    if (file.isOpen())
    {
        data = file.readAll();
    }

    document_ = QJsonDocument::fromJson(data);
    if (!document_.isObject())
        document_.setObject(QJsonObject());

    if (!file.exists())
    {
        qInfo() << "State does not exists. Create empty one";
        saveFile();
    }
}

void UserState::saveFile()
{
    if (fileName_.isEmpty())
        return;
    QFile file(fileName_);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        qWarning() << "Could not write state file" << fileName_;
        return;
    }
    file.write(document_.toJson());
}
