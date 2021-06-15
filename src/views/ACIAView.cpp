#include "ACIAView.h"
#include "ui_ACIAView.h"

ACIAView::ACIAView(ACIA* acia, MainWindow* parent) :
    DeviceView(acia, parent),
    ui(new Ui::ACIAView())
{
    ui->setupUi(this);
    setup();
}

ACIAView::~ACIAView()
{
    delete ui;
}

void ACIAView::setup()
{
    ui->chipSelect->setBitCount(1);
    ui->txFlag->setBitCount(1);
    ui->txFlag->setEnableColor(BitsView::EnabledColor::Red);
    ui->rxFlag->setBitCount(1);
    ui->rxFlag->setEnableColor(BitsView::EnabledColor::Red);

    connect(acia(), &ACIA::selectedChanged, this, &ACIAView::onChipSelectedChanged);
    connect(acia(), &ACIA::sendByte, this, &ACIAView::onSendByte);
    connect(acia(), &ACIA::transmittingChanged, this, &ACIAView::onTransmittingChanged);
    connect(acia(), &ACIA::receivingChanged, this, &ACIAView::onReceivingChanged);

    connect(ui->console, &Console::inputData, this, &ACIAView::onDataEntered);
}

void ACIAView::onChipSelectedChanged()
{
    ui->chipSelect->setValue(acia()->isSelected() ? 1 : 0);
}

void ACIAView::onDataEntered(const QByteArray& data)
{
    for (auto byte : data)
    {
        acia()->receiveByte(byte);
    }
}

void ACIAView::onSendByte(uint8_t byte)
{
    ui->console->outputData(QByteArray(1, byte));
}

void ACIAView::onTransmittingChanged()
{
    ui->txFlag->setValue(acia()->isTransmitting() ? 1 : 0);
}

void ACIAView::onReceivingChanged()
{
    ui->rxFlag->setValue(acia()->isReceiving() ? 1 : 0);
}
