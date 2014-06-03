/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
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
#ifndef RECENTFILEMANAGER_H
#define RECENTFILEMANAGER_H

#include <QObject>

#include "krita_sketch_export.h"

/**
 * @brief The RecentFileManager class keeps track of recent files
 */
class KRITA_SKETCH_EXPORT RecentFileManager : public QObject
{
    Q_OBJECT
public:
    explicit RecentFileManager(QObject *parent = 0);
    ~RecentFileManager();

    /// @return the size of the recent files list
    int size();

    /// @return the recent file at position index or an empty string
    QString recentFile(int index) const;

    /// @return the recent filename at position index or an empty string
    QString recentFileName(int index) const;

    /// @return the list of filenames without their paths
    QStringList recentFileNames() const;

    /// @return the list of recent files with their paths
    QStringList recentFiles() const;


Q_SIGNALS:

    void recentFilesListChanged();

public Q_SLOTS:


    /// add the given filename to the front of the list of recent filenames
    void addRecent(const QString &_url);

private:
    class Private;
    Private* d;

};

#endif // FILEMANAGER_H
