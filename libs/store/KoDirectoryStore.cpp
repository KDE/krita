/* This file is part of the KDE project
   Copyright (C) 2002, 2006 David Faure <faure@kde.org>

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

#include "KoDirectoryStore.h"
#include "KoStore_p.h"

#include <QFile>
#include <QDir>
#include <StoreDebug.h>

// HMMM... I used QFile and QDir.... but maybe this should be made network transparent?

KoDirectoryStore::KoDirectoryStore(const QString& path, Mode mode, bool writeMimetype)
    : KoStore(mode, writeMimetype)
    , m_basePath(path)
{
    //debugStore << "path:" << path


    //debugStore << "base path:" << m_basePath;

    init();
}

KoDirectoryStore::~KoDirectoryStore()
{
}

void KoDirectoryStore::init()
{
    Q_D(KoStore);

    if (!m_basePath.endsWith('/'))
        m_basePath += '/';
    m_currentPath = m_basePath;

    QDir dir(m_basePath);
    if (dir.exists()) {
        d->good = true;
        return;
    }
    // Dir doesn't exist. If reading -> error. If writing -> create.
    if (d->mode == Write && dir.mkpath(m_basePath)) {
        debugStore << "KoDirectoryStore::init Directory created:" << m_basePath;
        d->good = true;
    }
}

bool KoDirectoryStore::openReadOrWrite(const QString& name, QIODevice::OpenModeFlag iomode)
{
    Q_D(KoStore);
    //debugStore <<"KoDirectoryStore::openReadOrWrite m_currentPath=" << m_currentPath <<" name=" << name;
    int pos = name.lastIndexOf('/');
    if (pos != -1) { // there are subdirs in the name -> maybe need to create them, when writing
        pushDirectory(); // remember where we were
        enterAbsoluteDirectory(QString());
        //debugStore <<"KoDirectoryStore::openReadOrWrite entering" << name.left(pos);
        bool ret = enterDirectory(name.left(pos));
        popDirectory();
        if (!ret)
            return false;
    }
    d->stream = new QFile(m_basePath + name);
    if (!d->stream->open(iomode)) {
        delete d->stream;
        d->stream = 0;
        return false;
    }
    if (iomode == QIODevice::ReadOnly)
        d->size = d->stream->size();
    return true;
}

bool KoDirectoryStore::enterRelativeDirectory(const QString& dirName)
{
    QDir origDir(m_currentPath);
    m_currentPath += dirName;
    if (!m_currentPath.endsWith('/'))
        m_currentPath += '/';
    //debugStore <<"KoDirectoryStore::enterRelativeDirectory m_currentPath now" << m_currentPath;
    QDir newDir(m_currentPath);
    if (newDir.exists())
        return true;
    // Dir doesn't exist. If reading -> error. If writing -> create.
    if (mode() == Write && origDir.mkdir(dirName)) {
        debugStore << "Created" << dirName << " under" << origDir.absolutePath();
        return true;
    }
    return false;
}

bool KoDirectoryStore::enterAbsoluteDirectory(const QString& path)
{
    m_currentPath = m_basePath + path;
    //debugStore <<"KoDirectoryStore::enterAbsoluteDirectory" << m_currentPath;
    QDir newDir(m_currentPath);
    Q_ASSERT(newDir.exists());   // We've been there before, therefore it must exist.
    return newDir.exists();
}

bool KoDirectoryStore::fileExists(const QString& absPath) const
{
    debugStore << "KoDirectoryStore::fileExists" << m_basePath + absPath;
    return QFile::exists(m_basePath + absPath);
}
