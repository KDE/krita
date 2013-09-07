/* This file is part of the KDE project
   Copyright (C) 2013 Yue Liu <yue.liu@mail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoFileDialog.h"
#include <QDebug>
#include <kmimetype.h>

const QString KoFileDialog::getFilterString(const QStringList &mimeList)
{
    QString filter;
    for (QStringList::ConstIterator it = mimeList.begin(); it != mimeList.end(); ++it) {
        KMimeType::Ptr type = KMimeType::mimeType( *it );
        if(!type)
            continue;
        filter.append(type->comment() + " (");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt)
            filter.append(*jt + " ");
        filter.append(");;");
    }
    filter.chop(2);

    qDebug() << "nameFilters: " + filter;
    return filter;
}

const QString KoFileDialog::getFilterString(const QString &defaultMime)
{
    QString filter;
    KMimeType::Ptr type = KMimeType::mimeType(defaultMime);
    if(type) {
        filter.append(type->comment() + " (");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt)
            filter.append(*jt + " ");
        filter.append(")");
    }

    qDebug() << "nameFilter: " + filter;
    return filter;
}

QFileDialog *KoFileDialog::initDialog(QWidget *parent,
                                      const QString &caption,
                                      const QString &dir,
                                      const QStringList &mimeList,
                                      const QString &defaultMime,
                                      QFileDialog::AcceptMode aMode,
                                      QFileDialog::FileMode fMode)
{
    QFileDialog *dialog = new QFileDialog(parent,
                                          caption,
                                          dir,
                                          getFilterString(mimeList));
    dialog->setAcceptMode(aMode);
    dialog->setFileMode(fMode);
    if (!defaultMime.isEmpty()) {
        m_selectedFilter = getFilterString(defaultMime);
        dialog->selectNameFilter(m_selectedFilter);
    }
    return dialog;
}

QString KoFileDialog::getOpenFileName(QWidget *parent,
                                      const QString &caption,
                                      const QString &dir,
                                      const QStringList &mimeList,
                                      const QString &defaultMime)
{
    m_selectedFilter = getFilterString(defaultMime);
    return QFileDialog::getOpenFileName(parent,
                                        caption,
                                        dir,
                                        getFilterString(mimeList),
                                        &m_selectedFilter);
}

QStringList KoFileDialog::getOpenFileNames(QWidget *parent,
                                           const QString &caption,
                                           const QString &dir,
                                           const QStringList &mimeList,
                                           const QString &defaultMime)
{
    m_selectedFilter = getFilterString(defaultMime);
    return QFileDialog::getOpenFileNames(parent,
                                         caption,
                                         dir,
                                         getFilterString(mimeList),
                                         &m_selectedFilter);
}

QString KoFileDialog::getOpenDirectory(QWidget *parent,
                                       const QString &caption,
                                       const QString &dir)
{
    return QFileDialog::getExistingDirectory(parent, caption, dir);
}

QString KoFileDialog::getImportFileName(QWidget *parent,
                                        const QString &caption,
                                        const QString &dir,
                                        const QStringList &mimeList,
                                        const QString &defaultMime)
{
    QFileDialog* dialog =
            initDialog(parent,
                       caption,
                       dir,
                       mimeList,
                       defaultMime,
                       QFileDialog::AcceptOpen,
                       QFileDialog::ExistingFile);

    dialog->open(this, SLOT(getFileName(QString)));
    return m_fileName;
}

QStringList KoFileDialog::getImportFileNames(QWidget *parent,
                                         const QString &caption,
                                         const QString &dir,
                                         const QStringList &mimeList,
                                         const QString &defaultMime)
{
    QFileDialog* dialog =
            initDialog(parent,
                       caption,
                       dir,
                       mimeList,
                       defaultMime,
                       QFileDialog::AcceptOpen,
                       QFileDialog::ExistingFiles);

    dialog->open(this, SLOT(getFileNames(QStringList)));
    return m_fileNames;
}

QString KoFileDialog::getImportDirectory(QWidget *parent,
                                         const QString &caption,
                                         const QString &dir)
{
    QFileDialog *dialog = new QFileDialog(parent, caption, dir);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly, true);
    dialog->open(this, SLOT(getFileName(QString)));
    return m_fileName;
}

QString KoFileDialog::getSaveFileName(QWidget *parent,
                                      const QString &caption,
                                      const QString &dir,
                                      const QStringList &mimeList,
                                      const QString &defaultMime)
{
    /*QFileDialog* dialog =
            initDialog(parent,
                       caption,
                       dir,
                       mimeList,
                       defaultMime,
                       QFileDialog::AcceptSave,
                       QFileDialog::AnyFile);
    dialog->open(this, SLOT(getFileName(QString))); // FIXME
    return m_fileName;*/
    m_selectedFilter = getFilterString(defaultMime);
    return QFileDialog::getSaveFileName(parent,
                                         caption,
                                         dir,
                                         getFilterString(mimeList),
                                         &m_selectedFilter);
}

QString KoFileDialog::getExportFileName(QWidget *parent,
                                        const QString &caption,
                                        const QString &dir,
                                        const QStringList &mimeList,
                                        const QString &defaultMime)
{
    m_selectedFilter = getFilterString(defaultMime);
    return QFileDialog::getSaveFileName(parent,
                                        caption,
                                        dir,
                                        getFilterString(mimeList),
                                        &m_selectedFilter);
}

void KoFileDialog::getFileName(QString fileName)
{
    qDebug() << fileName;
    m_fileName = fileName;
}

void KoFileDialog::getFileNames(QStringList fileNames)
{
    m_fileNames = fileNames;
}
