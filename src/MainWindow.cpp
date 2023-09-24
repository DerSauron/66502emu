/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle accurate emulator gui.
 *
 * 6502emu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with 6502emu. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutDialog.h"
#include "BoardLoader.h"
#include "UserState.h"
#include "board/Board.h"
#include "board/Clock.h"
#include "board/Debugger.h"
#include "board/Device.h"
#include "views/DeviceView.h"
#include "views/DisassemblerView.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>

namespace {

const QString kSettingsLastAccesesdFilePath = QStringLiteral("LastAccesesdFilePath");
const QString kSettingsLastLoadedBoardFileName = QStringLiteral("LastLoadedBoardFileName");
const QString kSettingsGeometry = QStringLiteral("MainWindow/Geometry");
const QString kSesstionsState = QStringLiteral("MainWindow/State");
const QList<QString> Units = {QLatin1String(""), // clazy:exclude=non-pod-global-static
                              QStringLiteral("k"),
                              QStringLiteral("M"),
                              QStringLiteral("G"),
                              QStringLiteral("T")};

template<typename T>
QString humanReadable(T number)
{
    auto num = static_cast<double>(number);
    int unit = 0;

    while (num >= 1000.0)
    {
        num /= 1000.0;
        unit++;
    }

    return QString::number(num, 'f', 1) + Units[unit];
}

} // namespace

ViewFactoryPointer extractViewFactory(QAction* action)
{
    if (!action->data().canConvert<ViewFactoryPointer>())
        return nullptr;

    auto viewFactory = action->data().value<ViewFactoryPointer>();
    Q_ASSERT(viewFactory);

    return viewFactory;
}

DeviceViewFactoryPointer extractDeviceViewFactory(QAction* action)
{
    return extractViewFactory(action).dynamicCast<DeviceViewFactory>();
}

