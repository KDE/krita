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

#ifndef koZipStore_h
#define koZipStore_h

#include "KoStore.h"
//Added by qt3to4:
#include <QByteArray>

class KZip;
class KArchiveDirectory;
class KUrl;

class KoZipStore : public KoStore
{
public:
    KoZipStore(const QString & _filename, Mode _mode, const QByteArray & appIdentification);
    KoZipStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification);
    /**
     * KUrl-constructor
     * @todo saving not completely implemented (fixed temporary file)
     */
    KoZipStore(QWidget* window, const KUrl& _url, const QString & _filename, Mode _mode, const QByteArray & appIdentification);
    ~KoZipStore();

    virtual void setCompressionEnabled(bool e);
    virtual qint64 write(const char* _data, qint64 _len);
protected:
    using KoStore::init;

    virtual bool init(Mode _mode, const QByteArray& appIdentification);
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

    /// The archive
    KZip * m_pZip;

    /** In "Read" mode this pointer is pointing to the
    current directory in the archive to speed up the verification process */
    const KArchiveDirectory* m_currentDir;
private:
    Q_DECLARE_PRIVATE(KoStore)
};

#endif
