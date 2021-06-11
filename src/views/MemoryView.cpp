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

#include "MemoryView.h"
#include "ui_MemoryView.h"

#include "MainWindow.h"
#include "UserState.h"
#include "Program.h"
#include "ProgramLoader.h"
#include "board/Board.h"
#include "board/Clock.h"
#include "views/SourcesView.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QSettings>

namespace {

const QString kSettingsLastAccesesdFilePath = QStringLiteral("LastAccesesdFilePath");
const QString kLoadedProgram = QStringLiteral("loaded_programm");

} // namespace

MemoryView::MemoryView(Memory* memory, MainWindow* parent) :
    DeviceView{memory, parent},
    ui(new Ui::MemoryView),
    pageAutomaticallyChanged_{},
    sourcesView_{nullptr},
    fileSystemWatcher_(new QFileSystemWatcher(this)),
    reloadInProgress_{false}
{
    ui->setupUi(this);
    setup();
}

MemoryView::~MemoryView()
{
    if (memory()->isPersistant())
    {
        rememberProgram();
        rememberShowSources();
    }

    if (sourcesView_)
        hideSources();

    delete ui;
}

void MemoryView::initialize()
{
    DeviceView::initialize();

    if (memory()->isPersistant())
    {
        maybeLoadProgram();
        maybeShowSources();
    }
}

void MemoryView::setup()
{
    ui->chipSelected->setBitCount(1);

    ui->page->setValue(0);
    ui->page->setMaximum(memory()->size() / 256 - 1);
    ui->memoryPage->setMemory(memory());
    ui->memoryPage->setAddressOffset(memory()->mapAddressStart());
    ui->memoryPage->setPage(0);

    connect(memory(), &Memory::byteAccessed, this, &MemoryView::onMemoryAccessed);
    connect(memory(), &Memory::selectedChanged, this, &MemoryView::onMemorySelectedChanged);

    connect(fileSystemWatcher_, &QFileSystemWatcher::fileChanged, this, &MemoryView::onProgramFileChanged);
}

void MemoryView::maybeLoadProgram()
{
    auto userState = mainWindow()->userState();
    QString fileName = userState->viewValue(name(), kLoadedProgram).toString();
    if (fileName.isEmpty())
        return;

    loadProgram(fileName);
}

void MemoryView::maybeShowSources()
{
    auto userState = mainWindow()->userState();
    bool show = userState->viewVisible(sourcesName(), true) && program_.hasSources();
    ui->showSourcesButton->setEnabled(program_.hasSources());
    ui->showSourcesButton->setChecked(show);
    if (show)
        showSources();
}

void MemoryView::onMemoryAccessed(uint16_t address, bool write)
{
    uint16_t startAddress = address & 0xFF00;
    uint8_t page = startAddress >> 8;

    if (ui->followButton->isChecked())
    {
        pageAutomaticallyChanged_ = true;
        ui->memoryPage->setPage(page);
        ui->page->setValue(page);
        pageAutomaticallyChanged_ = false;
    }

    if (page == ui->memoryPage->page())
        ui->memoryPage->highlight(address & 0xFF, write);
    else
        ui->memoryPage->resetHighlight();
}

void MemoryView::onMemorySelectedChanged()
{
    ui->chipSelected->setValue(memory()->isSelected());

    if (!memory()->isSelected())
    {
        ui->memoryPage->resetHighlight();
    }
}

void MemoryView::onSourcesViewClosingEvent()
{
    auto* sourcesView = qobject_cast<SourcesView*>(sender());
    Q_ASSERT(sourcesView && sourcesView == sourcesView_);
    ui->showSourcesButton->setChecked(false);
    hideSources();
}

