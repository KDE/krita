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

const QString KoFileDialog::getNameFilters(const QStringList &mimeFilter)
{
    QString filter;
    for (QStringList::ConstIterator it = mimeFilter.begin(); it != mimeFilter.end(); ++it) {
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

QString KoFileDialog::getOpenFileName(QWidget * parent, const QString & caption,
                                           const QString & dir, const QString &filter,
                                           QString *selectedFilter, QFileDialog::Options options)
{
    QFileDialog* dialog = new QFileDialog(parent, caption, dir, filter);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFile);
    if (selectedFilter)
        dialog->selectNameFilter(*selectedFilter);
    if (options)
        dialog->setOptions(options);
    dialog->open(this, SLOT(getFileName(QString)));

    return m_fileName;
}

QStringList KoFileDialog::getOpenFileNames(QWidget * parent, const QString & caption,
                                                const QString & dir, const QString &filter,
                                                QString *selectedFilter, QFileDialog::Options options)
{
    QFileDialog* dialog = new QFileDialog(parent, caption, dir, filter);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    if (selectedFilter)
        dialog->selectNameFilter(*selectedFilter);
    if (options)
        dialog->setOptions(options);
    dialog->open(this, SLOT(getFileNames(QStringList)));

    return m_fileNames;
}
QString KoFileDialog::getSaveFileName(QWidget * parent, const QString & caption,
                                           const QString & dir, const QString & filter,
                                           QString *selectedFilter, QFileDialog::Options options)
{
    QFileDialog* dialog = new QFileDialog(parent, caption, dir, filter);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    if (selectedFilter)
        dialog->selectNameFilter(*selectedFilter);
    if (options)
        dialog->setOptions(options);
    dialog->open(this, SLOT(getFileName(QString)));

    return m_fileName;
}

QString KoFileDialog::getExistingDirectory(QWidget * parent, const QString & caption,
                                                const QString & dir, QFileDialog::Options options)
{
    QFileDialog* dialog = new QFileDialog(parent, caption, dir);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::Directory);
    if (options)
        dialog->setOptions(options);
    else
        dialog->setOption(QFileDialog::ShowDirsOnly, true);
    dialog->open(this, SLOT(getFileName(QString)));

    return m_fileName;
}

void KoFileDialog::getFileName(QString fileName)
{
    m_fileName = fileName;
}

void KoFileDialog::getFileNames(QStringList fileNames)
{
    m_fileNames = fileNames;
}
