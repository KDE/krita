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

#include "VideoData.h"

#include "VideoCollection.h"

#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kdebug.h>

#include <QBuffer>
#include <QCryptographicHash>
#include <QFileInfo>
#include <KTemporaryFile>
#include <QPainter>

VideoData::VideoData()
    : KoShapeUserData()
    , key(0)
    , errorCode(VideoData::Success)
    , collection(0)
    , dataStoreState(StateEmpty)
    , temporaryFile(0)
{
}

VideoData::VideoData(const VideoData &videoData)
    : KoShapeUserData()
{
    //TODO copy the videodata - this is a copy constructor
}

VideoData::~VideoData()
{
    if (collection)
        collection->removeOnKey(key);
    delete temporaryFile;
}

QString VideoData::tagForSaving(int &counter)
{
    if(saveName.isEmpty())
        return saveName;
    
    if ( suffix.isEmpty() ) {
        return saveName = QString("Videos/video%1").arg(++counter);
    }
    else {
        return saveName = QString("Videos/video%1.%2").arg(++counter).arg(suffix);
    }
}

void VideoData::setExternalVideo(const QUrl &location, VideoCollection *col)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        VideoData *other = col->createExternalVideoData(location);
        this->operator=(*other);
        delete other;
    } else {
        videoLocation = location;
        setSuffix(location.toEncoded());
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(location.toEncoded());
        key = VideoData::generateKey(md5.result());
    }
}

void VideoData::setVideo(const QString &url, KoStore *store, VideoCollection *collection)
{
    if (collection) {
        // let the collection first check if it already has one. If it doesn't it'll call this method
        // again and we'll go to the other clause
        VideoData *other = collection->createVideoData(url, store);
        this->operator=(*other);
        delete other;
    } else {
        setSuffix(url);

        if (store->open(url)) {
            struct Finalizer {
                ~Finalizer() { store->close(); }
                KoStore *store;
            };
            Finalizer closer;
            closer.store = store;
            KoStoreDevice device(store);
            QByteArray data = device.readAll();
            if (!device.open(QIODevice::ReadOnly)) {
                kWarning(30006) << "open file from store " << url << "failed";
                errorCode = OpenFailed;
                return;
            }
            copyToTemporary(device);
        } else {
            kWarning(30006) << "Find file in store " << url << "failed";
            errorCode = OpenFailed;
            return;
        }
    }
}


bool VideoData::isValid() const
{
    return dataStoreState != VideoData::StateEmpty
        && errorCode == Success;
}

bool VideoData::operator==(const VideoData &other) const
{
    return false;
}

VideoData &VideoData::operator=(const VideoData &other)
{
    return *this;
}

bool VideoData::saveData(QIODevice &device)
{
    if (dataStoreState == StateSpooled) {
        Q_ASSERT(temporaryFile); // otherwise the collection should not have called this
        if (temporaryFile) {
            if (!temporaryFile->open()) {
                kWarning(30006) << "Read file from temporary store failed";
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
        }
        return true;
    }
    return false;
}

void VideoData::copyToTemporary(QIODevice &device)
{
    delete temporaryFile;
    temporaryFile = new KTemporaryFile();
    temporaryFile->setPrefix("KoVideoData");
    if (!temporaryFile->open()) {
        kWarning(30006) << "open temporary file for writing failed";
        errorCode = VideoData::StorageFailed;
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
    key = VideoData::generateKey(md5.result());
    temporaryFile->close();

    QFileInfo fi(*temporaryFile);
    dataStoreState = StateSpooled;
}

void VideoData::setSuffix(const QString &name)
{
    QRegExp rx("\\.([^/]+$)"); // TODO does this work on windows or do we have to use \ instead of / for a path separator?
    if (rx.indexIn(name) != -1) {
        suffix = rx.cap(1);
    }
}

qint64 VideoData::generateKey(const QByteArray &bytes)
{
    qint64 answer = 1;
    const int max = qMin(8, bytes.count());
    for (int x = 0; x < max; ++x)
        answer += bytes[x] << (8 * x);
    return answer;
}

#include <VideoData.moc>
