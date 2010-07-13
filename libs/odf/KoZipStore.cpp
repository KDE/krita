/* This file is part of the KDE project
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>

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

#include "KoZipStore.h"
#include "KoStore_p.h"

#include <QBuffer>
//Added by qt3to4:
#include <QByteArray>

#include <kzip.h>
#include <kdebug.h>

#include <kurl.h>
#include <kio/netaccess.h>

KoZipStore::KoZipStore(const QString & _filename, Mode _mode, const QByteArray & appIdentification)
{
    kDebug(30002) << "KoZipStore Constructor filename =" << _filename
    << " mode = " << int(_mode)
    << " mimetype = " << appIdentification << endl;
    Q_D(KoStore);

    d->localFileName = _filename;

    m_pZip = new KZip(_filename);

    d->good = init(_mode, appIdentification);   // open the zip file and init some vars
}

KoZipStore::KoZipStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification)
{
    Q_D(KoStore);
    m_pZip = new KZip(dev);
    d->good = init(mode, appIdentification);
}

KoZipStore::KoZipStore(QWidget* window, const KUrl & _url, const QString & _filename, Mode _mode, const QByteArray & appIdentification)
{
    kDebug(30002) << "KoZipStore Constructor url" << _url.pathOrUrl()
    << " filename = " << _filename
    << " mode = " << int(_mode)
    << " mimetype = " << appIdentification << endl;
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

    m_pZip = new KZip(d->localFileName);
    d->good = init(_mode, appIdentification);   // open the zip file and init some vars
}

KoZipStore::~KoZipStore()
{
    Q_D(KoStore);
    kDebug(30002) << "KoZipStore::~KoZipStore";
    if (!d->finalized)
        finalize(); // ### no error checking when the app forgot to call finalize itself
    delete m_pZip;

    // Now we have still some job to do for remote files.
    if (d->fileMode == KoStorePrivate::RemoteRead) {
        KIO::NetAccess::removeTempFile(d->localFileName);
    } else if (d->fileMode == KoStorePrivate::RemoteWrite) {
        KIO::NetAccess::upload(d->localFileName, d->url, d->window);
        // ### FIXME: delete temp file
    }
}

bool KoZipStore::init(Mode _mode, const QByteArray& appIdentification)
{
    KoStore::init(_mode);
    m_currentDir = 0;
    bool good = m_pZip->open(_mode == Write ? QIODevice::WriteOnly : QIODevice::ReadOnly);

    if (good && _mode == Read)
        good = m_pZip->directory() != 0;
    else if (good && _mode == Write) {
        //kDebug(30002) <<"KoZipStore::init writing mimetype" << appIdentification;

        m_pZip->setCompression(KZip::NoCompression);
        m_pZip->setExtraField(KZip::NoExtraField);
        // Write identification
        (void)m_pZip->writeFile("mimetype", "", "", appIdentification.data() , appIdentification.length());
        m_pZip->setCompression(KZip::DeflateCompression);
        // We don't need the extra field in KOffice - so we leave it as "no extra field".
    }
    return good;
}

void KoZipStore::setCompressionEnabled(bool e)
{
    if (e) {
        m_pZip->setCompression(KZip::DeflateCompression);
    } else {
        m_pZip->setCompression(KZip::NoCompression);        
    }
}

bool KoZipStore::doFinalize()
{
    return m_pZip->close();
}

bool KoZipStore::openWrite(const QString& name)
{
    Q_D(KoStore);
#if 0
    // Prepare memory buffer for writing
    m_byteArray.resize(0);
    d->stream = new QBuffer(m_byteArray);
    d->stream->open(QIODevice::WriteOnly);
    return true;
#endif
    d->stream = 0; // Don't use!
    return m_pZip->prepareWriting(name, "", "" /*m_pZip->rootDir()->user(), m_pZip->rootDir()->group()*/, 0);
}

bool KoZipStore::openRead(const QString& name)
{
    Q_D(KoStore);
    const KArchiveEntry * entry = m_pZip->directory()->entry(name);
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
    // Must cast to KZipFileEntry, not only KArchiveFile, because device() isn't virtual!
    const KZipFileEntry * f = static_cast<const KZipFileEntry *>(entry);
    delete d->stream;
    d->stream = f->createDevice();
    d->size = f->size();
    return true;
}

qint64 KoZipStore::write(const char* _data, qint64 _len)
{
    Q_D(KoStore);
    if (_len == 0) return 0;
    //kDebug(30002) <<"KoZipStore::write" << _len;

    if (!d->isOpen) {
        kError(30002) << "KoStore: You must open before writing" << endl;
        return 0;
    }
    if (d->mode != Write) {
        kError(30002) << "KoStore: Can not write to store that is opened for reading" << endl;
        return 0;
    }

    d->size += _len;
    if (m_pZip->writeData(_data, _len))     // writeData returns a bool!
        return _len;
    return 0;
}

bool KoZipStore::closeWrite()
{
    Q_D(KoStore);
    kDebug(30002) << "Wrote file" << d->fileName << " into ZIP archive. size" << d->size;
    return m_pZip->finishWriting(d->size);
#if 0
    if (!m_pZip->writeFile(d->fileName , "user", "group", d->size, m_byteArray.data()))
        kWarning(30002) << "Failed to write " << d->fileName;
    m_byteArray.resize(0);   // save memory
    return true;
#endif
}

bool KoZipStore::enterRelativeDirectory(const QString& dirName)
{
    Q_D(KoStore);
    if (d->mode == Read) {
        if (!m_currentDir) {
            m_currentDir = m_pZip->directory(); // initialize
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

bool KoZipStore::enterAbsoluteDirectory(const QString& path)
{
    if (path.isEmpty()) {
        m_currentDir = 0;
        return true;
    }
    m_currentDir = dynamic_cast<const KArchiveDirectory*>(m_pZip->directory()->entry(path));
    Q_ASSERT(m_currentDir);
    return m_currentDir != 0;
}

bool KoZipStore::fileExists(const QString& absPath) const
{
    const KArchiveEntry *entry = m_pZip->directory()->entry(absPath);
    return entry && entry->isFile();
}
