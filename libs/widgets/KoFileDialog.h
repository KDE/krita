/* This file is part of the KDE project
   Copyright (C) 2013 - 2014 Yue Liu <yue.liu@mail.com>

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

#ifndef KOFILEDIALOG_H
#define KOFILEDIALOG_H

#include "komain_export.h"

#include <KUrl>
#include <QFileDialog>
#include <QString>
#include <QUrl>
#include <QStringList>
#include <QList>

/**
 * Wrapper around QFileDialog providing native file dialogs
 * on KDE/Gnome/Windows/OSX/etc.
 */
class KOMAIN_EXPORT KoFileDialog
{
public:
    enum DialogType {
        OpenFile,
        OpenFiles,
        OpenDirectory,
        OpenDirectories,
        ImportFile,
        ImportFiles,
        ImportDirectory,
        ImportDirectories,
        SaveFile,
        SaveFiles
    };

    /**
     * @brief constructor
     * @param parent The parent of the file dialog
     * @param dialogType usage of the file dialog
     * @param caption Caption of the file dialog
     * @param defaultDir the default directory the file dialog shows. If uniqueName is provided, this will be replaced by the last open directory.
     * @param uniqueName the name for the file dialog. This will be used to open the filedialog in the last open location, instead the specified directory.
     *
     * @return The name of the entry user selected in the file dialog
     *
     */
    KoFileDialog(QWidget *parent = 0,
                 KoFileDialog::DialogType type = OpenFile,
                 const QString uniqueName = QString());

    ~KoFileDialog();

    void setCaption(const QString &caption);
    void setDefaultDir(const QString &defaultDir);
    void setNameFilter(const QString &filter);
    void setNameFilters(const QStringList &filterList,
                        const QString &defaultFilter = QString());
    void setMimeTypeFilters(const QStringList &filterList,
                            const QString &defaultFilter = QString());

    /**
     * @brief this function is for cases where detailed control is needed, e.g. evaulate dialog.exec()
     * @param returnType setup the dialog for selecting single file or multiple files
     *
     * @return the qfiledialog pointer, remember to delete it after no longer used!
     */
    QFileDialog* ptr();

    QStringList urls();
    QString url();

private:
    enum FilterType {
        MimeFilter,
        NameFilter
    };

    const QString getUsedDir(const QString &dialogName);
    void saveUsedDir(const QString &fileName, const QString &dialogName);

    const QStringList getFilterString(const QStringList &mimeList,
                                      bool withAllSupportedEntry = true);
    const QString getFilterString(const QString &defaultMime);

    class Private;
    Private * const d;
};

#endif /* KOFILEDIALOG_H */