void MemoryView::onProgramFileChanged(const QString& path)
{
    if (reloadInProgress_)
        return;

    reloadInProgress_ = true;

    QFileInfo info(programFileName_);
    if (!info.exists())
        return;

    QMessageBox dialog;
    dialog.setWindowTitle(tr("File changed"));
    dialog.setIcon(QMessageBox::Question);
    dialog.setText(tr("The program file was changed outside the emulator"));
    dialog.setInformativeText(tr("Would yuour like to reload the program into memory?"));
    QPushButton* reloadBtn = dialog.addButton(tr("Reload"), QMessageBox::ActionRole);
    QPushButton* reloadAndResetBtn = dialog.addButton(tr("Reload and Reset CPU"), QMessageBox::ActionRole);
    dialog.addButton(tr("Abort"), QMessageBox::RejectRole);
    dialog.setDefaultButton(reloadAndResetBtn);

    dialog.exec();

    if (dialog.clickedButton() == reloadBtn || dialog.clickedButton() == reloadAndResetBtn)
    {
        mainWindow()->board()->clock()->stop();

        loadProgram(programFileName_);

        if (dialog.clickedButton() == reloadAndResetBtn)
        {
            mainWindow()->board()->setResetLine(WireState::Low);
        }
    }

    reloadInProgress_ = false;
}

void MemoryView::on_followButton_toggled(bool checked)
{
    // nothing to do
}

void MemoryView::on_loadButton_clicked()
{
    QSettings s;

    QString startPath = s.value(kSettingsLastAccesesdFilePath).toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Binary Image"),
                                                    startPath,
                                                    tr("Listing files (*.lst);;Binary Image Files (*.img *.bin)"));

    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    s.setValue(kSettingsLastAccesesdFilePath, fi.absolutePath());

    loadProgram(fileName);
}

void MemoryView::on_page_valueChanged(int value)
{
    ui->memoryPage->setPage(value);
    if (!pageAutomaticallyChanged_)
        ui->followButton->setChecked(false);
}

void MemoryView::on_showSourcesButton_clicked()
{
    if (ui->showSourcesButton->isChecked())
        showSources();
    else
        hideSources();
}

void MemoryView::loadProgram(const QString& fileName)
{
    disableFileWatcher();

    programFileName_ = fileName;

    ProgramLoader loader;
    program_ = loader.loadProgram(programFileName_);

    if (program_.isNull())
        return;

    for (int i = 0; i < qMin(program_.binaryData().size(), static_cast<int>(memory()->size())); ++i)
    {
        memory()->data()[i] = program_.binaryData()[i];
    }

    ui->showSourcesButton->setEnabled(program_.hasSources());
    if (program_.hasSources() && sourcesView_)
    {
        sourcesView_->setProgram(&program_);
    }

    enableFileWatcher();
}

void MemoryView::rememberProgram()
{
    auto userState = mainWindow()->userState();
    userState->setViewValue(name(), kLoadedProgram, programFileName_);
}

void MemoryView::rememberShowSources()
{
    auto userState = mainWindow()->userState();
    userState->setViewVisible(sourcesName(), !!sourcesView_);
}

void MemoryView::showSources()
{
    Q_ASSERT(!sourcesView_);
    Q_ASSERT(program_.hasSources());
    sourcesView_ = new SourcesView(sourcesName(), &program_, mainWindow(), this);
    sourcesView_->initialize();
    connect(sourcesView_, &SourcesView::closingEvent, this, &MemoryView::onSourcesViewClosingEvent);
    sourcesView_->show();
}

void MemoryView::hideSources()
{
    Q_ASSERT(sourcesView_);
    delete sourcesView_;
    sourcesView_ = nullptr;
}

void MemoryView::enableFileWatcher()
{
    QFileInfo info(programFileName_);
    fileSystemWatcher_->addPath(info.absoluteFilePath());
}

void MemoryView::disableFileWatcher()
{
    const auto list = fileSystemWatcher_->files();
    if (!list.isEmpty())
        fileSystemWatcher_->removePaths(list);
}

QString MemoryView::sourcesName()
{
    return name() + QLatin1String(" - Source");
}
