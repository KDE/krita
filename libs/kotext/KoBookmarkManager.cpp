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

#include "KoBookmarkManager.h"
#include "KoBookmark.h"

#include <KDebug>
#include <QHash>

class KoBookmarkManagerPrivate
{
public:
    KoBookmarkManagerPrivate() { }
    QHash<QString, KoBookmark*> bookmarkHash;
    QList<QString> bookmarkNameList;
    int lastId;
};

KoBookmarkManager::KoBookmarkManager()
        : d(new KoBookmarkManagerPrivate)
{
}

KoBookmarkManager::~KoBookmarkManager()
{
    delete d;
}

void KoBookmarkManager::insert(const QString &name, KoBookmark *bookmark)
{
    d->bookmarkHash[name] = bookmark;
    d->bookmarkNameList.append(name);
}

void KoBookmarkManager::remove(const QString &name)
{
    d->bookmarkHash.remove(name);
    d->bookmarkNameList.removeAt(d->bookmarkNameList.indexOf(name));
}

void KoBookmarkManager::rename(const QString &oldName, const QString &newName)
{
    QHash<QString, KoBookmark*>::iterator i = d->bookmarkHash.begin();

    while (i != d->bookmarkHash.end()) {
        if (i.key() == oldName) {
            KoBookmark *bookmark = d->bookmarkHash.take(i.key());
            bookmark->setName(newName);
            d->bookmarkHash.insert(newName, bookmark);
            int listPos = d->bookmarkNameList.indexOf(oldName);
            d->bookmarkNameList.replace(listPos, newName);
            return;
        }
        i++;
    }
}

KoBookmark *KoBookmarkManager::retrieveBookmark(const QString &name)
{
    KoBookmark *bookmark = d->bookmarkHash.value(name);
    return bookmark;
}

QList<QString> KoBookmarkManager::bookmarkNameList()
{
    return d->bookmarkNameList;
}

#include <KoBookmarkManager.moc>

