/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <QSet>
#include <QPlainTextEdit>

namespace ce {

class Highlighter;
class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget* parent = nullptr);
    ~CodeEditor() override;

    Highlighter* highlighter() const { return highlighter_; }

    void lineNumberAreaDoubleClickEvent(QMouseEvent* event);
    void lineNumberAreaWheelEvent(QWheelEvent* event);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setDocument(QTextDocument* document);

    bool hasBreakpoint(int line);
    void addBreakpoint(int line);
    void removeBreakpoint(int line);

    void highlightCurrentAddressLine(int line);

signals:
    void lineNumberDoubleClicked(int line);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void rebuildExtraSelections();

private:
    LineNumberArea* lineNumberArea_;
    Highlighter* highlighter_;
    QTextCursor currentAddressCursor_;
    QSet<int> breakpoints_;
    QPixmap breakpointIcon_;
    QPixmap arrowRightIcon_;

    Q_DISABLE_COPY_MOVE(CodeEditor)
};

} // namespace ce
