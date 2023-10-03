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

#include "MemoryView.h"
#include "ui_MemoryView.h"

#include "LooseSignal.h"
#include "MainWindow.h"
#include "UserState.h"
#include "Program.h"
#include "ProgramLoader.h"
#include "ProgramFileWatcher.h"
#include "board/Board.h"
#include "board/Clock.h"
#include "views/SourcesView.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

namespace {

const QString kSettingsLastAccesesdFilePath = QStringLiteral("LastAccesesdFilePath");
const QString kLoadedProgram = QStringLiteral("loaded_programm");
const QString kAutoReload = QStringLiteral("auto_reload");

} // namespace

MemoryView::MemoryView(Memory* memory, MainWindow* parent) :
    DeviceView{memory, parent},
    ui{new Ui::MemoryView{}},
    memory_{memory},
    pageAutomaticallyChanged_{},
    sourcesView_{nullptr},
    fileSystemWatcher_(new ProgramFileWatcher(this)),
    reloadInProgress_{false}
{
    ui->setupUi(this);
    setup();
}

MemoryView::~MemoryView()
{
    if (memory_->isPersistant())
    {
        rememberProgram();
        rememberShowSources();
    }

    mainWindow()->userState()->setViewValue(name(), kAutoReload, ui->autoReloadMode->currentIndex());

    if (sourcesView_)
        hideSources();

    delete ui;
}

void MemoryView::initialize()
{
    DeviceView::initialize();

    ui->autoReloadMode->setCurrentIndex(mainWindow()->userState()->viewValue(name(), kAutoReload).toInt());

    if (memory_->isPersistant())
    {
        maybeLoadProgram();
        maybeShowSources();
    }
}

void MemoryView::setup()
{
    ui->chipSelected->setBitCount(1);

    ui->page->setValue(0);
    ui->page->setMaximum(memory_->size() / 256 - 1);
    ui->memoryPage->setMemory(memory_);
    ui->memoryPage->setAddressOffset(memory_->mapAddressStart());
    ui->memoryPage->setPage(0);

    LooseSignal::connect(memory_, &Memory::accessed, this, &MemoryView::onMemoryAccessed);
    LooseSignal::connect(memory_, &Memory::selectedChanged, this, &MemoryView::onMemorySelectedChanged);

    connect(fileSystemWatcher_, &ProgramFileWatcher::programFileChanged, this, &MemoryView::onProgramFileChanged);
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

void MemoryView::onMemoryAccessed()
{
    int32_t address = memory_->lastAccessAddress();
    bool write = memory_->lastAccessWasWrite();

    int32_t startAddress = address & 0xFF00;
    int32_t page = startAddress >> 8;

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
    ui->chipSelected->setValue(memory_->isSelected());

    if (!memory_->isSelected())
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

void MemoryView::onProgramFileChanged()
{
    if (reloadInProgress_)
        return;

    reloadInProgress_ = true;

    if (ui->autoReloadMode->currentIndex() != 0)
    {
        qInfo() << "Start program reload" << programFileName_;

        mainWindow()->board()->clock()->stop();

        loadProgram(programFileName_);

        if (ui->autoReloadMode->currentIndex() == 2)
        {
            mainWindow()->board()->setResetLine(WireState::Low);
        }
    }

    reloadInProgress_ = false;
}

void MemoryView::onFollowButtonToggled(bool checked)
{
    // nothing to do
}

void MemoryView::onLoadButtonClicked()
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

void MemoryView::onPageValueChanged(int value)
{
    ui->memoryPage->setPage(static_cast<uint8_t>(value));
    if (!pageAutomaticallyChanged_)
        ui->followButton->setChecked(false);
}

void MemoryView::onShowSourcesButtonClicked()
{
    if (ui->showSourcesButton->isChecked())
        showSources();
    else
        hideSources();
}

void MemoryView::loadProgram(const QString& fileName)
{
    programFileName_ = fileName;

    fileSystemWatcher_->setFileName({});

    ProgramLoader loader;
    program_ = loader.loadProgram(programFileName_);

    if (program_.isNull())
        return;

    for (int i = 0; i < qMin(program_.binaryData().size(), memory_->size()); ++i)
    {
        memory_->data()[i] = static_cast<uint8_t>(program_.binaryData()[i]);
    }

    ui->showSourcesButton->setEnabled(program_.hasSources());
    if (program_.hasSources() && sourcesView_)
    {
        sourcesView_->setProgram(&program_);
    }

    fileSystemWatcher_->setFileName(programFileName_);
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

QString MemoryView::sourcesName()
{
    return name() + QLatin1String(" - Source");
}
