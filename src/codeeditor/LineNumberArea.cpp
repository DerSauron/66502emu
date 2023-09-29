/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#include "LineNumberArea.h"

#include "CodeEditor.h"

namespace ce {

LineNumberArea::LineNumberArea(CodeEditor* codeEditor) :
    QWidget{codeEditor},
    codeEditor_{codeEditor}
{
}

LineNumberArea::~LineNumberArea()
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor()->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    codeEditor()->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mouseDoubleClickEvent(QMouseEvent* event)
{
    codeEditor()->lineNumberAreaDoubleClickEvent(event);
}

void LineNumberArea::wheelEvent(QWheelEvent* event)
{
    codeEditor()->lineNumberAreaWheelEvent(event);
}

} // namespace ce
