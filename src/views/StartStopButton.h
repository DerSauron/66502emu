#pragma once

#include <QPushButton>

class StartStopButton : public QPushButton
{
    Q_OBJECT

public:
    explicit StartStopButton(QWidget* parent = nullptr);
    ~StartStopButton() override;

    void showStartMode();
    void showStopMode();
};
