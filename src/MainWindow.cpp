/*
 * Copyright (C) 2021 Daniel Volk <mail@volkarts.com>
 *
 * This file is part of 6502emu - 6502 cycle correct emulator.
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

#include "UserState.h"
#include "board/Board.h"
#include "board/Clock.h"
#include "board/Device.h"
#include "views/DeviceView.h"
#include "views/DisassemblerView.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

namespace {

const QString kSettingsLastAccesesdFilePath = QStringLiteral("LastAccesesdFilePath");
const QString kSettingsLastLoadedBoardFileName = QStringLiteral("LastLoadedBoardFileName");
const QString kSettingsGeometry = QStringLiteral("MainWindow/Geometry");
const QString kSesstionsState = QStringLiteral("MainWindow/State");

class DisassemblerViewFactory : public ViewFactory
{
public:
    static ViewFactoryPointer create(const QString& name)
    {
        return ViewFactoryPointer{new DisassemblerViewFactory(name)};
    }

private:
    DisassemblerViewFactory(const QString& name) :
        ViewFactory(name)
    {
    }

    View* createViewImpl(MainWindow* mainWindow) override
    {
        auto* view = new DisassemblerView(viewName_, mainWindow);
        view->connectBoard(mainWindow->board());
        return view;
    }
};

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
    board_{board}
{
    ui->setupUi(this);
    setup();
}

MainWindow::~MainWindow()
{
    closeAllViews(); // delete views when mainframe is still valid
    saveWindowState();
    delete ui;
}

void MainWindow::setup()
{
    connect(board_, &Board::deviceListChanged, this, &MainWindow::onBoardDevicesChanged);
    ui->addressBusView->setBus(board_->addressBus());
    ui->dataBusView->setBus(board_->dataBus());
    ui->clockView->setClock(board_->clock());
    ui->cpuView->setCPU(board_->cpu());
    ui->signalsView->setBoard(board_);

    connect(ui->singleInstructionButton, &QPushButton::clicked, board_, &Board::startSingleInstructionStep);
    connect(board_->clock(), &Clock::runningChanged, this, &MainWindow::onClockRunningChanged);

    ui->centralwidget->setEnabled(false);

    createBoardMenu();
    loadWindowState();
    maybeLoadBoard();

    adjustSize();
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

void MainWindow::closeView(View* view)
{
    auto actions = ui->menuBoard->actions();

    for (auto* a : actions)
    {
        auto factory = extractViewFactory(a);
        if (!factory)
            continue;

        if (factory->view() == view)
        {
            factory->destroyView(this);
            a->setChecked(false);
        }
    }
}

void MainWindow::closeAllViews()
{
    auto views = findChildren<View*>();
    for (const auto view : views)
    {
        closeView(view);
    }
}

void MainWindow::createBoardMenu()
{
    ui->actionDisassemblyLog->setEnabled(false);
    ui->actionDisassemblyLog->setData(
                QVariant::fromValue(DisassemblerViewFactory::create(tr("Disassembly log"))));
    connect(ui->actionDisassemblyLog, &QAction::triggered, this, &MainWindow::onBoardViewAction);
}

QAction* MainWindow::createDeviceViewAction(int index, Device* device, const ViewFactoryPointer& factory)
{
    const auto viewClassName = device->metaObject()->className();

    auto action = new QAction(ui->menuBoard);
    action->setCheckable(true);
    action->setText(tr("&%1 %2 (%3)").arg(index).arg(factory->viewName(), viewClassName));
    action->setShortcut(QKeySequence(tr("Alt+%1").arg(index)));
    action->setData(QVariant::fromValue(factory));
    connect(action, &QAction::triggered, this, &MainWindow::onBoardViewAction);
    ui->menuBoard->addAction(action);
    return action;
}

void MainWindow::rebuildBoardMenuDevices()
{
    const QList<Device*> devices{board_->devices()};

    QHash<Device*, ViewFactoryPointer> factoryMap;
    for (auto action : ui->menuBoard->actions())
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
        auto show = conditionallyShowView(viewFactory, true);
        action->setChecked(show);
        i++;
    }

    for (auto& factory : factoryMap)
    {
        if (factory->view())
            factory->destroyView(this);
    }

    if (auto viewFactory = extractViewFactory(ui->actionDisassemblyLog))
    {
        bool show = conditionallyShowView(viewFactory, false);
        ui->actionDisassemblyLog->setChecked(show);
    }

    ui->actionNoDevices->setVisible(devices.isEmpty());
}

bool MainWindow::conditionallyShowView(const ViewFactoryPointer& factory, bool defaultShow)
{
    if (!board_->userState()->viewVisible(factory->viewName(), defaultShow))
        return false;
    showView(factory);
    return true;
}

void MainWindow::showView(const ViewFactoryPointer& factory)
{
    if (factory->view())
        return;
    auto view = factory->createView(this);
    Q_ASSERT(view);
    view->show();
    activateWindow();
}

void MainWindow::loadBoard(const QString& fileName)
{
    if (!board_->load(fileName))
    {
        QMessageBox::warning(this, tr("Could not load file"), tr("Board file could not be loaded"));
        return;
    }

    QSettings s;
    s.setValue(kSettingsLastLoadedBoardFileName, fileName);

    loadedFile_ = fileName;

    handleBoardLoaded();
}

void MainWindow::onClockRunningChanged()
{
    ui->singleInstructionButton->setEnabled(!board_->clock()->isRunning());
}

void MainWindow::onBoardDevicesChanged()
{
    rebuildBoardMenuDevices();
}

void MainWindow::onBoardViewAction()
{
    auto action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    auto viewFactory = extractViewFactory(action);
    Q_ASSERT(viewFactory);

    if (action->isChecked())
        showView(viewFactory);
    else
        viewFactory->destroyView(this);
    board_->userState()->setViewVisible(viewFactory->viewName(), action->isChecked());
}

void MainWindow::handleBoardLoaded()
{
    closeAllViews();
    rebuildBoardMenuDevices();

    QFileInfo fi(loadedFile_);
    setWindowTitle(tr("6502 emulator - %1").arg(fi.completeBaseName()));

    ui->actionDisassemblyLog->setEnabled(true);
    ui->centralwidget->setEnabled(true);
}

void MainWindow::on_actionManageDevices_triggered()
{

}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionNewBoard_triggered()
{
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

    QFileInfo fi(fileName);
    s.setValue(kSettingsLastAccesesdFilePath, fi.absolutePath());

    loadBoard(fileName);
}

void MainWindow::on_actionOpenBoard_triggered()
{
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