MainWindow::MainWindow(Board* board, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    userState_(new UserState()),
    board_{board}
{
    ui->setupUi(this);
    setup();
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup()
{
    QIcon programIcon;
    programIcon.addFile(QStringLiteral(":/icons/icon-16.png"));
    programIcon.addFile(QStringLiteral(":/icons/icon-32.png"));
    programIcon.addFile(QStringLiteral(":/icons/icon.png"));
    setWindowIcon(programIcon);

    ui->addressBusView->setBus(board_->addressBus());
    ui->dataBusView->setBus(board_->dataBus());
    ui->clockView->setClock(board_->clock());
    ui->cpuView->setCPU(board_->cpu());
    ui->signalsView->setBoard(board_);

    statusMessage_ = new QLabel(ui->statusBar);
    ui->statusBar->addPermanentWidget(statusMessage_);

    connect(ui->actionManageBoard, &QAction::triggered, this, &MainWindow::onActionManageBoardTriggered);
    connect(ui->actionNewBoard, &QAction::triggered, this, &MainWindow::onActionNewBoardTriggered);
    connect(ui->actionOpenBoard, &QAction::triggered, this, &MainWindow::onActionOpenBoardTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::onActionQuitTriggered);

    connect(ui->stepInstructionButton, &QPushButton::clicked, board_->debugger(), &Debugger::stepInstruction);
    connect(ui->stepSubroutineButton, &QPushButton::clicked, board_->debugger(), &Debugger::stepSubroutine);

    connect(board_->clock(), &Clock::runningChanged, this, &MainWindow::onClockRunningChanged);
    connect(board_->clock(), &Clock::statsUpdatedClockCycles, this, &MainWindow::onStatsUpdatedClockCycles);

    ui->centralwidget->setEnabled(false);

    createBoardMenu();
    loadWindowState();
    maybeLoadBoard();
}

void MainWindow::maybeLoadBoard()
{
    QSettings s;

    QString lastBoardFileName = s.value(kSettingsLastLoadedBoardFileName).toString();
    QFileInfo fi(lastBoardFileName);
    if (fi.exists())
        loadBoard(lastBoardFileName);
}

void MainWindow::loadWindowState()
{
    QSettings s;

    restoreGeometry(s.value(kSettingsGeometry).toByteArray());
    restoreState(s.value(kSesstionsState).toByteArray());
}

void MainWindow::saveWindowState()
{
    QSettings s;

    s.setValue(kSettingsGeometry, saveGeometry());
    s.setValue(kSesstionsState, saveState());
}

void MainWindow::saveViewsVisibleState()
{
    foreachView([this](QAction* action, ViewFactory* factory)
    {
        userState_->setViewVisible(factory->viewName(), action->isChecked());
    });
}

void MainWindow::onCloseView()
{
    auto* view = qobject_cast<View*>(sender());
    foreachView([this, view](QAction* action, ViewFactory* factory)
    {
        if (factory->view() != view)
            return;
        factory->destroyView(this);
        action->setChecked(false);
    });
}

void MainWindow::destroyAllViews()
{
    foreachView([this](QAction* action, ViewFactory* factory)
    {
        if (!factory->view())
            return;
        factory->destroyView(this);
        action->setChecked(false);
    });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveViewsVisibleState();
    destroyAllViews();
    saveWindowState();

    QMainWindow::closeEvent(event);
}

void MainWindow::onBoardLoadingFinished(bool result)
{
    if (!result)
    {
        loadedFile_ = QString{};

        loadedBoardChanged();

        QMessageBox::warning(this, tr("Could not load file"), tr("Board file could not be loaded"));
        return;
    }

    loadedBoardChanged();

    if (auto* s = qobject_cast<BoardLoader*>(sender()))
        s->deleteLater();

    QString stateFileName = loadedFile_ + QLatin1String(".state");
    userState_->setFileName(stateFileName);

    QSettings s;
    s.setValue(kSettingsLastLoadedBoardFileName, loadedFile_);

    handleBoardLoaded();
}

void MainWindow::onBoardSavingFinished(bool result)
{
    // TODO implement
}

void MainWindow::createBoardMenu()
{
    ui->actionDisassemblyLog->setEnabled(false);
    ui->actionDisassemblyLog->setData(
                QVariant::fromValue(DisassemblerViewFactory::create(tr("Disassembly log"))));
    connect(ui->actionDisassemblyLog, &QAction::triggered, this, &MainWindow::onBoardViewAction);
}

void MainWindow::loadedBoardChanged()
{
    bool loaded = !loadedFile_.isEmpty();

    ui->actionManageBoard->setEnabled(loaded);
}

QAction* MainWindow::createDeviceViewAction(int index, Device* device, const ViewFactoryPointer& factory)
{
    const auto viewClassName = QString::fromUtf8(device->metaObject()->className());

    auto action = new QAction(ui->menuBoard);
    action->setCheckable(true);
    action->setText(tr("&%1 %2 (%3)").arg(index).arg(factory->viewName(), viewClassName));
    action->setShortcut(QKeySequence(tr("Alt+%1").arg(index)));
    action->setData(QVariant::fromValue(factory));
    connect(action, &QAction::triggered, this, &MainWindow::onBoardViewAction);
    ui->menuBoard->addAction(action);
    return action;
}

void MainWindow::rebuildBoardMenuActions()
{
    const QList<Device*> devices{board_->devices()};
    const QList<QAction*> actions = ui->menuBoard->actions();

    QHash<Device*, ViewFactoryPointer> factoryMap;
    for (auto action : actions)
    {
        auto factory = extractDeviceViewFactory(action);
        if (!factory)
            continue;
        factoryMap.insert(factory->device(), factory);
        action->deleteLater();
    }

    int i = 1;
    for (const auto device : devices)
    {
        ViewFactoryPointer viewFactory;
        if (auto pos = factoryMap.find(device); pos != factoryMap.end())
        {
            viewFactory = pos.value();
            factoryMap.erase(pos);
        }
        else
        {
            viewFactory = DeviceViewFactory::create(device);
        }

        auto action = createDeviceViewAction(i, device, viewFactory);
        auto show = userState_->viewVisible(viewFactory->viewName(), true);
        action->setChecked(show);
        i++;
    }

    for (auto& factory : factoryMap)
    {
        if (factory->view())
            factory->destroyView(this);
    }

    {
        auto viewFactory = extractViewFactory(ui->actionDisassemblyLog);
        Q_ASSERT(viewFactory);
        auto show = userState_->viewVisible(viewFactory->viewName(), false);
        ui->actionDisassemblyLog->setChecked(show);
    }

    ui->actionNoDevices->setVisible(devices.isEmpty());
}

void MainWindow::showEnabledViews()
{
    foreachView([this](QAction* action, ViewFactory* factory)
    {
        if (action->isChecked() && !factory->view())
            showView(factory);
    });
}

void MainWindow::showView(ViewFactory* factory)
{
    if (factory->view())
        return;
    auto view = factory->createView(this);
    Q_ASSERT(view);
    connect(view, &View::closingEvent, this, &MainWindow::onCloseView);
    view->show();
    activateWindow();
}

void MainWindow::hideView(ViewFactory* factory)
{
    if (!factory->view())
        return;
    factory->destroyView(this);
}

void MainWindow::loadBoard(const QString& fileName)
{
    saveViewsVisibleState();
    destroyAllViews();

    loadedFile_ = fileName;

    auto* file = new QFile{fileName};
    if (!file->open(QFile::ReadOnly))
    {
        qWarning() << "Cloud not open file" << fileName;
        onBoardLoadingFinished(false);
        return;
    }

    auto* loader = new BoardLoader{file, this};
    connect(loader, &BoardLoader::loadingFinished, this, &MainWindow::onBoardLoadingFinished);

    loader->load(board_);
}

void MainWindow::saveBoard()
{
    if (loadedFile_.isEmpty())
        return;

    auto* file = new QFile{loadedFile_};
    if (!file->open(QFile::WriteOnly))
    {
        qWarning() << "Cloud not open file" << loadedFile_;
        onBoardLoadingFinished(false);
        return;
    }

    auto* loader = new BoardLoader{file, this};
    connect(loader, &BoardLoader::savingFinished, this, &MainWindow::onBoardSavingFinished);

    loader->save(board_);
}

void MainWindow::onClockRunningChanged()
{
    ui->stepInstructionButton->setEnabled(!board_->clock()->isRunning());
    ui->stepSubroutineButton->setEnabled(!board_->clock()->isRunning());
}

void MainWindow::onBoardViewAction()
{
    auto action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    auto viewFactory = extractViewFactory(action);
    Q_ASSERT(viewFactory);

    if (action->isChecked())
        showView(viewFactory.get());
    else
        hideView(viewFactory.get());
}

void MainWindow::onStatsUpdatedClockCycles(uint32_t clockCycles)
{
    const QString message = tr("Running at %1Hz").arg(humanReadable(clockCycles));
    statusMessage_->setText(message);
}

void MainWindow::handleBoardLoaded()
{
    rebuildBoardMenuActions();
    showEnabledViews();

    QFileInfo fi(loadedFile_);
    setWindowTitle(tr("6502 emulator - %1").arg(fi.completeBaseName()));

    ui->actionDisassemblyLog->setEnabled(true);
    ui->centralwidget->setEnabled(true);
}

void MainWindow::foreachView(const std::function<void(QAction*, ViewFactory*)>& callback)
{
    auto actions = ui->menuBoard->actions();
    for (auto* action : actions)
    {
        auto factory = extractViewFactory(action);
        if (!factory)
            continue;
        callback(action, factory.get());
    }
}

bool MainWindow::warnOpenBoard()
{
    if (loadedFile_.isEmpty())
        return false;

    int result = QMessageBox::warning(this, tr("Board loaded"),
                                      tr("Close current board?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

     return result != QMessageBox::Yes;
}

void MainWindow::onActionManageBoardTriggered()
{
}

void MainWindow::onActionNewBoardTriggered()
{
    if (warnOpenBoard())
        return;

    QSettings s;
    QString lastDirectoryAccessed = s.value(kSettingsLastAccesesdFilePath).toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Select file for new board"), lastDirectoryAccessed,
                                                    tr("Board files (*.board)"));
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(QLatin1String(".board")))
        fileName += QLatin1String(".board");

    QFile file(fileName);
    if (file.exists())
    {
       int result = QMessageBox::warning(this, tr("File exists"),
                                         tr("Do you want to override the existing file"),
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::No);

        if (result != QMessageBox::Yes)
            return;
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::warning(this, tr("Could not open file"),
                             tr("The selected file coult no be open"));
        return;
    }
    file.write("{}");
    file.close();

    QFileInfo fi(fileName);
    s.setValue(kSettingsLastAccesesdFilePath, fi.absolutePath());

    loadBoard(fileName);
}

void MainWindow::onActionOpenBoardTriggered()
{
    if (warnOpenBoard())
        return;

    QSettings s;
    QString lastDirectoryAccessed = s.value(kSettingsLastAccesesdFilePath).toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select board file"), lastDirectoryAccessed,
                                                    tr("Board files (*.board)"));
    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if (!fi.exists())
        return;

    s.setValue(kSettingsLastAccesesdFilePath, fi.absolutePath());

    loadBoard(fileName);
}

void MainWindow::onActionAboutTriggered()
{
    AboutDialog* dialog = new AboutDialog(this);
    dialog->show();
}

void MainWindow::onActionQuitTriggered()
{
    QApplication::quit();
}
