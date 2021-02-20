/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004 Nicolas GOUTTE <goutte@kde.org>
   
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __koStore_p_h_
#define __koStore_p_h_


#include "KoStore.h"

#include <QStringList>
#include <QStack>

#include <QUrl>

class QWidget;

class KoStorePrivate
{
public:
    explicit KoStorePrivate(KoStore *qq, KoStore::Mode _mode, bool _writeMimetype)
        : q(qq),
        window(0),
        mode(_mode),
        size(0),
        stream(0),
        isOpen(false),
        good(false),
        finalized(false),
        writeMimetype(_writeMimetype)
    {
    }

    /**
     * Conversion routine
     * @param internalNaming name used internally : "root", "tar:/0", ...
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
     * Enter *one* single directory. Nothing like foo/bar/bleh allowed.
     * Performs some checking when in Read mode
     */
    bool enterDirectoryInternal(const QString &directory);

    bool extractFile(const QString &sourceName, QIODevice &buffer);

    KoStore *q;

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

    bool writeMimetype; ///< true if the backend is allowed to create "mimetype" automatically.

    QString substituteThis;
    QString substituteWith;

    QUrl url;
};

#endif
