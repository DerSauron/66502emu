#include "Console.h"

#include <QKeyEvent>
#include <QScrollBar>

Console::Console(QWidget* parent) :
    QPlainTextEdit(parent)
{
    setup();
}

Console::~Console()
{
}

void Console::setScrollBackBuffer(int scrollBackBuffer)
{
    document()->setMaximumBlockCount(scrollBackBuffer);
}

void Console::setup()
{
    setScrollBackBuffer(1000);

    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::lightGray);
    setPalette(p);
}

void Console::setLocalEcho(bool localEcho)
{
    localEcho_ = localEcho;
}

void Console::outputData(const QByteArray& data)
{
    insertPlainText(QString::fromLocal8Bit(data));

    QScrollBar* scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void Console::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Backspace:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            break;

        default:
            if (localEcho_)
                QPlainTextEdit::keyPressEvent(event);
            emit inputData(event->text().toLocal8Bit());
            break;
    }
}

void Console::mousePressEvent(QMouseEvent* event)
{
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent* event)
{
}

void Console::contextMenuEvent(QContextMenuEvent* event)
{
}
