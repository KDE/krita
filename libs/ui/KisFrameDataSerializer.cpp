/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisFrameDataSerializer.h"

#include <cstring>
#include <QTemporaryDir>


struct KRITAUI_NO_EXPORT KisFrameDataSerializer::Private
{
    Private(const QString &frameCachePath, KisTextureTileInfoPoolSP _pool)
        : framesDir(
              (!frameCachePath.isEmpty() ? frameCachePath : QDir::tempPath()) +
              QDir::separator() + "KritaFrameCacheXXXXXX"),
          pool(_pool)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(framesDir.isValid());
        framesDirObject = QDir(framesDir.path());
        framesDirObject.makeAbsolute();
    }

    QString subfolderNameForFrame(int frameId)
    {
        const int subfolderIndex = frameId & 0xff00;
        return QString::number(subfolderIndex);
    }

    QString fileNameForFrame(int frameId) {
        return QString("frame_%1").arg(frameId);
    }

    QString filePathForFrame(int frameId)
    {
        return framesDirObject.filePath(
                    subfolderNameForFrame(frameId) + QDir::separator() +
                    fileNameForFrame(frameId));
    }

    QTemporaryDir framesDir;
    QDir framesDirObject;
    KisTextureTileInfoPoolSP pool;
};

KisFrameDataSerializer::KisFrameDataSerializer(KisTextureTileInfoPoolSP pool)
    : KisFrameDataSerializer(pool, QString())
{
}

KisFrameDataSerializer::KisFrameDataSerializer(KisTextureTileInfoPoolSP pool, const QString &frameCachePath)
    : m_d(new Private(frameCachePath, pool))
{
}

KisFrameDataSerializer::~KisFrameDataSerializer()
{
}

void KisFrameDataSerializer::saveFrame(const KisFrameDataSerializer::Frame &frame)
{
    const QString frameSubfolder = m_d->subfolderNameForFrame(frame.frameId);

    if (!m_d->framesDirObject.exists(frameSubfolder)) {
        m_d->framesDirObject.mkpath(frameSubfolder);
    }

    const QString frameRelativePath = frameSubfolder + QDir::separator() + m_d->fileNameForFrame(frame.frameId);

    if (m_d->framesDirObject.exists(frameRelativePath)) {
        qWarning() << "WARNING: overwriting existing frame file!" << frameRelativePath;
        forgetFrame(frame.frameId);
    }

    const QString frameFilePath = m_d->framesDirObject.filePath(frameRelativePath);

    QFile file(frameFilePath);
    file.open(QFile::WriteOnly);

    QDataStream stream(&file);
    stream << frame.frameId;
    stream << frame.pixelSize;

    stream << int(frame.frameTiles.size());

    for (int i = 0; i < int(frame.frameTiles.size()); i++) {
        const FrameTile &tile = frame.frameTiles[i];

        stream << tile.col;
        stream << tile.row;
        stream << tile.rect;

        const int bufferSize = frame.pixelSize * tile.rect.width() * tile.rect.height();
        stream.writeRawData((char*)tile.data.data(), bufferSize);
    }

    file.close();
}

KisFrameDataSerializer::Frame KisFrameDataSerializer::loadFrame(int frameId)
{
    KisFrameDataSerializer::Frame frame;

    const QString framePath = m_d->filePathForFrame(frameId);

    QFile file(framePath);
    KIS_SAFE_ASSERT_RECOVER_NOOP(file.exists());
    if (!file.open(QFile::ReadOnly)) return frame;

    QDataStream stream(&file);

    int numTiles = 0;

    stream >> frame.frameId;
    stream >> frame.pixelSize;
    stream >> numTiles;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(frame.frameId == frameId, KisFrameDataSerializer::Frame());

    for (int i = 0; i < numTiles; i++) {
        FrameTile tile(m_d->pool);
        stream >> tile.col;
        stream >> tile.row;
        stream >> tile.rect;

        const int bufferSize = frame.pixelSize * tile.rect.width() * tile.rect.height();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(bufferSize <= m_d->pool->chunkSize(frame.pixelSize),
                                             KisFrameDataSerializer::Frame());

        tile.data.allocate(frame.pixelSize);
        stream.readRawData((char*)tile.data.data(), bufferSize);

        frame.frameTiles.push_back(std::move(tile));
    }

    file.close();

    return frame;
}

