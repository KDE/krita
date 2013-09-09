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

#include "KoFileDialogHelper.h"
#include <QDebug>
#include <QFileDialog>
#include <kmimetype.h>

const QString KoFileDialogHelper::getFilterString(const QStringList &mimeList)
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

const QString KoFileDialogHelper::getFilterString(const QString &defaultMime)
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

    qDebug() << "selectedFilter: " + filter;
    return filter;
}

QStringList KoFileDialogHelper::getFileNames(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             const QStringList &mimeList,
                                             const QString &defaultMime,
                                             QFileDialog::AcceptMode aMode,
                                             QFileDialog::FileMode fMode)
{
    QFileDialog dialog(parent,
                       caption,
                       dir,
                       getFilterString(mimeList));
    dialog.setAcceptMode(aMode);
    dialog.setFileMode(fMode);

    if (!defaultMime.isEmpty()) {
        dialog.selectNameFilter(getFilterString(defaultMime));
    }
    if (fMode == QFileDialog::Directory) {
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
    }

    // osx sheet dialog style, open() is ideal, but this is easier
    dialog.setWindowModality(Qt::WindowModal);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.selectedFiles();
    return QStringList();
}

QString KoFileDialogHelper::getOpenFileName(QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QStringList &mimeList,
                                            const QString &defaultMime)
{
    QString str = getFilterString(defaultMime);
    return QFileDialog::getOpenFileName(parent,
                                        caption,
                                        dir,
                                        getFilterString(mimeList),
                                        &str);
}

QStringList KoFileDialogHelper::getOpenFileNames(QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 const QStringList &mimeList,
                                                 const QString &defaultMime)
{
    QString str = getFilterString(defaultMime);
    return QFileDialog::getOpenFileNames(parent,
                                         caption,
                                         dir,
                                         getFilterString(mimeList),
                                         &str);
}

QString KoFileDialogHelper::getOpenDirectory(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir)
{
    return QFileDialog::getExistingDirectory(parent, caption, dir);
}

QString KoFileDialogHelper::getImportFileName(QWidget *parent,
                                              const QString &caption,
                                              const QString &dir,
                                              const QStringList &mimeList,
                                              const QString &defaultMime)
{
    return getFileNames(parent,
                        caption,
                        dir,
                        mimeList,
                        defaultMime,
                        QFileDialog::AcceptOpen,
                        QFileDialog::ExistingFile).first();
}

QStringList KoFileDialogHelper::getImportFileNames(QWidget *parent,
                                                   const QString &caption,
                                                   const QString &dir,
                                                   const QStringList &mimeList,
                                                   const QString &defaultMime)
{
    return getFileNames(parent,
                        caption,
                        dir,
                        mimeList,
                        defaultMime,
                        QFileDialog::AcceptOpen,
                        QFileDialog::ExistingFiles);
}

QString KoFileDialogHelper::getImportDirectory(QWidget *parent,
                                               const QString &caption,
                                               const QString &dir)
{
    return getFileNames(parent,
                        caption,
                        dir,
                        QStringList(),
                        QString(),
                        QFileDialog::AcceptOpen,
                        QFileDialog::Directory).first();
}

QString KoFileDialogHelper::getSaveFileName(QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QStringList &mimeList,
                                            const QString &defaultMime)
{
    return getFileNames(parent,
                        caption,
                        dir,
                        mimeList,
                        defaultMime,
                        QFileDialog::AcceptSave,
                        QFileDialog::AnyFile).first();

}
