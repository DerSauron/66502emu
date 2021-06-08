#include "StartStopButton.h"

StartStopButton::StartStopButton(QWidget* parent) :
    QPushButton(parent)
{
}

StartStopButton::~StartStopButton()
{
}

void StartStopButton::showStartMode()
{
    setText(tr("Start"));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/start.png"), QSize(), QIcon::Normal, QIcon::Off);
    setIcon(icon);
}

void StartStopButton::showStopMode()
{
    setText(tr("Stop"));
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/stop.png"), QSize(), QIcon::Normal, QIcon::Off);
    setIcon(icon);
}
