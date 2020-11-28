/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_legacy_tile_compressor.h"
#include "kis_paint_device_writer.h"
#include <QIODevice>

#define TILE_DATA_SIZE(pixelSize) ((pixelSize) * KisTileData::WIDTH * KisTileData::HEIGHT)

KisLegacyTileCompressor::KisLegacyTileCompressor()
{
}

KisLegacyTileCompressor::~KisLegacyTileCompressor()
{
}

bool KisLegacyTileCompressor::writeTile(KisTileSP tile, KisPaintDeviceWriter &store)
{
    const qint32 tileDataSize = TILE_DATA_SIZE(tile->pixelSize());

    const qint32 bufferSize = maxHeaderLength() + 1;
    QScopedArrayPointer<quint8> headerBuffer(new quint8[bufferSize]);

    bool retval = writeHeader(tile, headerBuffer.data());
    Q_ASSERT(retval);  // currently the code returns true unconditionally
    if (!retval) {
        return false;
    }

    store.write((char *)headerBuffer.data(), strlen((char *)headerBuffer.data()));

    tile->lockForRead();
    retval = store.write((char *)tile->data(), tileDataSize);
    tile->unlockForRead();

    return retval;
}

bool KisLegacyTileCompressor::readTile(QIODevice *stream, KisTiledDataManager *dm)
{
    const qint32 tileDataSize = TILE_DATA_SIZE(pixelSize(dm));

    const qint32 bufferSize = maxHeaderLength() + 1;
    quint8 *headerBuffer = new quint8[bufferSize];

    qint32 x, y;
    qint32 width, height;

    stream->readLine((char *)headerBuffer, bufferSize);
    sscanf((char *) headerBuffer, "%d,%d,%d,%d", &x, &y, &width, &height);

    qint32 row = yToRow(dm, y);
    qint32 col = xToCol(dm, x);

    KisTileSP tile = dm->getTile(col, row, true);

    tile->lockForWrite();
    stream->read((char *)tile->data(), tileDataSize);
    tile->unlockForWrite();

    return true;
}

void KisLegacyTileCompressor::compressTileData(KisTileData *tileData,
                                               quint8 *buffer,
                                               qint32 bufferSize,
                                               qint32 &bytesWritten)
{
    bytesWritten = 0;
    const qint32 tileDataSize = TILE_DATA_SIZE(tileData->pixelSize());
    Q_UNUSED(bufferSize);
    Q_ASSERT(bufferSize >= tileDataSize);
    memcpy(buffer, tileData->data(), tileDataSize);
    bytesWritten += tileDataSize;
}

bool KisLegacyTileCompressor::decompressTileData(quint8 *buffer,
                                                 qint32 bufferSize,
                                                 KisTileData *tileData)
{
    const qint32 tileDataSize = TILE_DATA_SIZE(tileData->pixelSize());
    if (bufferSize >= tileDataSize) {
        memcpy(tileData->data(), buffer, tileDataSize);
        return true;
    }
    return false;
}

qint32 KisLegacyTileCompressor::tileDataBufferSize(KisTileData *tileData)
{
    return TILE_DATA_SIZE(tileData->pixelSize());
}

inline qint32 KisLegacyTileCompressor::maxHeaderLength()
{
    static const qint32 LEGACY_MAGIC_NUMBER = 79;
    return LEGACY_MAGIC_NUMBER;
}

inline bool KisLegacyTileCompressor::writeHeader(KisTileSP tile,
                                                 quint8 *buffer)
{
    qint32 x, y;
    qint32 width, height;

    tile->extent().getRect(&x, &y, &width, &height);
    sprintf((char *)buffer, "%d,%d,%d,%d\n", x, y, width, height);

    return true;
}
