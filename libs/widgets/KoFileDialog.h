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

#include "kowidgets_export.h"

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
class KOWIDGETS_EXPORT KoFileDialog : public QObject
{
    Q_OBJECT

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
     * @param uniqueName the name for the file dialog. This will be used to open
     * the filedialog in the last open location, instead the specified directory.
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
    void setHideNameFilterDetailsOption();

    QStringList urls();
    QString url();

private slots:

    void filterSelected(const QString &filter);

private:
    enum FilterType {
        MimeFilter,
        NameFilter
    };

    void createFileDialog();

    const QString getUsedDir(const QString &dialogName);
    void saveUsedDir(const QString &fileName, const QString &dialogName);

    const QStringList getFilterStringList(const QStringList &mimeList,
                                      bool withAllSupportedEntry = false);

    const QString getFilterString(const QStringList &mimeList,
                                  bool withAllSupportedEntry = false);

    const QString getFilterString(const QString &defaultMime);

    class Private;
    Private * const d;
};

#endif /* KOFILEDIALOG_H */
