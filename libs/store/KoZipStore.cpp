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

#include "KoZipStore.h"
#include "KoStore_p.h"

#include <QBuffer>
#include <QByteArray>

#include <kzip.h>
#include <StoreDebug.h>

#include <QUrl>
#include <KoNetAccess.h>

KoZipStore::KoZipStore(const QString & _filename, Mode mode, const QByteArray & appIdentification,
                       bool writeMimetype)
  : KoStore(mode, writeMimetype)
{
    debugStore << "KoZipStore Constructor filename =" << _filename
    << " mode = " << int(mode)
    << " mimetype = " << appIdentification << endl;
    Q_D(KoStore);

    d->localFileName = _filename;

    m_pZip = new KZip(_filename);

    init(appIdentification);   // open the zip file and init some vars
}

KoZipStore::KoZipStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification,
                       bool writeMimetype)
  : KoStore(mode, writeMimetype)
{
    m_pZip = new KZip(dev);
    init(appIdentification);
}

KoZipStore::KoZipStore(QWidget* window, const QUrl &_url, const QString & _filename, Mode mode,
                       const QByteArray & appIdentification, bool writeMimetype)
  : KoStore(mode, writeMimetype)
{
    debugStore << "KoZipStore Constructor url" << _url.url(QUrl::PreferLocalFile)
    << " filename = " << _filename
    << " mode = " << int(mode)
    << " mimetype = " << appIdentification << endl;
    Q_D(KoStore);

    d->url = _url;
    d->window = window;

    if (mode == KoStore::Read) {
        d->fileMode = KoStorePrivate::RemoteRead;
        d->localFileName = _filename;

    } else {
        d->fileMode = KoStorePrivate::RemoteWrite;
        d->localFileName = QLatin1String("/tmp/kozip"); // ### FIXME with KTempFile
    }

    m_pZip = new KZip(d->localFileName);
    init(appIdentification);   // open the zip file and init some vars
}

KoZipStore::~KoZipStore()
{
    Q_D(KoStore);
    debugStore << "KoZipStore::~KoZipStore";
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

void KoZipStore::init(const QByteArray& appIdentification)
{
    Q_D(KoStore);

    m_currentDir = 0;
    d->good = m_pZip->open(d->mode == Write ? QIODevice::WriteOnly : QIODevice::ReadOnly);

    if (!d->good)
        return;

    if (d->mode == Write) {
        //debugStore <<"KoZipStore::init writing mimetype" << appIdentification;

        m_pZip->setCompression(KZip::NoCompression);
        m_pZip->setExtraField(KZip::NoExtraField);

        // Write identification
        if (d->writeMimetype) {
            (void)m_pZip->writeFile(QLatin1String("mimetype"), appIdentification);
        }

        m_pZip->setCompression(KZip::DeflateCompression);
        // We don't need the extra field in Calligra - so we leave it as "no extra field".
    } else {
        d->good = m_pZip->directory() != 0;
    }
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
    d->stream = 0; // Don't use!
    return m_pZip->prepareWriting(name, "", "" /*m_pZip->rootDir()->user(), m_pZip->rootDir()->group()*/, 0);
}

bool KoZipStore::openRead(const QString& name)
{
    Q_D(KoStore);
    const KArchiveEntry * entry = m_pZip->directory()->entry(name);
    if (entry == 0) {
        return false;
    }
    if (entry->isDirectory()) {
        warnStore << name << " is a directory !";
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
    //debugStore <<"KoZipStore::write" << _len;

    if (!d->isOpen) {
        errorStore << "KoStore: You must open before writing" << endl;
        return 0;
    }
    if (d->mode != Write) {
        errorStore << "KoStore: Can not write to store that is opened for reading" << endl;
        return 0;
    }

    d->size += _len;
    if (m_pZip->writeData(_data, _len))     // writeData returns a bool!
        return _len;
    return 0;
}

QStringList KoZipStore::directoryList() const
{
    QStringList retval;
    const KArchiveDirectory *directory = m_pZip->directory();
    foreach(const QString &name, directory->entries()) {
        const KArchiveEntry* fileArchiveEntry = m_pZip->directory()->entry(name);
        if (fileArchiveEntry->isDirectory()) {
            retval << name;
        }
    }
    return retval;
}

bool KoZipStore::closeWrite()
{
    Q_D(KoStore);
    debugStore << "Wrote file" << d->fileName << " into ZIP archive. size" << d->size;
    return m_pZip->finishWriting(d->size);
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
