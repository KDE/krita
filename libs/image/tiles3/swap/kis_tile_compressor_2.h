/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TILE_COMPRESSOR_2_H
#define __KIS_TILE_COMPRESSOR_2_H

#include "kis_abstract_tile_compressor.h"

class KisAbstractCompression;

class KRITAIMAGE_EXPORT KisTileCompressor2 : public KisAbstractTileCompressor
{
public:
    KisTileCompressor2();
    ~KisTileCompressor2() override;

    bool writeTile(KisTileSP tile, KisPaintDeviceWriter &store) override;
    bool readTile(QIODevice *io, KisTiledDataManager *dm) override;


    void compressTileData(KisTileData *tileData,quint8 *buffer,
                          qint32 bufferSize, qint32 &bytesWritten) override;
    bool decompressTileData(quint8 *buffer, qint32 bufferSize, KisTileData *tileData) override;
    qint32 tileDataBufferSize(KisTileData *tileData) override;

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

