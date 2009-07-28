/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoImageData_p.h"
#include "KoImageCollection.h"

#include <QTemporaryFile>
#include <QImageWriter>
#include <QCryptographicHash>
#include <QFileInfo>
#include <KDebug>

KoImageDataPrivate::KoImageDataPrivate()
    : collection(0),
    errorCode(KoImageData::Success),
    key(0),
    cleanupTriggered(false),
    refCount(0),
    dataStoreState(StateEmpty),
    temporaryFile(0)
{
}

KoImageDataPrivate::~KoImageDataPrivate()
{
    if (collection)
        collection->removeOnKey(key);
    delete temporaryFile;
}

// called from the collection
bool KoImageDataPrivate::saveData(QIODevice &device)
{
    switch (dataStoreState) {
    case KoImageDataPrivate::StateEmpty:
        return false;
    case KoImageDataPrivate::StateNotLoaded:
        // spool directly.
        Q_ASSERT(temporaryFile); // otherwise the collection should not have called this
        if (temporaryFile) {
            if (!temporaryFile->open()) {
                kWarning(30006) << "Read file from temporary store failed";
                return false;
            }
            char buf[4096];
            while (true) {
                device.waitForReadyRead(-1);
                qint64 bytes = temporaryFile->read(buf, sizeof(buf));
                if (bytes == -1)
                    break; // done!
                do {
                    bytes -= device.write(buf, bytes);
                } while (bytes > 0);
            }
            temporaryFile->close();
        }
        return true;
    case KoImageDataPrivate::StateImageLoaded:
    case KoImageDataPrivate::StateImageOnly: {
        // save image
        QImageWriter writer(&device, suffix.toLatin1());
        return writer.write(image);
      }
    }
    return false;
}

void KoImageDataPrivate::setSuffix(const QString &name)
{
    QRegExp rx("\\.([^/]+$)"); // TODO does this work on windows or do we have to use \ instead of / for a path separator?
    if (rx.indexIn(name) != -1) {
        suffix = rx.cap(1);
    }
}

void KoImageDataPrivate::copyToTemporary(QIODevice &device)
{
    delete temporaryFile;
    temporaryFile = new QTemporaryFile("KoImageDataXXXXXX");
    if (!temporaryFile->open()) {
        kWarning(30006) << "open temporary file for writing failed";
        errorCode = KoImageData::StorageFailed;
        return;
    }
    QCryptographicHash md5(QCryptographicHash::Md5);
    char buf[8096];
    while (true) {
        device.waitForReadyRead(-1);
        qint64 bytes = device.read(buf, sizeof(buf));
        if (bytes <= 0)
            break; // done!
        md5.addData(buf, bytes);
        do {
            bytes -= temporaryFile->write(buf, bytes);
        } while (bytes > 0);
    }
    key = KoImageDataPrivate::generateKey(md5.result());
    temporaryFile->close();

    QFileInfo fi(*temporaryFile);
    imageLocation = QUrl(fi.absoluteFilePath());
    dataStoreState = StateNotLoaded;
}

void KoImageDataPrivate::cleanupImageCache()
{
    if (dataStoreState == KoImageDataPrivate::StateImageLoaded) {
        image = QImage();
        dataStoreState = KoImageDataPrivate::StateNotLoaded;
    }
    cleanupTriggered = false;
}

void KoImageDataPrivate::clear()
{
    errorCode = KoImageData::Success;
    dataStoreState = StateEmpty;
    imageLocation.clear();
    imageSize = QSizeF();
    key = 0;
    image = QImage();
}

qint64 KoImageDataPrivate::generateKey(const QByteArray &bytes)
{
    qint64 answer = 1;
    const int max = qMin(8, bytes.count());
    for (int x = 0; x < max; ++x)
        answer += bytes[x] << (8 * x);
    return answer;
}
