/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>, Werner Trobin <trobin@kde.org>
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

#include "KoStore.h"
#include "KoStore_p.h"

#include "KoTarStore.h"
#include "KoZipStore.h"
#include "KoDirectoryStore.h"
#ifdef QCA2
#include "KoEncryptedStore.h"
#endif

#include <QBuffer>
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include <kurl.h>
#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

#define DefaultFormat KoStore::Zip

const int KoStore::s_area = 30002;

KoStore::Backend KoStore::determineBackend(QIODevice *dev)
{
    unsigned char buf[5];
    if (dev->read((char *)buf, 4) < 4)
        return DefaultFormat; // will create a "bad" store (bad()==true)
    if (buf[0] == 0037 && buf[1] == 0213)   // gzip -> tar.gz
        return Tar;
    if (buf[0] == 'P' && buf[1] == 'K' && buf[2] == 3 && buf[3] == 4)
        return Zip;
    return DefaultFormat; // fallback
}

KoStore* KoStore::createStore(const QString& fileName, Mode mode, const QByteArray & appIdentification, Backend backend)
{
    bool automatic = false;
    if (backend == Auto) {
        automatic = true;
        if (mode == KoStore::Write)
            backend = DefaultFormat;
        else {
            QFileInfo inf(fileName);
            if (inf.isDir())
                backend = Directory;
            else {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly))
                    backend = determineBackend(&file);
                else
                    backend = DefaultFormat; // will create a "bad" store (bad()==true)
            }
        }
    }
    switch (backend) {
    case Tar:
        return new KoTarStore(fileName, mode, appIdentification);
    case Zip:
#ifdef QCA2
        if (automatic && mode == Read) {
            // When automatically detecting, this might as well be an encrypted file. We'll need to check anyway, so we'll just use the encrypted store.
            return new KoEncryptedStore(fileName, Read, appIdentification);
        }
#endif
        return new KoZipStore(fileName, mode, appIdentification);
    case Directory:
        return new KoDirectoryStore(fileName /* should be a dir name.... */, mode);
#ifdef QCA2
    case Encrypted:
        return new KoEncryptedStore(fileName, mode, appIdentification);
#endif
    default:
        kWarning(s_area) << "Unsupported backend requested for KoStore : " << backend;
        return 0;
    }
}

KoStore* KoStore::createStore(QIODevice *device, Mode mode, const QByteArray & appIdentification, Backend backend)
{
    bool automatic = false;
    if (backend == Auto) {
        automatic = true;
        if (mode == KoStore::Write)
            backend = DefaultFormat;
        else {
            if (device->open(QIODevice::ReadOnly)) {
                backend = determineBackend(device);
                device->close();
            }
        }
    }
    switch (backend) {
    case Tar:
        return new KoTarStore(device, mode, appIdentification);
    case Directory:
        kError(s_area) << "Can't create a Directory store for a memory buffer!" << endl;
        // fallback
    case Zip:
#ifdef QCA2
        if (automatic && mode == Read) {
            // When automatically detecting, this might as well be an encrypted file. We'll need to check anyway, so we'll just use the encrypted store.
            return new KoEncryptedStore(device, Read, appIdentification);
        }
#endif
        return new KoZipStore(device, mode, appIdentification);
#ifdef QCA2
    case Encrypted:
        return new KoEncryptedStore(device, mode, appIdentification);
#endif
    default:
        kWarning(s_area) << "Unsupported backend requested for KoStore : " << backend;
        return 0;
    }
}

