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

#include "kis_url_requester.h"
#include "ui_wdg_url_requester.h"

#include <QDesktopServices>

#include <KisImportExportManager.h>
#include "KoIcon.h"

#include "kis_debug.h"

KisUrlRequester::KisUrlRequester(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::WdgUrlRequester)
    , m_mode(KoFileDialog::OpenFile)
{
    m_ui->setupUi(this);

    m_ui->btnSelectFile->setIcon(themedIcon("folder"));

    connect(m_ui->btnSelectFile, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(m_ui->txtFileName, SIGNAL(textChanged(const QString&)), SIGNAL(textChanged(const QString&)));
}

KisUrlRequester::~KisUrlRequester()
{
}

void KisUrlRequester::setStartDir(const QString &path)
{
    m_basePath = path;
}

void KisUrlRequester::setFileName(const QString &path)
{
    m_ui->txtFileName->setText(path);
    KUrl url(path);
    emit urlSelected(url);
}

QString KisUrlRequester::fileName() const
{
    return m_ui->txtFileName->text();
}

KUrl KisUrlRequester::url() const
{
    return KUrl(fileName());
}

void KisUrlRequester::setUrl(const KUrl &urlObj)
{
    QString url = urlObj.path();

    if (m_basePath.isEmpty()) {
        setFileName(url);
    }
    else {
        QDir d(m_basePath);
        setFileName(d.relativeFilePath(url));
    }
}

void KisUrlRequester::setMode(KoFileDialog::DialogType mode)
{
    m_mode = mode;
}

KoFileDialog::DialogType KisUrlRequester::mode() const
{
    return m_mode;
}

void KisUrlRequester::setMimeTypeFilters(const QStringList &filterList,
                            QString defaultFilter)
{
    m_mime_filter_list = filterList;
    m_mime_default_filter = defaultFilter;
}

void KisUrlRequester::setNameFilter(const QString& filter)
{
    m_nameFilter = filter;
}


void KisUrlRequester::slotSelectFile()
{
    KoFileDialog dialog(this, m_mode, "OpenDocument");
    if (m_mode == KoFileDialog::OpenFile)
    {
        dialog.setCaption(i18n("Select a file to load..."));
    }
    else if (m_mode == KoFileDialog::OpenDirectory)
    {
        dialog.setCaption(i18n("Select a directory to load..."));
    }

    dialog.setDefaultDir(m_basePath.isEmpty() ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) : m_basePath);

    if (m_mime_filter_list.isEmpty())
    {
        dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Import));
    }
    else
    {
        dialog.setMimeTypeFilters(m_mime_filter_list, m_mime_default_filter);
    }

    if (!m_nameFilter.isEmpty())
    {
        dialog.setNameFilter(m_nameFilter);
    }

    QString url = dialog.url();

    if (m_basePath.isEmpty())
    {
        setFileName(url);
    }
    else
    {
        QDir d(m_basePath);
        setFileName(d.relativeFilePath(url));
    }

}
