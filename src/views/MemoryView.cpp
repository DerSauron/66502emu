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
#include <QIntValidator>
#include <QFileDialog>
#include <QSettings>

namespace {

const QString kSettingsLastAccesesdFilePath = QStringLiteral("LastAccesesdFilePath");

} // namespace

MemoryView::MemoryView(Memory* memory, MainWindow* parent) :
    DeviceView{memory, parent},
    ui(new Ui::MemoryView),
    pageAutomaticallyChanged_{}
{
    ui->setupUi(this);
    setup();
}

MemoryView::~MemoryView()
{
    delete ui;
}

void MemoryView::setup()
{
    ui->chipSelected->setBitCount(1);

    ui->page->setValue(0);
    ui->memoryPage->setMemory(memory());
    ui->memoryPage->setAddressOffset(memory()->mapAddressStart());
    ui->memoryPage->setPage(0);

    connect(memory(), &Memory::byteAccessed, this, &MemoryView::onMemoryAccessed);
    connect(memory(), &Memory::selectedChanged, this, &MemoryView::onMemorySelectedChanged);
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

void MemoryView::on_followButton_toggled(bool checked)
{
}

void MemoryView::on_loadButton_clicked()
{
    QSettings s;

    QString startPath = s.value(kSettingsLastAccesesdFilePath).toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Binary Image"),
                                                    startPath,
                                                    tr("Binary Image Files (*.img *.bin)"));

    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    s.setValue(kSettingsLastAccesesdFilePath, fi.absolutePath());

    loadFileToMemory(fileName);
}

void MemoryView::on_page_valueChanged(int value)
{
    ui->memoryPage->setPage(value);
    if (!pageAutomaticallyChanged_)
        ui->followButton->setChecked(false);
}

void MemoryView::loadFileToMemory(const QString& fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QByteArray fileData = file.readAll();
    for (int i = 0; i < qMin(fileData.size(), static_cast<int>(memory()->size())); ++i)
    {
        memory()->data()[i] = fileData[i];
    }
}
