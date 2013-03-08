/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TILE_COMPRESSOR_2_H
#define __KIS_TILE_COMPRESSOR_2_H

#include "kis_abstract_tile_compressor.h"

class KisAbstractCompression;

class KRITAIMAGE_EXPORT KisTileCompressor2 : public KisAbstractTileCompressor
{
public:
    KisTileCompressor2();
    virtual ~KisTileCompressor2();

    void writeTile(KisTileSP tile, KisPaintDeviceWriter &store);
    void readTile(QIODevice *io, KisTiledDataManager *dm);


    void compressTileData(KisTileData *tileData,quint8 *buffer,
                          qint32 bufferSize, qint32 &bytesWritten);
    void decompressTileData(quint8 *buffer, qint32 bufferSize, KisTileData *tileData);
    qint32 tileDataBufferSize(KisTileData *tileData);

private:
    /**
     * Quite self describing
     */
    qint32 maxHeaderLength();

    QString getHeader(KisTileSP tile, qint32 compressedSize);

    void prepareWorkBuffers(qint32 tileDataSize);
    void prepareStreamingBuffer(qint32 tileDataSize);

private:
    static const qint8 RAW_DATA_FLAG = 0;
    static const qint8 COMPRESSED_DATA_FLAG = 1;

private:
    QByteArray m_linearizationBuffer;
    QByteArray m_compressionBuffer;
    QByteArray m_streamingBuffer;
    KisAbstractCompression *m_compression;
    static const QString m_compressionName;
};

#endif /* __KIS_TILE_COMPRESSOR_2_H */

