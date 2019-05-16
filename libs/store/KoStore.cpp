/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>, Werner Trobin <trobin@kde.org>
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

#include "KoStore.h"
#include "KoStore_p.h"

#include "KoLegacyZipStore.h"
#include "KoQuaZipStore.h"
#include "KoDirectoryStore.h"

#include <QBuffer>
#include <QFileInfo>
#include <QFile>

#include <QUrl>
#include <StoreDebug.h>

#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>


#define DefaultFormat KoStore::Zip

static KoStore::Backend determineBackend(QIODevice *dev)
{
    unsigned char buf[5];
    if (dev->read((char *)buf, 4) < 4)
        return DefaultFormat; // will create a "bad" store (bad()==true)
    if (buf[0] == 'P' && buf[1] == 'K' && buf[2] == 3 && buf[3] == 4)
        return KoStore::Zip;
    return DefaultFormat; // fallback
}

KoStore* KoStore::createStore(const QString& fileName, Mode mode, const QByteArray & appIdentification, Backend backend, bool writeMimetype)
{
    if (backend == Auto) {
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
    case Zip:
        if (appIdentification == "application/x-krita" && KSharedConfig::openConfig()->group("").readEntry<bool>("UseZip64", false)) {
            return new KoQuaZipStore(fileName, mode, appIdentification, writeMimetype);
        }
        else {
            KoStore *store = new KoLegacyZipStore(fileName, mode, appIdentification, writeMimetype);
            if (store->bad()) {
                return new KoQuaZipStore(fileName, mode, appIdentification, writeMimetype);
            }
            return store;
        }

    case Directory:
        return new KoDirectoryStore(fileName /* should be a dir name.... */, mode, writeMimetype);
    default:
        warnStore << "Unsupported backend requested for KoStore : " << backend;
        return 0;
    }
}

KoStore* KoStore::createStore(QIODevice *device, Mode mode, const QByteArray & appIdentification, Backend backend, bool writeMimetype)
{
    if (backend == Auto) {
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
    case Directory:
        errorStore << "Can't create a Directory store for a memory buffer!" << endl;
        return 0;
    case Zip:
        if (appIdentification == "application/x-krita" && KSharedConfig::openConfig()->group("").readEntry<bool>("UseZip64", false)) {
            return new KoQuaZipStore(device, mode, appIdentification, writeMimetype);
        }
        else {
            KoStore *store = new KoLegacyZipStore(device, mode, appIdentification, writeMimetype);
            if (store->bad()) {
                return new KoQuaZipStore(device, mode, appIdentification, writeMimetype);
            }
            return store;
        }
    default:
        warnStore << "Unsupported backend requested for KoStore : " << backend;
        return 0;
    }
}

namespace
{
const char ROOTPART[] = "root";
const char MAINNAME[] = "maindoc.xml";
}

KoStore::KoStore(Mode mode, bool writeMimetype)
    : d_ptr(new KoStorePrivate(this, mode, writeMimetype))
{}

KoStore::~KoStore()
{
    Q_D(KoStore);
    delete d->stream;
    delete d_ptr;
}

