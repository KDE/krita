/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LEGACY_TILE_COMPRESSOR_H
#define __KIS_LEGACY_TILE_COMPRESSOR_H

#include "kis_abstract_tile_compressor.h"


class KRITAIMAGE_EXPORT KisLegacyTileCompressor : public KisAbstractTileCompressor
{
public:
    KisLegacyTileCompressor();
    ~KisLegacyTileCompressor() override;

    bool writeTile(KisTileSP tile, KisPaintDeviceWriter &store) override;
    bool readTile(QIODevice *stream, KisTiledDataManager *dm) override;


    void compressTileData(KisTileData *tileData,quint8 *buffer,
                          qint32 bufferSize, qint32 &bytesWritten) override;
    bool decompressTileData(quint8 *buffer, qint32 bufferSize, KisTileData *tileData) override;
    qint32 tileDataBufferSize(KisTileData *tileData) override;

private:
    /**
     * Quite self describing
     */
    qint32 maxHeaderLength();

    /**
     * Writes header into the buffer. Buffer size
     * should be maxHeaderLength() + 1 bytes at least
     * (to fit terminating '\0')
     */
    bool writeHeader(KisTileSP tile, quint8 *buffer);
};

#endif /* __KIS_LEGACY_TILE_COMPRESSOR_H */

