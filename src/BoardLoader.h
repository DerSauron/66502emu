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

#include <BoardFile.h>
#include <QObject>

class Board;

class BoardLoader : public QObject
{
    Q_OBJECT

public:
    BoardLoader(BoardInfo& boardInfo, QObject* parent = {});

    void load(Board* board);
    void save(const Board* board);

signals:
    void loaded(bool result);
    void saved(bool result);

private:
    bool loadImpl(const BoardInfo& boardInfo, Board* board);
    bool saveImpl(BoardInfo& boardInfo, const Board* board);
    bool validate(Board* board);

private:
    BoardInfo& boardInfo_;
};