bool KoStore::open(const QString & _name)
{
    Q_D(KoStore);
    // This also converts from relative to absolute, i.e. merges the currentPath()
    d->fileName = d->toExternalNaming(_name);

    debugStore << "KOStore" << _name << d->fileName;

    if (d->isOpen) {
        warnStore << "Store is already opened, missing close";
        return false;
    }

    if (d->fileName.length() > 512) {
        errorStore << "KoStore: Filename " << d->fileName << " is too long" << endl;
        return false;
    }

    if (d->mode == Write) {
        debugStore << "opening for writing" << d->fileName;
        if (d->filesList.contains(d->fileName)) {
            warnStore << "KoStore: Duplicate filename" << d->fileName;
            return false;
        }

        d->filesList.append(d->fileName);

        d->size = 0;
        if (!openWrite(d->fileName))
            return false;
    } else if (d->mode == Read) {
        debugStore << "Opening for reading" << d->fileName;
        if (!openRead(d->fileName))
            return false;
    } else
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
    if (!d->isOpen) {
        warnStore << "You must open before closing";
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
        warnStore << "You must open before asking for a device";
    if (d->mode != Read)
        warnStore << "Can not get device from store that is opened for writing";
    return d->stream;
}

QByteArray KoStore::read(qint64 max)
{
    Q_D(KoStore);
    QByteArray data;

    if (!d->isOpen) {
        warnStore << "You must open before reading";
        return data;
    }
    if (d->mode != Read) {
        errorStore << "KoStore: Can not read from store that is opened for writing" << endl;
        return data;
    }

    return d->stream->read(max);
}

qint64 KoStore::write(const QByteArray& data)
{
    return write(data.constData(), data.size());   // see below
}

qint64 KoStore::read(char *_buffer, qint64 _len)
{
    Q_D(KoStore);
    if (!d->isOpen) {
        errorStore << "KoStore: You must open before reading" << endl;
        return -1;
    }
    if (d->mode != Read) {
        errorStore << "KoStore: Can not read from store that is opened for writing" << endl;
        return -1;
    }

    return d->stream->read(_buffer, _len);
}

qint64 KoStore::write(const char* _data, qint64 _len)
{
    Q_D(KoStore);
    if (_len == 0) return 0;

    if (!d->isOpen) {
        errorStore << "KoStore: You must open before writing" << endl;
        return 0;
    }
    if (d->mode != Write) {
        errorStore << "KoStore: Can not write to store that is opened for reading" << endl;
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
        warnStore << "You must open before asking for a size";
        return static_cast<qint64>(-1);
    }
    if (d->mode != Read) {
        warnStore << "Can not get size from store that is opened for writing";
        return static_cast<qint64>(-1);
    }
    return d->size;
}

bool KoStore::enterDirectory(const QString &directory)
{
    Q_D(KoStore);
    //debugStore <<"enterDirectory" << directory;
    int pos;
    bool success = true;
    QString tmp(directory);

    while ((pos = tmp.indexOf('/')) != -1 &&
            (success = d->enterDirectoryInternal(tmp.left(pos))))
        tmp.remove(0, pos + 1);

    if (success && !tmp.isEmpty())
        return d->enterDirectoryInternal(tmp);
    return success;
}

bool KoStore::leaveDirectory()
{
    Q_D(KoStore);
    if (d->currentPath.isEmpty())
        return false;

    d->currentPath.pop_back();

    return enterAbsoluteDirectory(currentPath());
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

bool KoStore::extractFile(const QString &srcName, QByteArray &data)
{
    Q_D(KoStore);
    QBuffer buffer(&data);
    return d->extractFile(srcName, buffer);
}

bool KoStorePrivate::extractFile(const QString &srcName, QIODevice &buffer)
{
    if (!q->open(srcName))
        return false;

    if (!buffer.open(QIODevice::WriteOnly)) {
        q->close();
        return false;
    }

    QByteArray data;
    data.resize(8 * 1024);
    uint total = 0;
    for (int block = 0; (block = q->read(data.data(), data.size())) > 0; total += block) {
        buffer.write(data.data(), block);
    }

    if (q->size() != static_cast<qint64>(-1))
        Q_ASSERT(total == q->size());

    buffer.close();
    q->close();

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
QString KoStorePrivate::toExternalNaming(const QString & _internalNaming) const
{
    if (_internalNaming == ROOTPART)
        return q->currentPath() + MAINNAME;

    QString intern;
    if (_internalNaming.startsWith("tar:/"))     // absolute reference
        intern = _internalNaming.mid(5);   // remove protocol
    else
        intern = q->currentPath() + _internalNaming;

    return intern;
}


bool KoStorePrivate::enterDirectoryInternal(const QString &directory)
{
    if (q->enterRelativeDirectory(directory)) {
        currentPath.append(directory);
        return true;
    }
    return false;
}

bool KoStore::hasFile(const QString& fileName) const
{
    Q_D(const KoStore);
    return fileExists(d->toExternalNaming(fileName));
}

bool KoStore::hasDirectory(const QString &directoryName)
{
    return enterAbsoluteDirectory(directoryName);
}

bool KoStore::finalize()
{
    Q_D(KoStore);
    Q_ASSERT(!d->finalized);   // call this only once!
    d->finalized = true;
    return doFinalize();
}

void KoStore::setCompressionEnabled(bool /*e*/)
{
}

void KoStore::setSubstitution(const QString &name, const QString &substitution)
{
    Q_D(KoStore);
    d->substituteThis = name;
    d->substituteWith = substitution;
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

QStringList KoStore::directoryList() const
{
    return QStringList();
}
