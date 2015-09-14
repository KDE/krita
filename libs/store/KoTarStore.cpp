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

#include <QByteArray>

#include <ktar.h>
#include <StoreDebug.h>
#include <QUrl>

#include <KoNetAccess.h>

KoTarStore::KoTarStore(const QString & _filename, Mode mode, const QByteArray & appIdentification,
                       bool writeMimetype)
 : KoStore(mode, writeMimetype)
{
    debugStore << "KoTarStore Constructor filename =" << _filename
    << " mode = " << int(mode) << endl;
    Q_D(KoStore);

    d->localFileName = _filename;

    m_pTar = new KTar(_filename, "application/x-gzip");

    init(appIdentification);   // open the targz file and init some vars
}

KoTarStore::KoTarStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification,
                       bool writeMimetype)
 : KoStore(mode, writeMimetype)
{
    m_pTar = new KTar(dev);

    init(appIdentification);
}

KoTarStore::KoTarStore(QWidget* window, const QUrl &_url, const QString & _filename, Mode mode,
                       const QByteArray & appIdentification, bool writeMimetype)
 : KoStore(mode, writeMimetype)
{
    debugStore << "KoTarStore Constructor url=" << _url.url(QUrl::PreferLocalFile)
                  << " filename = " << _filename
                  << " mode = " << int(mode) << endl;
    Q_D(KoStore);

    d->url = _url;
    d->window = window;

    if (mode == KoStore::Read) {
        d->fileMode = KoStorePrivate::RemoteRead;
        d->localFileName = _filename;

    } else {
        d->fileMode = KoStorePrivate::RemoteWrite;
        d->localFileName = "/tmp/kozip"; // ### FIXME with KTempFile
    }

    m_pTar = new KTar(d->localFileName, "application/x-gzip");

    init(appIdentification);   // open the targz file and init some vars
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

QStringList KoTarStore::directoryList() const
{
    QStringList retval;
    const KArchiveDirectory *directory = m_pTar->directory();
    foreach(const QString &name, directory->entries()) {
        const KArchiveEntry* fileArchiveEntry = m_pTar->directory()->entry(name);
        if (fileArchiveEntry->isDirectory()) {
            retval << name;
        }
    }
    return retval;
}

QByteArray KoTarStore::completeMagic(const QByteArray& appMimetype)
{
    debugStore << "QCString KoTarStore::completeMagic( const QCString& appMimetype )********************";
    QByteArray res("Calligra ");
    res += appMimetype;
    res += '\004'; // Two magic bytes to make the identification
    res += '\006'; // more reliable (DF)
    debugStore << "sssssssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    debugStore << " return :!!!!!!!!!!!!!!! :" << res;
    return res;
}

void KoTarStore::init(const QByteArray &appIdentification)
{
    Q_D(KoStore);
    m_currentDir = 0;
    d->good = m_pTar->open(d->mode == Write ? QIODevice::WriteOnly : QIODevice::ReadOnly);

    if (!d->good)
        return;

    if (d->mode == Write) {
        debugStore << "appIdentification :" << appIdentification;
        m_pTar->setOrigFileName(completeMagic(appIdentification));
    } else {
        d->good = m_pTar->directory() != 0;
    }
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
        return false;
    }
    if (entry->isDirectory()) {
        warnStore << name << " is a directory !";
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

    debugStore << "Writing file" << d->fileName << " into TAR archive. size" << d->size;
    m_byteArray.resize(d->size); // TODO: check if really needed
    if (!m_pTar->writeFile(d->fileName, m_byteArray, 0100644, QLatin1String("user"), QLatin1String("group")))
        warnStore << "Failed to write " << d->fileName;
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
