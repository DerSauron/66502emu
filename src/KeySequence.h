#pragma once

class QKeyEvent;
class QString;

class KeySequence
{
public:
    static int toKeyCode(QKeyEvent* event);
    static QString toString(int keyCode);

private:
    KeySequence() = delete;
};
