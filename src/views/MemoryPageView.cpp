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

#include "MemoryPageView.h"

#include "board/Memory.h"
#include <QPainter>

namespace {

constexpr int HEADLINE_MARGIN = 2;

}

MemoryPageView::MemoryPageView(QWidget *parent) :
    QWidget(parent),
    font_{QStringLiteral("Monospaced"), 11},
    memory_{},
    page_{},
    addressOffset_{},
    highLightByte_{0xFFFF},
    highlightWrite_{false}
{
    QFontMetrics metric{font_};
    charWidth_ = metric.horizontalAdvance(QLatin1Char('0'));
    charHeight_ = metric.height();
    charAscent_ = metric.ascent();
}

MemoryPageView::~MemoryPageView()
{
}

QSize MemoryPageView::sizeHint() const
{
    return {charWidth_ * 16 * 2 + charWidth_ * 15, charHeight_ * 17 + HEADLINE_MARGIN};
}

QSize MemoryPageView::minimumSizeHint() const
{
    return sizeHint();
}

void MemoryPageView::setMemory(Memory* memory)
{
    if (memory == memory_)
        return;

    memory_ = memory;
    resetHighlight();

    update();
}

void MemoryPageView::setAddressOffset(u_int16_t addressOffset)
{
    if (addressOffset == addressOffset_)
        return;

    addressOffset_ = addressOffset;
    resetHighlight();

    update();
}

void MemoryPageView::setPage(uint8_t page)
{
    if (page == page_)
        return;

    Q_ASSERT(page < memory_->size() / 256);
    page_ = page;
    resetHighlight();

    update();
}

void MemoryPageView::highlight(uint8_t byte, bool write)
{
    if (byte == highLightByte_ && write == highlightWrite_)
        return;

    highLightByte_ = byte;
    highlightWrite_ = write;
    update();
}

void MemoryPageView::resetHighlight()
{
    highLightByte_ = 0xFFFF;
    update();
}

void MemoryPageView::paintEvent(QPaintEvent* event)
{
    static QBrush red{QColor{0xDD, 0x22, 0x22}};
    static QBrush green{QColor{0x22, 0xDD, 0x22}};
    static QBrush posBarBrush{QColor{0xCC, 0xCC, 0xCC}};

    QPainter p(this);

    p.setPen(Qt::black);
    p.setFont(font_);

    int dataViewOffset = HEADLINE_MARGIN + charHeight_;
    QSize clientSize = size();

    p.fillRect(QRect{0, dataViewOffset, clientSize.width(), clientSize.height() - dataViewOffset}, Qt::white);

    if (!memory_)
        return;

    int posBarWidth = qMax(5, clientSize.width() / (memory_->size() / 256));
    int posBarPos = posBarWidth * page_;

    p.fillRect(posBarPos, 0, posBarWidth, charHeight_, posBarBrush);

    uint16_t address = addressOffset_ + page_ * 0x100;
    p.drawText(0, charAscent_,
               QStringLiteral("Address: %2 - %3")
               .arg(address, 4, 16, QLatin1Char('0'))
               .arg(address + 0xFF, 4, 16, QLatin1Char('0')));

    for (int y = 0; y < 0x10; ++y)
    {
        for (int x = 0; x < 0x10; ++x)
        {
            uint16_t byte = y * 0x10 + x;
            uint16_t addr = page_ * 0x100 + byte;

            int rowPos = y * charHeight_;

            if (byte == highLightByte_)
            {
                p.fillRect(x * charWidth_ * 3, dataViewOffset + rowPos, charWidth_ * 2, charHeight_,
                           highlightWrite_ ? red : green);
            }

            p.drawText(x * charWidth_ * 3, dataViewOffset + rowPos + charAscent_,
                       QStringLiteral("%1").arg(memory_->byte(addr), 2, 16, QLatin1Char('0')));
        }
    }
}
