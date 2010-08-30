/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOBOOKMARKMANAGER_H
#define KOBOOKMARKMANAGER_H

#include "kotext_export.h"

#include <QObject>
#include <QList>

class KoBookmark;
class KoBookmarkManagerPrivate;

/**
 * A manager for all bookmarks in a document. Every bookmark is identified by a unique name.
 * Note that only SinglePosition and StartBookmark bookmarks can be retrieved from this
 * manager. An end bookmark should be retrieved from it's parent (StartBookmark) using
 * KoBookmark::endBookmark()
 * This class also maintains a list of bookmark names so that it can be easily used to
 * show all available bookmark.
 */
class KOTEXT_EXPORT KoBookmarkManager : public QObject
{
    Q_OBJECT
public:
    /// constructor
    KoBookmarkManager();
    ~KoBookmarkManager();

    /// @return a bookmark with the specified name, or 0 if there is none
    KoBookmark *retrieveBookmark(const QString &name);

    /// @return a list of QString containing all bookmark names
    QList<QString> bookmarkNameList();

public slots:
    /**
     * Insert a new bookmark to this manager
     * @param name the name of the bookmark
     * @param bookmark the bookmark object to insert
     */
    void insert(const QString &name, KoBookmark *bookmark);

    /**
     * Remove a bookmark from this manager.
     * @param name the name of the bookmark to remove
     */
    void remove(const QString &name);

    /**
     * Rename a bookmark
     * @param oldName the old name of the bookmark
     * @param newName the new name of the bookmark
     */
    void rename(const QString &oldName, const QString &newName);

private:
    KoBookmarkManagerPrivate * const d;
};

#endif
