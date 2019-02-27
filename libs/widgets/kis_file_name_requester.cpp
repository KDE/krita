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

#include "kis_file_name_requester.h"
#include "ui_wdg_file_name_requester.h"

#include <QStandardPaths>
#include <QDebug>

#include "KoIcon.h"
#include <KisFileUtils.h>

KisFileNameRequester::KisFileNameRequester(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::WdgFileNameRequester)
    , m_mode(KoFileDialog::OpenFile)
    , m_name("OpenDocument")
{
    m_ui->setupUi(this);

    m_ui->btnSelectFile->setIcon(kisIcon("folder"));

    connect(m_ui->btnSelectFile, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(m_ui->txtFileName, SIGNAL(textChanged(QString)), SIGNAL(textChanged(QString)));
}

KisFileNameRequester::~KisFileNameRequester()
{
}

void KisFileNameRequester::setStartDir(const QString &path)
{
    m_basePath = path;
}

void KisFileNameRequester::setConfigurationName(const QString &name)
{
    m_name = name;
}

void KisFileNameRequester::setFileName(const QString &path)
{
    m_ui->txtFileName->setText(path);
    emit fileSelected(path);
}

QString KisFileNameRequester::fileName() const
{
    return m_ui->txtFileName->text();
}

void KisFileNameRequester::setMode(KoFileDialog::DialogType mode)
{
    m_mode = mode;
}

KoFileDialog::DialogType KisFileNameRequester::mode() const
{
    return m_mode;
}

void KisFileNameRequester::setMimeTypeFilters(const QStringList &filterList, QString defaultFilter)
{
    m_mime_filter_list = filterList;
    m_mime_default_filter = defaultFilter;
}

void KisFileNameRequester::slotSelectFile()
{
    KoFileDialog dialog(this, m_mode, m_name);
    if (m_mode == KoFileDialog::OpenFile)
    {
        dialog.setCaption(i18n("Select a file to load..."));
    }
    else if (m_mode == KoFileDialog::OpenDirectory)
    {
        dialog.setCaption(i18n("Select a directory to load..."));
    }

    const QString basePath =
        KritaUtils::resolveAbsoluteFilePath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                            m_basePath);

    const QString filePath =
        KritaUtils::resolveAbsoluteFilePath(basePath, m_ui->txtFileName->text());

    dialog.setDefaultDir(filePath, true);
    dialog.setMimeTypeFilters(m_mime_filter_list, m_mime_default_filter);

    QString newFileName = dialog.filename();

    if (!newFileName.isEmpty()) {
        setFileName(newFileName);
    }
}
