#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_closeButton_clicked()
{
    close();
}
