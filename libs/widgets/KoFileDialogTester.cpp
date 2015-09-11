/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2014
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoFileDialogTester.h"

#include "ui_KoFileDialogTester.h"

#include <QDesktopServices>
#include <QListWidget>
#include <QCheckBox>
#include <QRadioButton>

#include <KoFileDialog.h>

KoFileDialogTester::KoFileDialogTester(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KoFileDialogTester)
{
    ui->setupUi(this);

    connect(ui->bnOpenFile, SIGNAL(clicked()), SLOT(testOpenFile()));
    connect(ui->bnOpenFiles, SIGNAL(clicked()), SLOT(testOpenFiles()));
    connect(ui->bnOpenDirectory, SIGNAL(clicked()), SLOT(testOpenDirectory()));
    connect(ui->bnImportFile, SIGNAL(clicked()), SLOT(testImportFile()));
    connect(ui->bnImportFiles, SIGNAL(clicked()), SLOT(testImportFiles()));
    connect(ui->bnImportDirectory, SIGNAL(clicked()), SLOT(testImportDirectory()));
    connect(ui->bnSaveFile, SIGNAL(clicked()), SLOT(testSaveFile()));

    m_nameFilters << "Documents (*.odt *.doc *.txt)"
                << "Images (*.png *.jpg *.jpeg)"
                << "Presentations (*.ppt *.odp)"
                << "Patterns (*.pat *.jpg *.gif *.png *.tif *.xpm *.bmp)"
                << "Palettes (*.gpl *.pal *.act *.aco *.colors)";

    m_mimeFilter = QStringList()
            << "application/x-krita" << "image/x-exr" << "image/openraster" << "image/x-tga" << "image/vnd.adobe.photoshop"
            << "image/x-xcf" << "image/x-portable-pixmap" << "image/x-portable-graymap"
            << "image/x-portable-bitmap" << "image/png" << "image/jp2"
            << "image/tiff" << "application/vnd.oasis.opendocument.graphics"
            << "application/pdf" << "image/jpeg" << "image/bmp" << "image/x-xpixmap"
            << "image/gif" << "image/x-xbitmap" << "application/x-krita-flipbook"
            << "image/x-adobe-dng" << "image/x-xfig" << "image/svg+xml" << "image/svg+xml-compressed"
            << "image/x-eps" << "image/eps" << "application/eps" << "application/x-eps" << "application/postscript"
            << "image/x-wmf" << "application/x-karbon"
            << "image/tiff" << "application/vnd.oasis.opendocument.graphics"
            ;
}

KoFileDialogTester::~KoFileDialogTester()
{
    delete ui;
}



void KoFileDialogTester::testOpenFile()
{
    ui->listResults->clear();
    KoFileDialog dlg(this, KoFileDialog::OpenFile, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: OpenFile"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }
    }
    else {
        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QString url = dlg.filename();
    ui->listResults->addItem(url);
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());

}

void KoFileDialogTester::testOpenFiles()
{
    ui->listResults->clear();
    KoFileDialog dlg(this, KoFileDialog::OpenFiles, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: OpenFile"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

          if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }

    }
    else {
           if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QStringList urls = dlg.filenames();
    foreach(const QString &url, urls) {
        ui->listResults->addItem(url);
    }
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}

void KoFileDialogTester::testOpenDirectory()
{
    ui->listResults->clear();
    KoFileDialog dlg(this, KoFileDialog::OpenDirectory, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: OpenDirectory"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

         if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }

    }
    else {
         if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QString url = dlg.filename();
    ui->listResults->addItem(url);
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}


void KoFileDialogTester::testImportFile()
{
    ui->listResults->clear();

    KoFileDialog dlg(this, KoFileDialog::ImportFile, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: ImportFile"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

         if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }

    }
    else {

        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QString url = dlg.filename();
    ui->listResults->addItem(url);
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}

void KoFileDialogTester::testImportFiles()
{
    ui->listResults->clear();
    KoFileDialog dlg(this, KoFileDialog::ImportFiles, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: ImportFiles"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

         if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }

    }
    else {

        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QStringList urls = dlg.filenames(); foreach(const QString &url, urls) {
        ui->listResults->addItem(url);
    }
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}

void KoFileDialogTester::testImportDirectory()
{
    ui->listResults->clear();
    KoFileDialog dlg(this, KoFileDialog::ImportDirectory, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: Import Directory"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {

        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }

    }
    else {
            if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QString url = dlg.filename();
    ui->listResults->addItem(url);
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}

void KoFileDialogTester::testSaveFile()
{
    ui->listResults->clear();

    KoFileDialog dlg(this, KoFileDialog::SaveFile, ui->txtUniqueKey->text());
    dlg.setCaption(i18n("Testing: SaveFile"));
    dlg.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    if (ui->radioName->isChecked()) {
        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setNameFilters(m_nameFilters, m_nameFilters.last());
        }
        else {
            dlg.setNameFilters(m_nameFilters);
        }
    }
    else {

        if (ui->chkSetDefaultFilter->isChecked()) {
            dlg.setMimeTypeFilters(m_mimeFilter, m_mimeFilter[4]);
        }
        else {
            dlg.setMimeTypeFilters(m_mimeFilter);
        }

    }

    if (ui->chkHideNameFilterDetailsOption->isChecked()) {
        dlg.setHideNameFilterDetailsOption();
    }

    QString url = dlg.filename();
    ui->listResults->addItem(url);
    ui->lblMime->setText(dlg.selectedMimeType());
    ui->txtFilter->setText(dlg.selectedNameFilter());
}
