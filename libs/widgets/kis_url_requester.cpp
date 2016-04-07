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

#include "KoIcon.h"

KisUrlRequester::KisUrlRequester(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::WdgUrlRequester)
    , m_mode(KoFileDialog::OpenFile)
{
    m_ui->setupUi(this);

    m_ui->btnSelectFile->setIcon(kisIcon("folder"));

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
    QString realPath = path;

    if (!m_basePath.isEmpty()) {
        QDir d(m_basePath);
        realPath = d.relativeFilePath(path);
    }

    m_ui->txtFileName->setText(realPath);
    QUrl url(realPath);
    emit urlSelected(url);
}

QString KisUrlRequester::fileName() const
{
    return m_ui->txtFileName->text();
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

    Q_ASSERT(!m_mime_filter_list.isEmpty());
    dialog.setMimeTypeFilters(m_mime_filter_list, m_mime_default_filter);

    setFileName(dialog.filename());
}
