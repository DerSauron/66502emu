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

#include "BitsView.h"

#include <QPaintEvent>
#include <QPainter>

namespace {

constexpr int BLOCK_SIZE = 22;
constexpr int SPACE = 2;

inline bool bit(uint64_t value, uint8_t _bit)
{
     return (value >> _bit) & 1;
}

inline uint64_t toggleBit(uint64_t value, uint8_t _bit)
{
    return value ^ (1ULL << _bit);
}

} // namespace

BitsView::BitsView(QWidget* parent) :
    QWidget{parent},
    sizeHint_{},
    bitCount_{},
    value_{},
    editableMask_{},
    enabledColor_{EnabledColor::Green}
{
}

BitsView::~BitsView()
{
}

void BitsView::setBitCount(uint8_t bitCount)
{
    if (bitCount == bitCount_)
        return;

    bitCount_ = bitCount;

    calcSizeHint();

    update();
}

void BitsView::setBitNames(const QStringList& names)
{
    names_ = names;
}

void BitsView::setEditableMask(uint64_t editableMask)
{
    if (editableMask == editableMask_)
        return;

    editableMask_ = editableMask;

    update();
}

void BitsView::setEnableColor(BitsView::EnabledColor enabledColor)
{
    if (enabledColor == enabledColor_)
        return;

    enabledColor_ = enabledColor;

    update();
}

void BitsView::setValue(uint64_t value)
{
    if (value == value_)
        return;

    value_ = value;
    emit valueChanged();

    update();
}

QSize BitsView::sizeHint() const
{
    return sizeHint_;
}

QSize BitsView::minimumSizeHint() const
{
    return sizeHint();
}

void BitsView::paintEvent(QPaintEvent* event)
{
    static QBrush gray{QColor{0x88, 0x88, 0x88}};
    static QBrush red{QColor{0xDD, 0x22, 0x22}};
    static QBrush green{QColor{0x22, 0xDD, 0x22}};
    static QPen editablePen{QColor{0xDD, 0xDD, 0x22}};
    static QPen textPen{Qt::black};
    static QVector<QFont> fonts = {
        {QStringLiteral("Monospaced"), 10},
        {QStringLiteral("Monospaced"), 8},
        {QStringLiteral("Monospaced"), 6},
    };

    if (bitCount_ == 0)
        return;

    QPainter p{this};

    auto offset = blocksOffset();

    int x = offset.width();
    uint8_t i = bitCount_;
    do
    {
        --i;

        QRect bitBlock{x, offset.height(), BLOCK_SIZE, BLOCK_SIZE};

        QBrush* color = &gray;
        if (bit(value_, i))
        {
            switch (enabledColor_)
            {
                case EnabledColor::Green:
                    color = &green;
                    break;
                case EnabledColor::Red:
                    color = &red;
                    break;
            }
        }
        p.fillRect(bitBlock, *color);

        if (bit(editableMask_, i))
        {
            p.setPen(editablePen);
            p.drawRect(bitBlock);
        }

        if (names_.size() > i)
        {
            p.setFont(fonts[qMin(qMax(names_[i].length(), 1), fonts.size()) - 1]);
            p.setPen(textPen);
            p.drawText(bitBlock, Qt::AlignCenter, names_[i]);
        }

        x += BLOCK_SIZE + SPACE + ((i % 8 == 0) ? 2 * SPACE : 0);
    }
    while (i > 0);
}

void BitsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPoint clickPos = event->pos();

    auto offset = blocksOffset();

    int x = offset.width();
    uint8_t i = bitCount_;
    do
    {
        --i;

        QRect bitBlock{x, offset.height(), BLOCK_SIZE, BLOCK_SIZE};

        if (bitBlock.contains(clickPos))
        {
            if (bit(editableMask_, i))
            {
                setValue(toggleBit(value_, i));
                break;
            }
        }

        x += BLOCK_SIZE + SPACE + ((i % 8 == 0) ? 2 * SPACE : 0);
    }
    while (i > 0);
}

void BitsView::calcSizeHint()
{
    sizeHint_ = {
        bitCount_ * BLOCK_SIZE + (bitCount_ - 1) * SPACE + ((bitCount_ -1 ) / 8) * 2 * SPACE,
        BLOCK_SIZE};
}

QSize BitsView::blocksOffset()
{
    auto as = size();
    QSize sh = sizeHint();
    return {qMax(0, as.width() - sh.width()) / 2, qMax(0, as.height() - sh.height()) / 2};
}
