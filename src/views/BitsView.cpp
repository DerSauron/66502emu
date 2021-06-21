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

#include "BitsView.h"

#include "HotkeyDialog.h"
#include "KeySequence.h"
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>

namespace {

constexpr int BLOCK_SIZE = 22;
constexpr int SPACE = 2;

inline bool bitValue(uint64_t value, uint8_t _bit)
{
     return (value >> _bit) & 1;
}

inline uint64_t toggleBit(uint64_t value, uint8_t _bit)
{
    return value ^ (1ULL << _bit);
}

inline uint64_t setBit(uint64_t value, uint8_t _bit)
{
    return value | (1ULL << _bit);
}

inline uint64_t unsetBit(uint64_t value, uint8_t _bit)
{
    return value & ~(1ULL << _bit);
}

} // namespace

BitsView::BitsView(QWidget* parent) :
    QWidget{parent},
    sizeHint_{},
    bitCount_{},
    value_{},
    editableMask_{},
    enabledColor_{EnabledColor::Green},
    hotkeysEnabled_{false}
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &BitsView::customContextMenuRequested, this, &BitsView::onShowContextMenu);
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

void BitsView::setHotkeysEnabled(bool enabled)
{
    if (enabled == hotkeysEnabled_)
        return;

    hotkeysEnabled_ = enabled;

    if (hotkeysEnabled_)
        installEventFilters();
    else
        uninstallEventFilters();
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

bool BitsView::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int keyCode = KeySequence::toKeyCode(keyEvent);
        handleKeyPress(keyCode);
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        int keyCode = KeySequence::toKeyCode(keyEvent);
        handleKeyRelease(keyCode);
    }

    return false;
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
    static QFont superSmallFont(QStringLiteral("Monospaced"), 5);
    static QFontMetrics superSmallFontMetric(superSmallFont);

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
        if (bitValue(value_, i))
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

        if (bitValue(editableMask_, i))
        {
            p.setPen(editablePen);
            p.drawRect(bitBlock.adjusted(0, 0, -1, -1));
        }

        if (names_.size() > i)
        {
            p.setFont(fonts[qMin(qMax(names_[i].length(), 1), fonts.size()) - 1]);
            p.setPen(textPen);
            p.drawText(bitBlock, Qt::AlignCenter, names_[i]);
        }

        int kc = findKeyCode(i);
        if (kc != Qt::Key_unknown)
        {
            p.setFont(superSmallFont);
            p.setPen(textPen);
            p.drawText(bitBlock.adjusted(0, 0, -1, superSmallFontMetric.descent() - 1),
                       Qt::AlignRight | Qt::AlignBottom, QStringLiteral("X"));
        }

        x += BLOCK_SIZE + SPACE + ((i % 8 == 0) ? 2 * SPACE : 0);
    }
    while (i > 0);
}

void BitsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    int bitPos = findBit(event->pos());
    if (bitPos < 0)
        return;

    uint8_t b = static_cast<uint8_t>(bitPos);

    if (bitValue(editableMask_, b))
        setValue(toggleBit(value_, b));
}

void BitsView::onShowContextMenu(const QPoint& pos)
{
    int bit = findBit(pos);
    if (bit < 0)
        return;

    QMenu contextMenu;

    if (hotkeysEnabled_)
    {
        auto* a1 = new QAction(tr("Assign Hotkey"), &contextMenu);
        contextMenu.addAction(a1);
        connect(a1, &QAction::triggered, [this, bit]()
        {
            auto dlg = new HotkeyDialog(bit, findKeyCode(bit), this);
            connect(dlg, &QDialog::accepted, this, &BitsView::onHotkeyDialogAccepted);
            dlg->show();
        });
    }

    if (!contextMenu.actions().isEmpty())
        contextMenu.exec(mapToGlobal(pos));
}

void BitsView::onHotkeyDialogAccepted()
{
    auto* dlg = qobject_cast<HotkeyDialog*>(sender());
    Q_ASSERT(dlg);

    assignHotkey(dlg->bit(), dlg->keyCode());
}

void BitsView::installEventFilters()
{
    window()->installEventFilter(this);
}

void BitsView::uninstallEventFilters()
{
    window()->removeEventFilter(this);
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

int BitsView::findBit(const QPoint& pos)
{
    auto offset = blocksOffset();

    int x = offset.width();
    int i = bitCount_;
    do
    {
        --i;

        QRect bitBlock{x, offset.height(), BLOCK_SIZE, BLOCK_SIZE};

        if (bitBlock.contains(pos))
        {
            return i;
        }

        x += BLOCK_SIZE + SPACE + ((i % 8 == 0) ? 2 * SPACE : 0);
    }
    while (i > 0);

    return -1;
}

void BitsView::assignHotkey(int bit, int keyCode)
{
    int kc = findKeyCode(bit);
    if (kc != Qt::Key_unknown)
        keyMap_.remove(kc);
    if (keyCode != Qt::Key_unknown)
        keyMap_[keyCode] = bit;
    update();
}

void BitsView::handleKeyPress(int keyCode)
{
    auto it = keyMap_.find(keyCode);
    if (it == keyMap_.end())
        return;
    setValue(setBit(value_, static_cast<uint8_t>(it.value())));
}

void BitsView::handleKeyRelease(int keyCode)
{
    auto it = keyMap_.find(keyCode);
    if (it == keyMap_.end())
        return;
    setValue(unsetBit(value_, static_cast<uint8_t>(it.value())));
}

int BitsView::findKeyCode(int bit)
{
    return keyMap_.key(bit, Qt::Key_unknown);
}
