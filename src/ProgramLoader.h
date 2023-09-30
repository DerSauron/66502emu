/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <QObject>

class Program;

class ProgramLoader : public QObject
{
    Q_OBJECT

public:
    ProgramLoader(QObject* parent = {});
    ~ProgramLoader() override;

    Program loadProgram(const QString& fileName);

private:
    Program loadBinary(const QString& fileName);
    Program loadListing(const QString& fileName);

    Q_DISABLE_COPY_MOVE(ProgramLoader)
};
