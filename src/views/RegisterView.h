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

#include <QWidget>

namespace Ui {
class RegisterView;
}

class RegisterView : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterView(QWidget *parent = nullptr);
    ~RegisterView() override;

    void setName(const QString& name);
    void setBitCount(uint8_t bitCount);
    void setBitNames(const QStringList& names);
    void setEditableMask(uint64_t mask);
    void setHotkeysEnabled(bool enabled);

    uint64_t value() const;
    void setValue(uint64_t value);

signals:
    void valueChanged();

private:
    Ui::RegisterView* ui;
    uint8_t bitCount_;

    Q_DISABLE_COPY_MOVE(RegisterView)
};

template<typename... T>
QStringList makeBitNames(T... names)
{
    return {QLatin1String(names)...};
}
