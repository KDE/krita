/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 C. Boemann <cbo@boemann.dk>
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

#include <QApplication>
#include <QTemporaryFile>
#include <QImageWriter>
#include <QCryptographicHash>
#include <QFileInfo>
#include <FlakeDebug.h>
#include <QBuffer>

KoImageDataPrivate::KoImageDataPrivate()
    : collection(0),
    errorCode(KoImageData::Success),
    key(0),
    dataStoreState(StateEmpty),
    temporaryFile(0)
{
    cleanCacheTimer->setSingleShot(true);
    cleanCacheTimer->setInterval(1000);
    QObject::connect(cleanCacheTimer.data(), &QTimer::timeout, [&]() { cleanupImageCache(); });
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
    // if we have a temp file save that to the store. This is needed as to not lose data when
    // saving lossy formats. Also writing out gif is not supported by qt so saving temp file
    // also fixes the problem that gif images are empty after saving.
    if (temporaryFile) {
        if (!temporaryFile->open()) {
            warnFlake << "Read file from temporary store failed";
            return false;
        }
        char buf[4096];
        while (true) {
            temporaryFile->waitForReadyRead(-1);
            qint64 bytes = temporaryFile->read(buf, sizeof(buf));
            if (bytes <= 0)
                break; // done!
            do {
                qint64 nWritten = device.write(buf, bytes);
                if (nWritten == -1) {
                    temporaryFile->close();
                    return false;
                }
                bytes -= nWritten;
            } while (bytes > 0);
        }
        temporaryFile->close();
        return true;
    }

    switch (dataStoreState) {
    case KoImageDataPrivate::StateEmpty:
        return false;
    case KoImageDataPrivate::StateNotLoaded:
        // we should not reach this state as above this will already be saved.
        Q_ASSERT(temporaryFile);
        return true;
    case KoImageDataPrivate::StateImageLoaded:
    case KoImageDataPrivate::StateImageOnly: {
        // save image
        QBuffer buffer;
        QImageWriter writer(&buffer, suffix.toLatin1());
        bool result = writer.write(image);
        device.write(buffer.data(), buffer.size());
        return result;
      }
    }
    return false;
}

void KoImageDataPrivate::setSuffix(const QString &name)
{
    QFileInfo fi(name);
    suffix = fi.suffix();
}

void KoImageDataPrivate::copyToTemporary(QIODevice &device)
{
    delete temporaryFile;
    temporaryFile = new QTemporaryFile(QDir::tempPath() + "/" + qAppName() + QLatin1String("_XXXXXX"));
    if (!temporaryFile->open()) {
        warnFlake << "open temporary file for writing failed";
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

    dataStoreState = StateNotLoaded;
}

void KoImageDataPrivate::cleanupImageCache()
{
    if (dataStoreState == KoImageDataPrivate::StateImageLoaded) {
        image = QImage();
        dataStoreState = KoImageDataPrivate::StateNotLoaded;
    }
}

void KoImageDataPrivate::clear()
{
    errorCode = KoImageData::Success;
    dataStoreState = StateEmpty;
    imageLocation.clear();
    imageSize = QSizeF();
    key = 0;
    image = QImage();
    pixmap = QPixmap();
}

qint64 KoImageDataPrivate::generateKey(const QByteArray &bytes)
{
    qint64 answer = 1;
    const int max = qMin(8, bytes.count());
    for (int x = 0; x < max; ++x)
        answer += qint64(bytes[x] << (8 * x));
    return answer;
}
