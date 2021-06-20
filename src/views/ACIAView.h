#pragma once

#include "DeviceView.h"
#include "board/ACIA.h"

namespace Ui {
class ACIAView;
}

class ACIAView : public DeviceView
{
    Q_OBJECT

public:
    ACIAView(ACIA* acia, MainWindow* parent);
    ~ACIAView() override;

    ACIA* acia() const { return static_cast<ACIA*>(device()); }

private:
    void setup();

private slots:
    void onChipSelectedChanged();
    void onDataEntered(const QByteArray& data);
    void onSendByte(uint8_t byte);
    void onTransmittingChanged();
    void onReceivingChanged();
    void onRegisterChanged();

private:
    Ui::ACIAView* ui;
};
