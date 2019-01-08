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

#ifndef __KIS_ABSTRACT_TILE_COMPRESSOR_H
#define __KIS_ABSTRACT_TILE_COMPRESSOR_H

#include "kritaimage_export.h"
#include "../kis_tile.h"
#include "../kis_tiled_data_manager.h"

class KisPaintDeviceWriter;
/**
 * Base class for compressing a tile and wrapping it with a header
 */

class KisAbstractTileCompressor;
typedef KisSharedPtr<KisAbstractTileCompressor> KisAbstractTileCompressorSP;

class KRITAIMAGE_EXPORT KisAbstractTileCompressor : public KisShared
{
public:
    KisAbstractTileCompressor();
    virtual ~KisAbstractTileCompressor();

public:

    /**
     * Compresses the \a tile and writes it into the \a stream.
     * Used by datamanager in load/save routines
     *
     * \see compressTile()
     */
    virtual bool writeTile(KisTileSP tile, KisPaintDeviceWriter &store) = 0;

    /**
     * Decompresses the \a tile from the \a stream.
     * Used by datamanager in load/save routines
     *
     * \see decompressTile()
     */
    virtual bool readTile(QIODevice *stream, KisTiledDataManager *dm) = 0;

    /**
     * Compresses a \p tileData and writes it into the \p buffer.
     * The buffer must be at least tileDataBufferSize() bytes long.
     * Actual number of bytes written is returned using out-parameter
     * \p bytesWritten
     *
     * \param tileData an existing tile data. It should be created
     * and acquired by the caller.
     * \param buffer the buffer
     * \param bufferSize the size of the buffer
     * \param bytesWritten the number of written bytes
     *
     * \see tileDataBufferSize()
     */
    virtual void compressTileData(KisTileData *tileData,quint8 *buffer,
                                  qint32 bufferSize, qint32 &bytesWritten) = 0;

    /**
     * Decompresses a \p tileData from a given \p buffer.
     *
     * \param buffer the buffer
     * \param bufferSize the size of the buffer
     * \param tileData an existing tile data where the result
     * will be written to. It should be created and acquired
     * by the caller.
     *
     */
    virtual bool decompressTileData(quint8 *buffer, qint32 bufferSize,
                                    KisTileData *tileData) = 0;

    /**
     * Return the number of bytes needed for compressing one tile
     */
    virtual qint32 tileDataBufferSize(KisTileData *tileData) = 0;

protected:
    inline qint32 xToCol(KisTiledDataManager *dm, qint32 x) {
        return dm->xToCol(x);
    }

    inline qint32 yToRow(KisTiledDataManager *dm, qint32 y) {
        return dm->yToRow(y);
    }

    inline qint32 pixelSize(KisTiledDataManager *dm) {
        return dm->pixelSize();
    }
};

#endif /* __KIS_ABSTRACT_TILE_COMPRESSOR_H */

