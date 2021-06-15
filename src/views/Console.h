#pragma once

#include <QPlainTextEdit>

class Console : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console(QWidget* parent = nullptr);
    ~Console() override;

public:
    void setScrollBackBuffer(int scrollBackBuffer);
    void setLocalEcho(bool localEcho);

public slots:
    void outputData(const QByteArray& data);

signals:
    void inputData(const QByteArray& data);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setup();

private:
    bool localEcho_{false};
};
