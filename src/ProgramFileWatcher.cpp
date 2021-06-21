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

#include "ProgramFileWatcher.h"

#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>

ProgramFileWatcher::ProgramFileWatcher(QObject *parent) :
    QObject(parent),
    fileSystemWatcher_{new QFileSystemWatcher(this)},
    rearmTimer_{new QTimer(this)},
    rearmFailedCounter_{0}
{
    connect(fileSystemWatcher_, &QFileSystemWatcher::fileChanged, this, &ProgramFileWatcher::onFileChanged);

    rearmTimer_->setInterval(100);
    connect(rearmTimer_, &QTimer::timeout, this, &ProgramFileWatcher::onTryRearm);
}

ProgramFileWatcher::~ProgramFileWatcher()
{
}

void ProgramFileWatcher::setFileName(const QString& fileName)
{
    if (fileName.isEmpty())
    {
        disableFileWatcher();
        fileInfo_.reset();
    }
    else
    {
        fileInfo_.reset(new QFileInfo(fileName));
        fileInfo_->setCaching(false);
        if (fileInfo_->exists())
            enableFileWatcher();
    }
}

void ProgramFileWatcher::onFileChanged(const QString& path)
{
    Q_ASSERT(fileInfo_);

    if (!fileSystemWatcher_->files().contains(path) || fileInfo_->size() == 0)
    {
        disableFileWatcher();
        startRearming();
        return;
    }

    emit programFileChanged();
}

void ProgramFileWatcher::onTryRearm()
{
    Q_ASSERT(fileInfo_);

    rearmFailedCounter_++;
    if (rearmFailedCounter_ > 10 * 1000 / rearmTimer_->interval())
    {
        stopRearming();
        return;
    }

    if (fileInfo_->exists() && fileInfo_->size() > 0)
    {
        stopRearming();

        emit programFileChanged();

        enableFileWatcher();
    }
}

void ProgramFileWatcher::enableFileWatcher()
{
    Q_ASSERT(fileInfo_);

    fileSystemWatcher_->addPath(fileInfo_->absoluteFilePath());
}

void ProgramFileWatcher::disableFileWatcher()
{
    if (fileInfo_)
        fileSystemWatcher_->removePath(fileInfo_->absoluteFilePath());
}

void ProgramFileWatcher::startRearming()
{
    rearmFailedCounter_ = 0;
    rearmTimer_->start();
}

void ProgramFileWatcher::stopRearming()
{
    rearmTimer_->stop();
}
