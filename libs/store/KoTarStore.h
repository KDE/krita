/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

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

#ifndef koTarStore_h
#define koTarStore_h

#include "KoStore.h"
//Added by qt3to4:
#include <QByteArray>

class KTar;
class KArchiveDirectory;
class KUrl;

class KoTarStore : public KoStore
{
public:
    KoTarStore(const QString & _filename, Mode _mode, const QByteArray & appIdentification);
    KoTarStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification);
    /**
     * KUrl-constructor
     * @todo saving not completely implemented (fixed temporary file)
     */
    KoTarStore(QWidget* window, const KUrl& url, const QString & _filename, Mode _mode, const QByteArray & appIdentification);
    ~KoTarStore();
protected:
    virtual bool init(Mode _mode);
    virtual bool doFinalize();
    virtual bool openWrite(const QString& name);
    virtual bool openRead(const QString& name);
    virtual bool closeWrite();
    virtual bool closeRead() {
        return true;
    }
    virtual bool enterRelativeDirectory(const QString& dirName);
    virtual bool enterAbsoluteDirectory(const QString& path);
    virtual bool fileExists(const QString& absPath) const;

    static QByteArray completeMagic(const QByteArray& appMimetype);

    /// The tar archive
    KTar * m_pTar;

    /** In "Read" mode this pointer is pointing to the
    current directory in the archive to speed up the verification process */
    const KArchiveDirectory* m_currentDir;

    /// Buffer used when writing
    QByteArray m_byteArray;
private:
    Q_DECLARE_PRIVATE(KoStore)
};

#endif
