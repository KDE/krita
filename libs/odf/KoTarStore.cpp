/* This file is part of the KDE project
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2010 C. Boemann <cbo@boemann.dk>

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

#include "KoTarStore.h"
#include "KoStore_p.h"

#include <QBuffer>
//Added by qt3to4:
#include <QByteArray>

#include <ktar.h>
#include <kdebug.h>
#include <kurl.h>

#include <kio/netaccess.h>

KoTarStore::KoTarStore(const QString & _filename, Mode _mode, const QByteArray & appIdentification)
{
    kDebug(30002) << "KoTarStore Constructor filename =" << _filename
    << " mode = " << int(_mode) << endl;
    Q_D(KoStore);

    d->localFileName = _filename;

    m_pTar = new KTar(_filename, "application/x-gzip");

    d->good = init(_mode);   // open the targz file and init some vars
    kDebug(30002) << "appIdentification :" << appIdentification;
    if (d->good && _mode == Write)
        m_pTar->setOrigFileName(completeMagic(appIdentification));
}

KoTarStore::KoTarStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification)
{
    Q_D(KoStore);
    m_pTar = new KTar(dev);

    d->good = init(mode);

    if (d->good && mode == Write)
        m_pTar->setOrigFileName(completeMagic(appIdentification));
}

KoTarStore::KoTarStore(QWidget* window, const KUrl& _url, const QString & _filename, Mode _mode, const QByteArray & appIdentification)
{
    kDebug(30002) << "KoTarStore Constructor url=" << _url.pathOrUrl()
    << " filename = " << _filename
    << " mode = " << int(_mode) << endl;
    Q_D(KoStore);

    d->url = _url;
    d->window = window;

    if (_mode == KoStore::Read) {
        d->fileMode = KoStorePrivate::RemoteRead;
        d->localFileName = _filename;

    } else {
        d->fileMode = KoStorePrivate::RemoteWrite;
        d->localFileName = "/tmp/kozip"; // ### FIXME with KTempFile
    }

    m_pTar = new KTar(d->localFileName, "application/x-gzip");

    d->good = init(_mode);   // open the targz file and init some vars

    if (d->good && _mode == Write)
        m_pTar->setOrigFileName(completeMagic(appIdentification));
}

KoTarStore::~KoTarStore()
{
    Q_D(KoStore);
    if (!d->finalized)
        finalize(); // ### no error checking when the app forgot to call finalize itself
    delete m_pTar;

    // Now we have still some job to do for remote files.
    if (d->fileMode == KoStorePrivate::RemoteRead) {
        KIO::NetAccess::removeTempFile(d->localFileName);
    } else if (d->fileMode == KoStorePrivate::RemoteWrite) {
        KIO::NetAccess::upload(d->localFileName, d->url, d->window);
        // ### FIXME: delete temp file
    }
}

QByteArray KoTarStore::completeMagic(const QByteArray& appMimetype)
{
    kDebug(30002) << "QCString KoTarStore::completeMagic( const QCString& appMimetype )********************";
    QByteArray res("Calligra ");
    res += appMimetype;
    res += '\004'; // Two magic bytes to make the identification
    res += '\006'; // more reliable (DF)
    kDebug(30002) << "sssssssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    kDebug(30002) << " return :!!!!!!!!!!!!!!! :" << res;
    return res;
}

bool KoTarStore::init(Mode _mode)
{
    KoStore::init(_mode);
    m_currentDir = 0;
    bool good = m_pTar->open(_mode == Write ? QIODevice::WriteOnly : QIODevice::ReadOnly);

    if (good && _mode == Read)
        good = m_pTar->directory() != 0;
    return good;
}

bool KoTarStore::doFinalize()
{
    return m_pTar->close();
}

// When reading, d->stream comes directly from KArchiveFile::device()
// When writing, d->stream buffers the data into m_byteArray

bool KoTarStore::openWrite(const QString& /*name*/)
{
    Q_D(KoStore);
    // Prepare memory buffer for writing
    m_byteArray.resize(0);
    d->stream = new QBuffer(&m_byteArray);
    d->stream->open(QIODevice::WriteOnly);
    return true;
}

bool KoTarStore::openRead(const QString& name)
{
    Q_D(KoStore);
    const KArchiveEntry * entry = m_pTar->directory()->entry(name);
    if (entry == 0) {
        //kWarning(30002) << "Unknown filename " << name;
        //return KIO::ERR_DOES_NOT_EXIST;
        return false;
    }
    if (entry->isDirectory()) {
        kWarning(30002) << name << " is a directory !";
        //return KIO::ERR_IS_DIRECTORY;
        return false;
    }
    KArchiveFile * f = (KArchiveFile *) entry;
    m_byteArray.resize(0);
    delete d->stream;
    d->stream = f->createDevice();
    d->size = f->size();
    return true;
}

bool KoTarStore::closeWrite()
{
    Q_D(KoStore);
    // write the whole bytearray at once into the tar file

    kDebug(30002) << "Writing file" << d->fileName << " into TAR archive. size" << d->size;
    if (!m_pTar->writeFile(d->fileName , "user", "group", m_byteArray.data(), d->size))
        kWarning(30002) << "Failed to write " << d->fileName;
    m_byteArray.resize(0);   // save memory
    return true;
}

bool KoTarStore::enterRelativeDirectory(const QString& dirName)
{
    Q_D(KoStore);
    if (d->mode == Read) {
        if (!m_currentDir) {
            m_currentDir = m_pTar->directory(); // initialize
            Q_ASSERT(d->currentPath.isEmpty());
        }
        const KArchiveEntry *entry = m_currentDir->entry(dirName);
        if (entry && entry->isDirectory()) {
            m_currentDir = dynamic_cast<const KArchiveDirectory*>(entry);
            return m_currentDir != 0;
        }
        return false;
    } else // Write, no checking here
        return true;
}

bool KoTarStore::enterAbsoluteDirectory(const QString& path)
{
    Q_D(KoStore);
    if (path.isEmpty()) {
        m_currentDir = 0;
        return true;
    }
    if (d->mode == Read) {
        m_currentDir = dynamic_cast<const KArchiveDirectory*>(m_pTar->directory()->entry(path));
        Q_ASSERT(m_currentDir);
        return m_currentDir != 0;
    } else
        return true;
}

bool KoTarStore::fileExists(const QString& absPath) const
{
    return m_pTar->directory()->entry(absPath) != 0;
}
