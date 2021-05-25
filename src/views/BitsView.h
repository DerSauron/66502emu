/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle correct emulator.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QStaticText>
#include <QWidget>

class BitsView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(uint64_t value READ value WRITE setValue NOTIFY valueChanged)

public:
    enum class EnabledColor
    {
        Green,
        Red
    };

public:
    BitsView(QWidget* parent = nullptr);
    ~BitsView() override;

    void setBitCount(uint8_t bitCount);
    void setBitNames(const QStringList& names);
    void setEditableMask(uint64_t editableMask);
    void setEnableColor(EnabledColor enabledColor);

    uint64_t value() const { return value_; }
    void setValue(uint64_t value);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void valueChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void calcSizeHint();
    QSize blocksOffset();

private:
    QSize sizeHint_;
    uint8_t bitCount_;
    QStringList names_;
    uint64_t value_;
    uint64_t editableMask_;
    EnabledColor enabledColor_;
};

