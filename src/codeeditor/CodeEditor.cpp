/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "CodeEditor.h"

#include "Highlighter.h"
#include "LineNumberArea.h"
#include <QPainter>
#include <QTextBlock>

namespace ce {

namespace {

const QColor CurrentAddressLineColor = QColor(Qt::yellow).lighter(160); // clazy:exclude=non-pod-global-static
const QColor CurrentCursorLineColor = QColor(Qt::blue).lighter(160); // clazy:exclude=non-pod-global-static

} // namespace

CodeEditor::CodeEditor(QWidget* parent) :
    QPlainTextEdit(parent),
    lineNumberArea_(new LineNumberArea(this))
{
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::rebuildExtraSelections);

    setCenterOnScroll(true);

    QFont font;
    font.setFamily(QStringLiteral("Monospace"));
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    highlighter_ = new Highlighter(document());

    updateLineNumberAreaWidth(0);
    rebuildExtraSelections();

    int iconSize = fontMetrics().height();
    breakpointIcon_ = QPixmap(QStringLiteral(":/icons/breakpoint.png"))
            .scaled(iconSize, iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    arrowRightIcon_ = QPixmap(QStringLiteral(":/icons/line-marker.png"))
            .scaled(iconSize, iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::lineNumberAreaDoubleClickEvent(QMouseEvent* event)
{
    auto cursor = cursorForPosition(QPoint(0, event->pos().y()));
    if (cursor.isNull())
        return;
    auto block = cursor.block();
    emit lineNumberDoubleClicked(block.firstLineNumber());
}

void CodeEditor::lineNumberAreaWheelEvent(QWheelEvent* event)
{
    wheelEvent(event);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea_);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(100));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea_->width(), fontMetrics().height(), Qt::AlignRight, number);

            QTextCursor currentBlockCursor(document()->findBlockByNumber(blockNumber));

            if (breakpoints_.contains(blockNumber))
            {
                painter.drawPixmap(0, top, breakpointIcon_);
            }

            if (currentBlockCursor == currentAddressCursor_)
            {
                painter.drawPixmap(0, top, arrowRightIcon_);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int iconWidth = fontMetrics().height();

    int space = iconWidth + 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::setDocument(QTextDocument* document)
{
    QPlainTextEdit::setDocument(document);
    highlighter_ = new Highlighter(document);
}

void CodeEditor::addBreakpoint(int line)
{
    breakpoints_.insert(line);
    lineNumberArea_->repaint();
}

void CodeEditor::removeBreakpoint(int line)
{
    breakpoints_.remove(line);
    lineNumberArea_->repaint();
}

void CodeEditor::highlightCurrentAddressLine(int line)
{
    QTextCursor cursor{};
    QTextBlock block = document()->findBlockByLineNumber(line);
    if (block.isValid())
        cursor = QTextCursor(block);

    if (cursor == currentAddressCursor_)
        return;

    currentAddressCursor_ = cursor;
    setTextCursor(currentAddressCursor_);
    rebuildExtraSelections();
}

void CodeEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea_->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        lineNumberArea_->scroll(0, dy);
    else
        lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::rebuildExtraSelections()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!currentAddressCursor_.isNull())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(CurrentAddressLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = currentAddressCursor_;
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(CurrentCursorLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

} // namespace ce
