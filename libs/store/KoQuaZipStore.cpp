/*
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoQuaZipStore.h"
#include "KoStore_p.h"

#include <StoreDebug.h>

#include <zlib.h>
#include <quazip.h>
#include <quazipfile.h>
#include <quazipdir.h>
#include <quazipfileinfo.h>
#include <quazipnewinfo.h>

#include <QTemporaryFile>
#include <QTextCodec>
#include <QByteArray>
#include <QBuffer>

#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>

struct KoQuaZipStore::Private {

    Private() {}
    ~Private() {}

    QuaZip *archive {0};
    QuaZipFile *currentFile {0};
    int compressionLevel {Z_DEFAULT_COMPRESSION};
    bool usingSaveFile {false};
    QByteArray cache;
    QBuffer buffer;
};


KoQuaZipStore::KoQuaZipStore(const QString &_filename, KoStore::Mode _mode, const QByteArray &appIdentification, bool writeMimetype)
    : KoStore(_mode, writeMimetype)
    , dd(new Private())
{
    Q_D(KoStore);
    debugStore << "KoQuaZipStore" << _filename;
    d->localFileName = _filename;
    dd->archive = new QuaZip(_filename);
    init(appIdentification);

}

KoQuaZipStore::KoQuaZipStore(QIODevice *dev, KoStore::Mode _mode, const QByteArray &appIdentification, bool writeMimetype)
    : KoStore(_mode, writeMimetype)
    , dd(new Private())
{
    debugStore << "KoQuaZipStore" << dev;
    dd->archive = new QuaZip(dev);
    init(appIdentification);
}

KoQuaZipStore::~KoQuaZipStore()
{
    Q_D(KoStore);

    if (dd->currentFile && dd->currentFile->isOpen()) {
        dd->currentFile->close();
    }

    if (!d->finalized) {
        finalize();
    }

    delete dd->archive;
    delete dd->currentFile;
}

void KoQuaZipStore::setCompressionEnabled(bool enabled)
{

    if (enabled) {
        dd->compressionLevel = Z_BEST_COMPRESSION;
    }
    else {
        dd->compressionLevel = Z_NO_COMPRESSION;
    }
}

qint64 KoQuaZipStore::write(const char *_data, qint64 _len)
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

    d->size += _len;
    if (dd->buffer.write(_data, _len)) {    // writeData returns a bool!
        return _len;
    }
    return 0;
}

QStringList KoQuaZipStore::directoryList() const
{
    debugStore << dd->archive->getFileNameList();
    return dd->archive->getFileNameList();
}

void KoQuaZipStore::init(const QByteArray &appIdentification)
{
    Q_D(KoStore);

    bool enableZip64 = false;
    if (appIdentification == "application/x-krita") {
        enableZip64 = KSharedConfig::openConfig()->group("").readEntry<bool>("UseZip64", false);
    }
    dd->archive->setZip64Enabled(enableZip64);
    dd->archive->setFileNameCodec("UTF-8");
    dd->usingSaveFile = dd->archive->getIoDevice() && dd->archive->getIoDevice()->inherits("QSaveFile");
    dd->archive->setAutoClose(!dd->usingSaveFile);

    d->good = dd->archive->open(d->mode == Write ? QuaZip::mdCreate : QuaZip::mdUnzip);

    if (!d->good) {
        return;
    }

    if (d->mode == Write) {
        if (d->writeMimetype) {
            QuaZipFile f(dd->archive);
            QuaZipNewInfo newInfo("mimetype");
            newInfo.setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            if (!f.open(QIODevice::WriteOnly, newInfo, 0, 0, Z_DEFLATED, Z_NO_COMPRESSION)) {
                d->good = false;
                return;
            }
            f.write(appIdentification);
            f.close();
        }
    }
    else {
        debugStore << dd->archive->getEntriesCount() << dd->archive->getFileNameList();
        d->good = dd->archive->getEntriesCount();
    }
}

bool KoQuaZipStore::doFinalize()
{
    Q_D(KoStore);

    d->stream = 0;
    if (!dd->usingSaveFile) {
        dd->archive->close();
    }
    return dd->archive->getZipError() == ZIP_OK;

}

bool KoQuaZipStore::openWrite(const QString &name)
{
    Q_D(KoStore);
    QString fixedPath = name;
    fixedPath.replace("//", "/");

    delete d->stream;
    d->stream = 0; // Not used when writing

    delete dd->currentFile;
    dd->currentFile = new QuaZipFile(dd->archive);
    QuaZipNewInfo newInfo(fixedPath);
    newInfo.setPermissions(QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    bool r = dd->currentFile->open(QIODevice::WriteOnly, newInfo, 0, 0, Z_DEFLATED, dd->compressionLevel);
    if (!r) {
        qWarning() << "Could not open" << name << dd->currentFile->getZipError();
    }

    dd->cache = QByteArray();
    dd->buffer.setBuffer(&dd->cache);
    dd->buffer.open(QBuffer::WriteOnly);

    return r;
}

bool KoQuaZipStore::openRead(const QString &name)
{
    Q_D(KoStore);

    QString fixedPath = name;
    fixedPath.replace("//", "/");

    delete d->stream;
    d->stream = 0;
    delete dd->currentFile;
    dd->currentFile = 0;

    if (!currentPath().isEmpty() && !fixedPath.startsWith(currentPath())) {
        fixedPath = currentPath() + '/' + fixedPath;
    }

    debugStore << "openRead" << name << fixedPath << currentPath();

    if (!dd->archive->setCurrentFile(fixedPath)) {
        //qWarning() << "\t\tCould not set current file" << dd->archive->getZipError() << fixedPath;
        return false;
    }

    dd->currentFile = new QuaZipFile(dd->archive);
    if (!dd->currentFile->open(QIODevice::ReadOnly)) {
        qWarning() << "\t\t\tBut could not open!!!" << dd->archive->getZipError();
        return false;
    }
    d->stream = dd->currentFile;
    d->size = dd->currentFile->size();
    return true;
}

bool KoQuaZipStore::closeWrite()
{
    Q_D(KoStore);

    bool r = true;
    if (!dd->currentFile->write(dd->cache)) {
        qWarning() << "Could not write buffer to the file";
        r = false;
    }
    dd->buffer.close();
    dd->currentFile->close();
    d->stream = 0;
    return (r && dd->currentFile->getZipError() == ZIP_OK);
}

bool KoQuaZipStore::closeRead()
{
    Q_D(KoStore);
    d->stream = 0;
    return true;
}

bool KoQuaZipStore::enterRelativeDirectory(const QString &path)
{
    debugStore << "enterRelativeDirectory()" << path;
    return true;
}

bool KoQuaZipStore::enterAbsoluteDirectory(const QString &path)
{
    debugStore << "enterAbsoluteDirectory()" << path;

    QString fixedPath = path;
    fixedPath.replace("//", "/");

    if (fixedPath.isEmpty()) {
        fixedPath = "/";
    }
    QuaZipDir currentDir (dd->archive, fixedPath);
    return currentDir.exists();
}

bool KoQuaZipStore::fileExists(const QString &absPath) const
{
    QString fixedPath = absPath;
    fixedPath.replace("//", "/");

    debugStore << "fileExists()" << fixedPath << dd->archive->getFileNameList().contains(fixedPath);

    return dd->archive->getFileNameList().contains(fixedPath);
}
