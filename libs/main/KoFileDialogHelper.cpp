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
#include <QApplication>
#include <QClipboard>

#include <kmimetype.h>
#include <klocale.h>

const QString KoFileDialogHelper::getFilterString(const QStringList &mimeList,
                                                  bool withAllSupported)
{
    QString allSupportedFilter;
    if (withAllSupported) {
        allSupportedFilter = QString(i18n("All supported formats") + " ( ");
    }
    QString restFilters;
    for (QStringList::ConstIterator it = mimeList.begin(); it != mimeList.end(); ++it) {
        KMimeType::Ptr type = KMimeType::mimeType( *it );
        if(!type)
            continue;
        restFilters.append(";;" + type->comment() + " ( ");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt) {
            restFilters.append(*jt + " ");
            if (withAllSupported) {
                allSupportedFilter.append(*jt + " ");
            }
        }
        restFilters.append(")");
    }

    if (withAllSupported) {
        return allSupportedFilter + ")" + restFilters;
    } else {
        return restFilters.remove(0, 2);
    }
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
                       dir);
    QString nameFilter;
    if (aMode == QFileDialog::AcceptSave) {
        nameFilter = getFilterString(mimeList, false);
    } else {
        nameFilter = getFilterString(mimeList, true);
    }
    dialog.setNameFilter(nameFilter);
    dialog.setAcceptMode(aMode);
    dialog.setFileMode(fMode);

    if (!defaultMime.isEmpty()) {
        dialog.selectNameFilter(getFilterString(defaultMime));
    }
    if (fMode == QFileDialog::Directory) {
        dialog.setOption(QFileDialog::ShowDirsOnly, true);
    }

    // open() is ideal, but this is easier
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
    QFileDialog::Options options = 0;
#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        options = QFileDialog::DontUseNativeDialog;
    }
#endif
    QString res = QFileDialog::getOpenFileName(parent,
                                               caption,
                                               dir,
                                               getFilterString(mimeList),
                                               &str,
                                               options);
    return res;
}

QStringList KoFileDialogHelper::getOpenFileNames(QWidget *parent,
                                                 const QString &caption,
                                                 const QString &dir,
                                                 const QStringList &mimeList,
                                                 const QString &defaultMime)
{
    QString str = getFilterString(defaultMime);
    QFileDialog::Options options = 0;
#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        options = QFileDialog::DontUseNativeDialog;
    }
#endif
    QStringList res = QFileDialog::getOpenFileNames(parent,
                                                    caption,
                                                    dir,
                                                    getFilterString(mimeList),
                                                    &str,
                                                    options);
    return res;
}

QString KoFileDialogHelper::getOpenDirectory(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir)
{
    QString res = QFileDialog::getExistingDirectory(parent, caption, dir);
    return res;
}

QString KoFileDialogHelper::getImportFileName(QWidget *parent,
                                              const QString &caption,
                                              const QString &dir,
                                              const QStringList &mimeList,
                                              const QString &defaultMime)
{
    QString res;
#ifdef Q_OS_MAC
    QStringList result =  getFileNames(parent,
                                       caption,
                                       dir,
                                       mimeList,
                                       defaultMime,
                                       QFileDialog::AcceptOpen,
                                       QFileDialog::ExistingFile);
    if (result.isEmpty())
        res = QString();
    else
        res = result.first();
#else
    QString str = getFilterString(defaultMime);
    QFileDialog::Options options = 0;
#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        options = QFileDialog::DontUseNativeDialog;
    }
#endif
    res = QFileDialog::getOpenFileName(parent,
                                       caption,
                                       dir,
                                       getFilterString(mimeList),
                                       &str,
                                       options);
#endif
    return res;
}

QStringList KoFileDialogHelper::getImportFileNames(QWidget *parent,
                                                   const QString &caption,
                                                   const QString &dir,
                                                   const QStringList &mimeList,
                                                   const QString &defaultMime)
{
#ifdef Q_OS_MAC
    QStringList res = getFileNames(parent,
                                   caption,
                                   dir,
                                   mimeList,
                                   defaultMime,
                                   QFileDialog::AcceptOpen,
                                   QFileDialog::ExistingFiles);
#else
    QString str = getFilterString(defaultMime);
    QFileDialog::Options options = 0;
#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        options = QFileDialog::DontUseNativeDialog;
    }
#endif
    QStringList res =QFileDialog::getOpenFileNames(parent,
                                                   caption,
                                                   dir,
                                                   getFilterString(mimeList),
                                                   &str,
                                                   options);
#endif
    return res;
}

QString KoFileDialogHelper::getImportDirectory(QWidget *parent,
                                               const QString &caption,
                                               const QString &dir)
{
    QString res;
#ifdef Q_OS_MAC
    QStringList result = getFileNames(parent,
                                      caption,
                                      dir,
                                      QStringList(),
                                      QString(),
                                      QFileDialog::AcceptOpen,
                                      QFileDialog::Directory);
    if (result.isEmpty())
        res = QString();
    else
        res = result.first();
#else
    res = QFileDialog::getExistingDirectory(parent, caption, dir);
#endif // Q_OS_MAC
    return res;
}

QString KoFileDialogHelper::getSaveFileName(QWidget *parent,
                                            const QString &caption,
                                            const QString &dir,
                                            const QStringList &mimeList,
                                            const QString &defaultMime)
{
    QString res;
#ifdef Q_OS_MAC
    QStringList result = getFileNames(parent,
                                      caption,
                                      dir,
                                      mimeList,
                                      defaultMime,
                                      QFileDialog::AcceptSave,
                                      QFileDialog::AnyFile);
    if (result.isEmpty())
        res = QString();
    else
        res = result.first();
#else
    QString str = getFilterString(defaultMime);
    QFileDialog::Options options = 0;
#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        options = QFileDialog::DontUseNativeDialog;
    }
#endif
    res = QFileDialog::getSaveFileName(parent,
                                       caption,
                                       dir,
                                       getFilterString(mimeList),
                                       &str,
                                       options);
#endif // Q_OS_MAC
    return res;
}
