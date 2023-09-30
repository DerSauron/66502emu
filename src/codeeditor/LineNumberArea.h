/*
 * Copyright (c) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This work is licensed under the terms of the MIT license.
 * For a copy, see LICENSE or <https://opensource.org/licenses/MIT>.
 */

#pragma once

#include <QWidget>

namespace ce {

class CodeEditor;

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(CodeEditor* codeEditor = nullptr);
    ~LineNumberArea() override;

    CodeEditor* codeEditor() const { return codeEditor_; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    CodeEditor* codeEditor_;

    Q_DISABLE_COPY_MOVE(LineNumberArea)
};

} // namespace ce