KoStore* KoStore::createStore(QWidget* window, const KUrl& url, Mode mode, const QByteArray & appIdentification, Backend backend)
{
    const bool automatic = (backend == Auto);
    if (url.isLocalFile())
        return createStore(url.toLocalFile(), mode,  appIdentification, backend);

    QString tmpFile;
    if (mode == KoStore::Write) {
        if (automatic)
            backend = DefaultFormat;
    } else {
        const bool downloaded =
            KIO::NetAccess::download(url, tmpFile, window);

        if (!downloaded) {
            kError(s_area) << "Could not download file!" << endl;
            backend = DefaultFormat; // will create a "bad" store (bad()==true)
        } else if (automatic) {
            QFile file(tmpFile);
            if (file.open(QIODevice::ReadOnly)) {
                backend = determineBackend(&file);
                file.close();
            }
        }
    }
    switch (backend) {
    case Tar:
        return new KoTarStore(window, url, tmpFile, mode, appIdentification);
    case Zip:
#ifdef QCA2
        if (automatic && mode == Read) {
            // When automatically detecting, this might as well be an encrypted file. We'll need to check anyway, so we'll just use the encrypted store.
            return new KoEncryptedStore(window, url, tmpFile, Read, appIdentification);
        }
#endif
        return new KoZipStore(window, url, tmpFile, mode, appIdentification);
#ifdef QCA2
    case Encrypted:
        return new KoEncryptedStore(window, url, tmpFile, mode, appIdentification);
#endif
    default:
        kWarning(s_area) << "Unsupported backend requested for KoStore (KUrl) : " << backend;
        KMessageBox::sorry(window,
                           i18n("The directory mode is not supported for remote locations."),
                           i18n("KOffice Storage"));
        return 0;
    }
}

namespace
{
const char* const ROOTPART = "root";
const char* const MAINNAME = "maindoc.xml";
}

KoStore::KoStore() : d_ptr(new KoStorePrivate)
{
}

bool KoStore::init(Mode mode)
{
    Q_D(KoStore);
    d->isOpen = false;
    d->mode = mode;
    d->stream = 0;
    d->finalized = false;

    // Assume new style names.
    d->namingVersion = KoStorePrivate::NamingVersion22;
    return true;
}

KoStore::~KoStore()
{
    Q_D(KoStore);
    delete d->stream;
    delete d_ptr;
}

KUrl KoStore::urlOfStore() const
{
    Q_D(const KoStore);
    if (d->fileMode == KoStorePrivate::RemoteRead || d->fileMode == KoStorePrivate::RemoteWrite)
        return d->url;
    else
        return KUrl(d->localFileName);
}

bool KoStore::open(const QString & _name)
{
    Q_D(KoStore);
    // This also converts from relative to absolute, i.e. merges the currentPath()
    d->fileName = toExternalNaming(_name);

    if (d->isOpen) {
        kWarning(s_area) << "Store is already opened, missing close";
        //return KIO::ERR_INTERNAL;
        return false;
    }

    if (d->fileName.length() > 512) {
        kError(s_area) << "KoStore: Filename " << d->fileName << " is too long" << endl;
        //return KIO::ERR_MALFORMED_URL;
        return false;
    }

    if (d->mode == Write) {
        kDebug(s_area) << "opening for writing" << d->fileName;
        if (d->filesList.contains(d->fileName)) {
            kWarning(s_area) << "KoStore: Duplicate filename" << d->fileName;
            //return KIO::ERR_FILE_ALREADY_EXIST;
            return false;
        }

        d->filesList.append(d->fileName);

        d->size = 0;
        if (!openWrite(d->fileName))
            return false;
    } else if (d->mode == Read) {
        kDebug(s_area) << "Opening for reading" << d->fileName;
        if (!openRead(d->fileName))
            return false;
    } else
        //return KIO::ERR_UNSUPPORTED_ACTION;
        return false;

    d->isOpen = true;
    return true;
}

bool KoStore::isOpen() const
{
    Q_D(const KoStore);
    return d->isOpen;
}

bool KoStore::close()
{
    Q_D(KoStore);
    kDebug(s_area) << "Closing";

    if (!d->isOpen) {
        kWarning(s_area) << "You must open before closing";
        //return KIO::ERR_INTERNAL;
        return false;
    }

    bool ret = d->mode == Write ? closeWrite() : closeRead();

    delete d->stream;
    d->stream = 0;
    d->isOpen = false;
    return ret;
}

QIODevice* KoStore::device() const
{
    Q_D(const KoStore);
    if (!d->isOpen)
        kWarning(s_area) << "You must open before asking for a device";
    if (d->mode != Read)
        kWarning(s_area) << "Can not get device from store that is opened for writing";
    return d->stream;
}

QByteArray KoStore::read(qint64 max)
{
    Q_D(KoStore);
    QByteArray data;

    if (!d->isOpen) {
        kWarning(s_area) << "You must open before reading";
        return data;
    }
    if (d->mode != Read) {
        kError(s_area) << "KoStore: Can not read from store that is opened for writing" << endl;
        return data;
    }

    return d->stream->read(max);
}

qint64 KoStore::write(const QByteArray& data)
{
    return write(data.data(), data.size());   // see below
}

