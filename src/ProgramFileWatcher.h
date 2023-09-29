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

#include <QObject>

class QFileInfo;
class QFileSystemWatcher;
class QTimer;

class ProgramFileWatcher : public QObject
{
    Q_OBJECT
public:
    ProgramFileWatcher(QObject *parent = nullptr);
    ~ProgramFileWatcher() override;

    void setFileName(const QString& fileName);

signals:
    void programFileChanged();

private slots:
    void onFileChanged(const QString& path);
    void onTryRearm();

private:
    void enableFileWatcher();
    void disableFileWatcher();
    void startRearming();
    void stopRearming();

private:
    QScopedPointer<QFileInfo> fileInfo_;
    QFileSystemWatcher* fileSystemWatcher_;
    QTimer* rearmTimer_;
    int rearmFailedCounter_;

    Q_DISABLE_COPY_MOVE(ProgramFileWatcher)
};
