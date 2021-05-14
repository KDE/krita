/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFrameDataSerializer.h"

#include <cstring>

#include <QTemporaryDir>
#include <QElapsedTimer>

#include "tiles3/swap/kis_lzf_compression.h"

struct KRITAUI_NO_EXPORT KisFrameDataSerializer::Private
{
    Private(const QString &frameCachePath)
        : framesDir(
              (!frameCachePath.isEmpty() && QTemporaryDir(frameCachePath + "/KritaFrameCacheXXXXXX").isValid()
               ? frameCachePath
               : QDir::tempPath())
              + "/KritaFrameCacheXXXXXX")
    {
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
                    subfolderNameForFrame(frameId) + '/' +
                    fileNameForFrame(frameId));
    }

    int generateFrameId() {
        // TODO: handle wrapping and range compression
        return nextFrameId++;
    }

    quint8* getCompressionBuffer(int size) {
        if (compressionBuffer.size() < size) {
            compressionBuffer.resize(size);
        }
        return reinterpret_cast<quint8*>(compressionBuffer.data());
    }

    QTemporaryDir framesDir;
    QDir framesDirObject;
    int nextFrameId = 0;

    QByteArray compressionBuffer;
};

KisFrameDataSerializer::KisFrameDataSerializer()
    : KisFrameDataSerializer(QString())
{
}

KisFrameDataSerializer::KisFrameDataSerializer(const QString &frameCachePath)
    : m_d(new Private(frameCachePath))
{
}

KisFrameDataSerializer::~KisFrameDataSerializer()
{
}

int KisFrameDataSerializer::saveFrame(const KisFrameDataSerializer::Frame &frame)
{
    KisLzfCompression compression;

    const int frameId = m_d->generateFrameId();

    const QString frameSubfolder = m_d->subfolderNameForFrame(frameId);

    if (!m_d->framesDirObject.exists(frameSubfolder)) {
        m_d->framesDirObject.mkpath(frameSubfolder);
    }

    const QString frameRelativePath = frameSubfolder + '/' + m_d->fileNameForFrame(frameId);

    if (m_d->framesDirObject.exists(frameRelativePath)) {
        qWarning() << "WARNING: overwriting existing frame file!" << frameRelativePath;
        forgetFrame(frameId);
    }

    const QString frameFilePath = m_d->framesDirObject.filePath(frameRelativePath);

    QFile file(frameFilePath);
    file.open(QFile::WriteOnly);

    QDataStream stream(&file);
    stream << frameId;
    stream << frame.pixelSize;

    stream << int(frame.frameTiles.size());

    for (int i = 0; i < int(frame.frameTiles.size()); i++) {
        const FrameTile &tile = frame.frameTiles[i];

        stream << tile.col;
        stream << tile.row;
        stream << tile.rect;

        const int frameByteSize = frame.pixelSize * tile.rect.width() * tile.rect.height();
        const int maxBufferSize = compression.outputBufferSize(frameByteSize);
        quint8 *buffer = m_d->getCompressionBuffer(maxBufferSize);

        const int compressedSize =
            compression.compress(tile.data.data(), frameByteSize, buffer, maxBufferSize);

        //ENTER_FUNCTION() << ppVar(compressedSize) << ppVar(frameByteSize);

        const bool isCompressed = compressedSize < frameByteSize;
        stream << isCompressed;

        if (isCompressed) {
            stream << compressedSize;
            stream.writeRawData((char*)buffer, compressedSize);
        } else {
            stream << frameByteSize;
            stream.writeRawData((char*)tile.data.data(), frameByteSize);
        }
    }

    file.close();

    return frameId;
}

KisFrameDataSerializer::Frame KisFrameDataSerializer::loadFrame(int frameId, KisTextureTileInfoPoolSP pool)
{
    KisLzfCompression compression;

    QElapsedTimer loadingTime;
    loadingTime.start();

    int loadedFrameId = -1;
    KisFrameDataSerializer::Frame frame;

    qint64 compressionTime = 0;

    const QString framePath = m_d->filePathForFrame(frameId);

    QFile file(framePath);
    KIS_SAFE_ASSERT_RECOVER_NOOP(file.exists());
    if (!file.open(QFile::ReadOnly)) return frame;

    QDataStream stream(&file);

    int numTiles = 0;

    stream >> loadedFrameId;
    stream >> frame.pixelSize;
    stream >> numTiles;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(loadedFrameId == frameId, KisFrameDataSerializer::Frame());



    for (int i = 0; i < numTiles; i++) {
        FrameTile tile(pool);
        stream >> tile.col;
        stream >> tile.row;
        stream >> tile.rect;

        const int frameByteSize = frame.pixelSize * tile.rect.width() * tile.rect.height();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(frameByteSize <= pool->chunkSize(frame.pixelSize),
                                             KisFrameDataSerializer::Frame());

        bool isCompressed = false;
        int inputSize = -1;

        stream >> isCompressed;
        stream >> inputSize;

        if (isCompressed) {
            const int maxBufferSize = compression.outputBufferSize(inputSize);
            quint8 *buffer = m_d->getCompressionBuffer(maxBufferSize);
            stream.readRawData((char*)buffer, inputSize);

            tile.data.allocate(frame.pixelSize);

            QElapsedTimer compTime;
            compTime.start();

            const int decompressedSize =
                compression.decompress(buffer, inputSize, tile.data.data(), frameByteSize);

            compressionTime += compTime.nsecsElapsed();

            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(frameByteSize == decompressedSize,
                                                 KisFrameDataSerializer::Frame());

        } else {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(frameByteSize == inputSize,
                                                 KisFrameDataSerializer::Frame());

            tile.data.allocate(frame.pixelSize);
            stream.readRawData((char*)tile.data.data(), inputSize);
        }

        frame.frameTiles.push_back(std::move(tile));
    }

    file.close();

    return frame;
}

void KisFrameDataSerializer::moveFrame(int srcFrameId, int dstFrameId)
{
    const QString srcFramePath = m_d->filePathForFrame(srcFrameId);
    const QString dstFramePath = m_d->filePathForFrame(dstFrameId);
    KIS_SAFE_ASSERT_RECOVER_RETURN(QFileInfo(srcFramePath).exists());

    KIS_SAFE_ASSERT_RECOVER(!QFileInfo(dstFramePath).exists()) {
        QFile::remove(dstFramePath);
    }

    QFile::rename(srcFramePath, dstFramePath);
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

                if (lhsDataPtr && rhsDataPtr && std::memcmp(lhsDataPtr, rhsDataPtr, pixelSize) != 0) {
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
