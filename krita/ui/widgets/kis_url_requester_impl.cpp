/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_url_requester_impl.h"
#include "ui_kis_url_requester_impl.h"

#include <QDesktopServices>
#include <KoFileDialog.h>
#include <KisImportExportManager.h>
#include "KoIcon.h"

#include "kis_debug.h"



KisUrlRequesterImpl::KisUrlRequesterImpl(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::KisUrlRequesterImpl)
{
    ui->setupUi(this);

    ui->btnSelectFile->setIcon(koIcon("folder"));

    connect(ui->btnSelectFile, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(ui->txtFileName, SIGNAL(textChanged(const QString&)), SIGNAL(textChanged(const QString&)));
}

KisUrlRequesterImpl::~KisUrlRequesterImpl()
{
    delete ui;
}

void KisUrlRequesterImpl::setStartDir(const QString &path)
{
    m_basePath = path;
}

void KisUrlRequesterImpl::setFileName(const QString &path)
{
    ui->txtFileName->setText(path);
}

QString KisUrlRequesterImpl::fileName() const
{
    QString path = ui->txtFileName->text();
    return path;
}

KUrl KisUrlRequesterImpl::url() const
{
    return KUrl(fileName());
}

void KisUrlRequesterImpl::setUrl(const KUrl &urlObj)
{
    QString url = urlObj.path();

    if (m_basePath.isEmpty()) {
        ui->txtFileName->setText(url);
    }
    else {
        QDir d(m_basePath);
        ui->txtFileName->setText(d.relativeFilePath(url));
    }
}

void KisUrlRequesterImpl::slotSelectFile()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setCaption(i18n("Select a file to load..."));
    dialog.setDefaultDir(m_basePath.isEmpty() ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) : m_basePath);
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Import));
    QString url = dialog.url();

    if (m_basePath.isEmpty()) {
        ui->txtFileName->setText(url);
    }
    else {
        QDir d(m_basePath);
        ui->txtFileName->setText(d.relativeFilePath(url));
    }
}
