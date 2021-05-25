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

#include "LCDCharPanel.h"
#include "utils/ArrayView.h"
#include <QPaintEvent>
#include <QPainter>

namespace {

constexpr int PixelSize = 2;
constexpr int CharWidth = 5;
constexpr int CharHeight = 8;
constexpr int DisplayWidth = 16;
constexpr int DisplayHeight = 2;
constexpr int PixelGap = 1;
constexpr int CharGap = 4;

constexpr int ClientCharWidth = CharWidth * PixelSize + (CharWidth - 1) * PixelGap;
constexpr int ClientCharHeight = CharHeight * PixelSize + (CharHeight - 1) * PixelGap;

constexpr int ClientDisplayWidth = DisplayWidth * ClientCharWidth + (DisplayWidth - 1) * CharGap;
constexpr int ClientDisplayHeight = DisplayHeight * ClientCharHeight + (DisplayHeight - 1) * CharGap;

constexpr int ClientHCharDistance = ClientCharWidth + CharGap;
constexpr int ClientVCharDistance = ClientCharHeight + CharGap;

const QBrush LidDotBrush{QColor{224, 239, 255}};
const QBrush DimDotBrush{QColor{73, 139, 215}};
const QBrush BackgroundBrush{QColor{93, 159, 235}};


QRect charRect(const QPoint& pos)
{
    return QRect{pos.x() * ClientHCharDistance, pos.y() * ClientVCharDistance,
                ClientHCharDistance, ClientVCharDistance};
}

} // namespace

int LCDCharPanel::width()
{
    return DisplayWidth;
}

int LCDCharPanel::height()
{
    return DisplayHeight;
}

bool LCDCharPanel::outOfView(const QPoint& panelPos)
{
    return panelPos.x() < 0 || panelPos.x() >= width() ||
            panelPos.y() < 0 || panelPos.y() >= height();
}

LCDCharPanel::LCDCharPanel(QWidget *parent) :
    QWidget(parent),
    data_(DisplayWidth * DisplayHeight * CharHeight)
{
}

LCDCharPanel::~LCDCharPanel()
{
}

QSize LCDCharPanel::sizeHint() const
{
    return QSize{ClientDisplayWidth, ClientDisplayHeight};
}

QSize LCDCharPanel::minimumSizeHint() const
{
    return sizeHint();
}

void LCDCharPanel::setCharacterData(const QPoint& pos, const ArrayView& data)
{
    updateCharacterData(pos, data);
    repaint(charRect(pos));
}

void LCDCharPanel::setCursorPos(const QPoint& cursorPos)
{
    if (cursorPos == cursorPos_)
        return;

    QPoint oldCursorPos = cursorPos_;
    cursorPos_ = cursorPos;

    if (!outOfView(oldCursorPos))
        repaint(charRect(oldCursorPos));
    if (!outOfView(cursorPos_))
        repaint(charRect(cursorPos_));
}

void LCDCharPanel::setCursorOn(bool cursorOn)
{
    QPoint pos = cursorPos_;
    if (cursorOn && pos.x() < 0)
        pos.setX(pos.x() + 1000);
    else if (pos.x() >= 0)
            pos.setX(pos.x() - 1000);
    setCursorPos(pos);
}

void LCDCharPanel::setDisplayOn(bool displayOn)
{

}

const QBrush& LCDCharPanel::backgroundBrush() const
{
    return BackgroundBrush;
}

void LCDCharPanel::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    const QRect& rect = event->rect();

    const int startX = rect.left() / ClientHCharDistance;
    const int endX = rect.right() / ClientHCharDistance;
    const int startY = rect.top() / ClientVCharDistance;
    const int endY = rect.bottom() / ClientVCharDistance;

    clearBackground(p, startX, startY, endX, endY);

    for (int y = startY; y <= qMin(DisplayHeight - 1, endY); y++)
    {
        for (int x = startX; x <= qMin(DisplayWidth - 1, endX); x++)
        {
            renderCharMatrix(p, x, y);
        }
    }
}

void LCDCharPanel::updateCharacterData(const QPoint& pos, const ArrayView& data)
{
    Q_ASSERT(pos.x() >= 0 && pos.x() < DisplayWidth);
    Q_ASSERT(pos.y() >= 0 && pos.y() < DisplayHeight);
    Q_ASSERT(data.size() == CharHeight);

    int i = 0;
    for (const auto& v : data)
    {
        data_[(pos.y() * DisplayWidth + pos.x()) * CharHeight + i] = v;
        i++;
    }
}

void LCDCharPanel::clearBackground(QPainter& p, int startX, int startY, int endX, int endY)
{
    const int clientX = startX * ClientHCharDistance;
    const int clientY = startY * ClientVCharDistance;
    const int clientW = (endX - startX + 1) * ClientHCharDistance;
    const int clientH = (endY - startY + 1) * ClientVCharDistance;
    p.fillRect(clientX, clientY, clientW, clientH, BackgroundBrush);
}

void LCDCharPanel::renderCharMatrix(QPainter& p, int charX, int charY)
{
    for (int y = 0; y < CharHeight; y++)
    {
        uint8_t rowValue;

        if (y == (CharHeight - 1) && charX == cursorPos_.x() && charY == cursorPos_.y())
            rowValue = 0xFF;
        else
            rowValue = data_[(charY * DisplayWidth + charX) * CharHeight + y];

        renderCharRow(p, charX, charY, y, rowValue);
    }
}

void LCDCharPanel::renderCharRow(QPainter& p, int charX, int charY, int row, uint8_t rowValue)
{
    for (int x = 0; x < CharWidth; x++)
    {
        const int drawX = (charX + 1) * ClientHCharDistance - CharGap + PixelGap - (x + 1) * (PixelSize + PixelGap);
        const int drawY = charY * ClientVCharDistance + row * (PixelSize + PixelGap);

        const QBrush* dotBrush = (rowValue & 1) ? &LidDotBrush : &DimDotBrush;
        p.fillRect(drawX, drawY, PixelSize, PixelSize, *dotBrush);

        rowValue >>= 1;
    }
}
