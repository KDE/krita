/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