qint64 KoStore::read(char *_buffer, qint64 _len)
{
    Q_D(KoStore);
    if (!d->isOpen) {
        kError(s_area) << "KoStore: You must open before reading" << endl;
        return -1;
    }
    if (d->mode != Read) {
        kError(s_area) << "KoStore: Can not read from store that is opened for writing" << endl;
        return -1;
    }

    return d->stream->read(_buffer, _len);
}

qint64 KoStore::write(const char* _data, qint64 _len)
{
    Q_D(KoStore);
    if (_len == 0) return 0;

    if (!d->isOpen) {
        kError(s_area) << "KoStore: You must open before writing" << endl;
        return 0;
    }
    if (d->mode != Write) {
        kError(s_area) << "KoStore: Can not write to store that is opened for reading" << endl;
        return 0;
    }

    int nwritten = d->stream->write(_data, _len);
    Q_ASSERT(nwritten == (int)_len);
    d->size += nwritten;

    return nwritten;
}

qint64 KoStore::size() const
{
    Q_D(const KoStore);
    if (!d->isOpen) {
        kWarning(s_area) << "You must open before asking for a size";
        return static_cast<qint64>(-1);
    }
    if (d->mode != Read) {
        kWarning(s_area) << "Can not get size from store that is opened for writing";
        return static_cast<qint64>(-1);
    }
    return d->size;
}

bool KoStore::enterDirectory(const QString& directory)
{
    //kDebug(s_area) <<"enterDirectory" << directory;
    int pos;
    bool success = true;
    QString tmp(directory);

    while ((pos = tmp.indexOf('/')) != -1 &&
            (success = enterDirectoryInternal(tmp.left(pos))))
        tmp = tmp.mid(pos + 1);

    if (success && !tmp.isEmpty())
        return enterDirectoryInternal(tmp);
    return success;
}

bool KoStore::leaveDirectory()
{
    Q_D(KoStore);
    if (d->currentPath.isEmpty())
        return false;

    d->currentPath.pop_back();

    return enterAbsoluteDirectory(expandEncodedDirectory(currentPath()));
}

QString KoStore::currentDirectory() const
{
    return expandEncodedDirectory(currentPath());
}

QString KoStore::currentPath() const
{
    Q_D(const KoStore);
    QString path;
    QStringList::ConstIterator it = d->currentPath.begin();
    QStringList::ConstIterator end = d->currentPath.end();
    for (; it != end; ++it) {
        path += *it;
        path += '/';
    }
    return path;
}

void KoStore::pushDirectory()
{
    Q_D(KoStore);
    d->directoryStack.push(currentPath());
}

void KoStore::popDirectory()
{
    Q_D(KoStore);
    d->currentPath.clear();
    enterAbsoluteDirectory(QString());
    enterDirectory(d->directoryStack.pop());
}

bool KoStore::addLocalFile(const QString &fileName, const QString &destName)
{
    QFileInfo fi(fileName);
    uint size = fi.size();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (!open(destName)) {
        return false;
    }

    QByteArray data;
    data.resize(8 * 1024);

    uint total = 0;
    for (int block = 0; (block = file.read(data.data(), data.size())) > 0; total += block) {
        data.resize(block);
        if (write(data) != block)
            return false;
        data.resize(8*1024);
    }
    Q_ASSERT(total == size);
    if (total != size) {
        kWarning(s_area) << "Did not write enough bytes. Expected: " << size << ", wrote" << total;
        return false;
    }

    close();
    file.close();

    return true;
}

bool KoStore::addDataToFile(QByteArray &buffer, const QString &destName)
{
    QBuffer file(&buffer);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (!open(destName)) {
        return false;
    }

    QByteArray data;
    data.resize(8 * 1024);

    uint total = 0;
    for (int block = 0; (block = file.read(data.data(), data.size())) > 0; total += block) {
        data.resize(block);
        if (write(data) != block)
            return false;
        data.resize(8*1024);
    }

    close();
    file.close();

    return true;
}

bool KoStore::extractFile(const QString &srcName, const QString &fileName)
{
    QFile file(fileName);
    return extractFile(srcName, file);
}


bool KoStore::extractFile(const QString &srcName, QByteArray &data)
{
    QBuffer buffer(&data);
    return extractFile(srcName, buffer);
}

