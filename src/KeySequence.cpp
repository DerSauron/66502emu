#include "KeySequence.h"

#include <QKeyEvent>
#include <QString>

int KeySequence::toKeyCode(QKeyEvent* event)
{
    int keyCode = event->key();
    Qt::Key key = static_cast<Qt::Key>(keyCode);

    if (key == Qt::Key_unknown)
        return Qt::Key_unknown;

    if (key == Qt::Key_unknown ||
        key == Qt::Key_Control ||  key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta)
        return keyCode;

    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (modifiers & Qt::ShiftModifier)
        keyCode += Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        keyCode += Qt::CTRL;
    if (modifiers & Qt::AltModifier)
        keyCode += Qt::ALT;
    if (modifiers & Qt::MetaModifier)
        keyCode += Qt::META;

    return keyCode;
}

QString KeySequence::toString(int keyCode)
{
    return QKeySequence(keyCode).toString(QKeySequence::NativeText);
}