bool KisFrameDataSerializer::hasFrame(int frameId) const
{
    const QString framePath = m_d->filePathForFrame(frameId);
    return QFileInfo(framePath).exists();
}

void KisFrameDataSerializer::forgetFrame(int frameId)
{
    const QString framePath = m_d->filePathForFrame(frameId);
    QFile::remove(framePath);
}

KisTextureTileInfoPoolSP KisFrameDataSerializer::tileInfoPool() const
{
    return m_d->pool;
}

boost::optional<qreal> KisFrameDataSerializer::estimateFrameUniqueness(const KisFrameDataSerializer::Frame &lhs, const KisFrameDataSerializer::Frame &rhs, qreal portion)
{
    if (lhs.pixelSize != rhs.pixelSize) return boost::none;
    if (lhs.frameTiles.size() != rhs.frameTiles.size()) return boost::none;

    const int pixelSize = lhs.pixelSize;
    int numSampledPixels = 0;
    int numUniquePixels = 0;
    const int sampleStep = portion > 0.0 ? qMax(1, qRound(1.0 / portion)) : 0;

    for (int i = 0; i < int(lhs.frameTiles.size()); i++) {
        const FrameTile &lhsTile = lhs.frameTiles[i];
        const FrameTile &rhsTile = rhs.frameTiles[i];

        if (lhsTile.col != rhsTile.col ||
            lhsTile.row != rhsTile.row ||
            lhsTile.rect != rhsTile.rect) {

            return boost::none;
        }

        if (sampleStep > 0) {
            const int numPixels = lhsTile.rect.width() * lhsTile.rect.height();
            for (int j = 0; j < numPixels; j += sampleStep) {
                quint8 *lhsDataPtr = lhsTile.data.data() + j * pixelSize;
                quint8 *rhsDataPtr = rhsTile.data.data() + j * pixelSize;

                if (std::memcmp(lhsDataPtr, rhsDataPtr, pixelSize) != 0) {
                    numUniquePixels++;
                }
                numSampledPixels++;
            }
        }
    }

    return numSampledPixels > 0 ? qreal(numUniquePixels) / numSampledPixels : 1.0;
}

template <template <typename U> class OpPolicy, typename T>
bool processData(T *dst, const T *src, int numUnits)
{
    OpPolicy<T> op;

    bool unitsAreSame = true;

    for (int j = 0; j < numUnits; j++) {
        *dst = op(*dst, *src);

        if (*dst != 0) {
            unitsAreSame = false;
        }

        src++;
        dst++;
    }
    return unitsAreSame;
}


template<template <typename U> class OpPolicy>
bool KisFrameDataSerializer::processFrames(KisFrameDataSerializer::Frame &dst, const KisFrameDataSerializer::Frame &src)
{
    bool framesAreSame = true;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(estimateFrameUniqueness(src, dst, 0.0), false);

    for (int i = 0; i < int(src.frameTiles.size()); i++) {
        const FrameTile &srcTile = src.frameTiles[i];
        FrameTile &dstTile = dst.frameTiles[i];

        const int numBytes = srcTile.rect.width() * srcTile.rect.height() * src.pixelSize;
        const int numQWords = numBytes / 8;

        const quint64 *srcDataPtr = reinterpret_cast<const quint64*>(srcTile.data.data());
        quint64 *dstDataPtr = reinterpret_cast<quint64*>(dstTile.data.data());

        framesAreSame &= processData<OpPolicy>(dstDataPtr, srcDataPtr, numQWords);


        const int tailBytes = numBytes % 8;
        const quint8 *srcTailDataPtr = srcTile.data.data() + numBytes - tailBytes;
        quint8 *dstTailDataPtr = dstTile.data.data() + numBytes - tailBytes;

        framesAreSame &= processData<OpPolicy>(dstTailDataPtr, srcTailDataPtr, tailBytes);
    }

    return framesAreSame;
}

bool KisFrameDataSerializer::subtractFrames(KisFrameDataSerializer::Frame &dst, const KisFrameDataSerializer::Frame &src)
{
    return processFrames<std::minus>(dst, src);
}

void KisFrameDataSerializer::addFrames(KisFrameDataSerializer::Frame &dst, const KisFrameDataSerializer::Frame &src)
{
    // TODO: don't spend time on calculation of "framesAreSame" in this case
    (void) processFrames<std::plus>(dst, src);
}
