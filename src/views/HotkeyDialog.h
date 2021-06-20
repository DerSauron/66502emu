#pragma once

#include <QDialog>

namespace Ui {
class HotkeyDialog;
}

class HotkeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HotkeyDialog(int bit, int keyCode, QWidget *parent = nullptr);
    ~HotkeyDialog();

    int bit() const { return bit_; }
    int keyCode() const { return keyCode_; }

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void on_clearButton_clicked();

private:
    void setKeyCodeText();

private:
    Ui::HotkeyDialog *ui;
    int bit_;
    int keyCode_;
};
