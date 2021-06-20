#pragma once

#include <QObject>

class QFileInfo;
class QFileSystemWatcher;
class QTimer;

class ProgramFileWatcher : public QObject
{
    Q_OBJECT
public:
    ProgramFileWatcher(QObject *parent = nullptr);
    ~ProgramFileWatcher() override;

    void setFileName(const QString& fileName);

signals:
    void programFileChanged();

private slots:
    void onFileChanged(const QString& path);
    void onTryRearm();

private:
    void enableFileWatcher();
    void disableFileWatcher();
    void startRearming();
    void stopRearming();

private:
    QScopedPointer<QFileInfo> fileInfo_;
    QFileSystemWatcher* fileSystemWatcher_;
    QTimer* rearmTimer_;
    int rearmFailedCounter_;
};