bool KoStore::extractFile(const QString &srcName, QIODevice &buffer)
{
    if (!open(srcName))
        return false;

    if (!buffer.open(QIODevice::WriteOnly)) {
        close();
        return false;
    }
    // ### This could use KArchive::copy or something, no?

    QByteArray data;
    data.resize(8 * 1024);
    uint total = 0;
    for (int block = 0; (block = read(data.data(), data.size())) > 0; total += block) {
        buffer.write(data.data(), block);
    }

    if (size() != static_cast<qint64>(-1))
        Q_ASSERT(total == size());

    buffer.close();
    close();

    return true;
}

bool KoStore::seek(qint64 pos)
{
    Q_D(KoStore);
    return d->stream->seek(pos);
}

qint64 KoStore::pos() const
{
    Q_D(const KoStore);
    return d->stream->pos();
}

bool KoStore::atEnd() const
{
    Q_D(const KoStore);
    return d->stream->atEnd();
}

// See the specification for details of what this function does.
QString KoStore::toExternalNaming(const QString & _internalNaming) const
{
    if (_internalNaming == ROOTPART)
        return expandEncodedDirectory(currentPath()) + MAINNAME;

    QString intern;
    if (_internalNaming.startsWith("tar:/"))     // absolute reference
        intern = _internalNaming.mid(5);   // remove protocol
    else
        intern = currentPath() + _internalNaming;

    return expandEncodedPath(intern);
}

QString KoStore::expandEncodedPath(const QString& _intern) const
{
    Q_D(const KoStore);
    QString intern = _intern;

    if (d->namingVersion == KoStorePrivate::NamingVersionRaw)
        return intern;

    QString result;
    int pos;

    if ((pos = intern.lastIndexOf('/', -1)) != -1) {
        result = expandEncodedDirectory(intern.left(pos)) + '/';
        intern = intern.mid(pos + 1);
    }

    // Now process the filename. If the first character is numeric, we have
    // a main document.
    if (QChar(intern.at(0)).isDigit()) {
        // If this is the first part name, check if we have a store with
        // old-style names.
        if ((d->namingVersion == KoStorePrivate::NamingVersion22) &&
                (d->mode == Read) &&
                (fileExists(result + "part" + intern + ".xml")))
            d->namingVersion = KoStorePrivate::NamingVersion21;

        if (d->namingVersion == KoStorePrivate::NamingVersion21)
            result = result + "part" + intern + ".xml";
        else
            result = result + "part" + intern + '/' + MAINNAME;
    } else
        result += intern;
    return result;
}

QString KoStore::expandEncodedDirectory(const QString& _intern) const
{
    Q_D(const KoStore);
    QString intern = _intern;

    if (d->namingVersion == KoStorePrivate::NamingVersionRaw)
        return intern;

    QString result;
    int pos;
    while ((pos = intern.indexOf('/')) != -1) {
        if (QChar(intern.at(0)).isDigit())
            result += "part";
        result += intern.left(pos + 1);   // copy numbers (or "pictures") + "/"
        intern = intern.mid(pos + 1);   // remove the dir we just processed
    }

    if (!intern.isEmpty() && QChar(intern.at(0)).isDigit())
        result += "part";
    result += intern;
    return result;
}

bool KoStore::enterDirectoryInternal(const QString& directory)
{
    Q_D(KoStore);
    if (enterRelativeDirectory(expandEncodedDirectory(directory))) {
        d->currentPath.append(directory);
        return true;
    }
    return false;
}

void KoStore::disallowNameExpansion()
{
    Q_D(KoStore);
    d->namingVersion = KoStorePrivate::NamingVersionRaw;
}

bool KoStore::hasFile(const QString& fileName) const
{
    return fileExists(toExternalNaming(currentPath() + fileName));
}

bool KoStore::finalize()
{
    Q_D(KoStore);
    Q_ASSERT(!d->finalized);   // call this only once!
    d->finalized = true;
    return doFinalize();
}

bool KoStore::isEncrypted()
{
    return false;
}

bool KoStore::setPassword(const QString& /*password*/)
{
    return false;
}

QString KoStore::password()
{
    return QString();
}

bool KoStore::bad() const
{
    Q_D(const KoStore);
    return !d->good;
}

KoStore::Mode KoStore::mode() const
{
    Q_D(const KoStore);
    return d->mode;
}
