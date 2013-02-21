/* This file is part of the KDE project
   Copyright 2004 Nicolas GOUTTE <goutte@kde.org>
   
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

#ifndef __koStore_p_h_
#define __koStore_p_h_


#include "KoStore.h"

#include <QString>

#include <kurl.h>

class QWidget;

class KoStorePrivate
{
public:
    explicit KoStorePrivate(KoStore *qq)
        : q(qq),
        fileMode(Local),
        window(0),
        size(0),
        stream(0),
        isOpen(false),
        good(false),
        finalized(false)
    {
    }

    enum FileMode { /*Bad=0,*/ Local = 1, RemoteRead, RemoteWrite };

    /**
     * Conversion routine
     * @param _internalNaming name used internally : "root", "tar:/0", ...
     * @return the name used in the file, more user-friendly ("maindoc.xml",
     *         "part0/maindoc.xml", ...)
     * Examples:
     *
     * tar:/0 is saved as part0/maindoc.xml
     * tar:/0/1 is saved as part0/part1/maindoc.xml
     * tar:/0/1/pictures/picture0.png is saved as part0/part1/pictures/picture0.png
     *
     * see specification (calligra/lib/store/SPEC) for details.
     */
    QString toExternalNaming(const QString &internalNaming) const;

    /**
     *  Expands a full path name for a stream (directories+filename)
     */
    QString expandEncodedPath(const QString &intern) const;

    /**
     * Expands only directory names(!)
     * Needed for the path handling code, as we only operate on internal names
     */
    QString expandEncodedDirectory(const QString &intern) const;

    /**
     * Enter *one* single directory. Nothing like foo/bar/bleh allowed.
     * Performs some checking when in Read mode
     */
    bool enterDirectoryInternal(const QString &directory);

    bool extractFile(const QString &sourceName, QIODevice &buffer);

    KoStore *q;
    /**
     * original URL of the remote file
     * (undefined for a local file)
     */
    KUrl url;
    FileMode fileMode;
    QString localFileName;
    QWidget *window;

    KoStore::Mode mode;

    /// Store the filenames (with full path inside the archive) when writing, to avoid duplicates
    QStringList filesList;

    /// The "current directory" (path)
    QStringList currentPath;

    /// Current filename (between an open() and a close())
    QString fileName;
    /// Current size of the file named m_sName
    qint64 size;

    /// The stream for the current read or write operation
    QIODevice *stream;

    bool isOpen;
    /// Must be set by the constructor.
    bool good;
    bool finalized;

    QStack<QString> directoryStack;

    mutable enum {
        NamingVersion21,
        NamingVersion22,
        NamingVersionRaw  ///< Never expand file and directory names
    } namingVersion;
};

#endif
