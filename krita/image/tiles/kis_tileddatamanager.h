/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_TILEDDATAMANAGER_H_
#define KIS_TILEDDATAMANAGER_H_

#include <qglobal.h>
#include <q3valuevector.h>

#include <ksharedptr.h>

#include "kis_tile_global.h"
#include "kis_tile.h"
#include "kis_memento.h"
#include "krita_export.h"

class KisTiledDataManager;
typedef KSharedPtr<KisTiledDataManager> KisTiledDataManagerSP;

class KisDataManager;
typedef KSharedPtr<KisDataManager> KisDataManagerSP;

class KisTiledIterator;
class KoStore;

class KisTileDataWrapper : public KShared {
    KisTile* m_tile;
    qint32 m_offset;
public:
    KisTileDataWrapper(KisTile* tile, qint32 offset);
    virtual ~KisTileDataWrapper();
    quint8* data() const { return m_tile->data() + m_offset; }
};

typedef KSharedPtr<KisTileDataWrapper> KisTileDataWrapperSP;

/**
 * KisTiledDataManager implements the interface that KisDataManager defines
 *
 * The interface definition is enforced by KisDataManager calling all the methods
 * which must also be defined in KisTiledDataManager. It is not allowed to change the interface
 * as other datamangers may also rely on the same interface.
 *
 * * Storing undo/redo data
 * * Offering ordered and unordered iterators over rects of pixels
 * * (eventually) efficiently loading and saving data in a format
 * that may allow deferred loading.
 *
 * A datamanager knows nothing about the type of pixel data except
 * how many quint8's a single pixel takes.
 */

class KRITAIMAGE_EXPORT KisTiledDataManager : public KShared {

protected:
    KisTiledDataManager(quint32 pixelSize, const quint8 *defPixel);
    ~KisTiledDataManager();
    KisTiledDataManager(const KisTiledDataManager &dm);
    KisTiledDataManager & operator=(const KisTiledDataManager &dm);


protected:
    // Allow the baseclass of iterators acces to the interior
    // derived iterator classes must go through KisTiledIterator
    friend class KisTiledIterator;
    friend class KisTiledRandomAccessor;
protected:

    void setDefaultPixel(const quint8 *defPixel);
    const quint8 * defaultPixel() const { return m_defPixel;};

    KisMementoSP getMemento();
    void rollback(KisMementoSP memento);
    void rollforward(KisMementoSP memento);

    // For debugging use.
    bool hasCurrentMemento() const { return !m_currentMemento.isNull(); }

protected:
    /**
     * Reads and writes the tiles from/onto a KoStore (which is simply a file within a zip file)
     *
     */
    bool write(KoStore *store);
    bool read(KoStore *store);

protected:

    quint32 pixelSize();

    void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;
    QRect extent() const;

    void setExtent(qint32 x, qint32 y, qint32 w, qint32 h);

protected:

    void clear(qint32 x, qint32 y, qint32 w, qint32 h, quint8 clearValue);
    void clear(qint32 x, qint32 y,  qint32 w, qint32 h, const quint8 *clearPixel);
    void clear();


protected:

    void paste(KisDataManagerSP data,  qint32 sx, qint32 sy, qint32 dx, qint32 dy,
                qint32 w, qint32 h);


protected:


    /**
     * Get a read-only pointer to pixel (x, y).
     */
    const quint8* pixel(qint32 x, qint32 y);

    /**
     * Get a read-write pointer to pixel (x, y).
     */
    quint8* writablePixel(qint32 x, qint32 y);

    /**
     * write the specified data to x, y. There is no checking on pixelSize!
     */
    void setPixel(qint32 x, qint32 y, const quint8 * data);


    /**
     * Copy the bytes in the specified rect to a vector. The caller is responsible
     * for managing the vector.
     */
    void readBytes(quint8 * bytes,
               qint32 x, qint32 y,
               qint32 w, qint32 h);
    /**
     * Copy the bytes in the vector to the specified rect. If there are bytes left
     * in the vector after filling the rect, they will be ignored. If there are
     * not enough bytes, the rest of the rect will be filled with the default value
     * given (by default, 0);
     */
    void writeBytes(const quint8 * bytes,
            qint32 x, qint32 y,
            qint32 w, qint32 h);

    /// Get the number of contiguous columns starting at x, valid for all values
    /// of y between minY and maxY.
    qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY);

    /// Get the number of contiguous rows starting at y, valid for all values
    /// of x between minX and maxX.
    qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX);

    /// Get the row stride at pixel (x, y). This is the number of bytes to add to a
    /// pointer to pixel (x, y) to access (x, y + 1).
    qint32 rowStride(qint32 x, qint32 y);

    // For debugging use
    qint32 numTiles() const;

private:

    quint32 m_pixelSize;
    quint32 m_numTiles;
    KisTile *m_defaultTile;
    KisTile **m_hashTable;
    KisMementoSP m_currentMemento;
    qint32 m_extentMinX;
    qint32 m_extentMinY;
    qint32 m_extentMaxX;
    qint32 m_extentMaxY;
    quint8 *m_defPixel;

private:

    void ensureTileMementoed(qint32 col, qint32 row, quint32 tileHash, const KisTile *refTile);
    KisTile *getOldTile(qint32 col, qint32 row, KisTile *def);
    KisTile *getTile(qint32 col, qint32 row, bool writeAccess);
    quint32 calcTileHash(qint32 col, qint32 row);
    void updateExtent(qint32 col, qint32 row);
    void recalculateExtent();
    void deleteTiles(const KisMemento::DeletedTile *deletedTileList);
    qint32 xToCol(qint32 x) const;
    qint32 yToRow(qint32 y) const;
    void getContiguousColumnsAndRows(qint32 x, qint32 y, qint32 *columns, qint32 *rows);
    quint8* pixelPtr(qint32 x, qint32 y, bool writable);
    KisTileDataWrapperSP pixelPtrSafe(qint32 x, qint32 y, bool writable);
};


inline quint32 KisTiledDataManager::pixelSize()
{
    return m_pixelSize;
}

inline qint32 KisTiledDataManager::xToCol(qint32 x) const
{
    if (x >= 0) {
        return x / KisTile::WIDTH;
    } else {
        return -(((-x - 1) / KisTile::WIDTH) + 1);
    }
}

inline qint32 KisTiledDataManager::yToRow(qint32 y) const
{
    if (y >= 0) {
        return y / KisTile::HEIGHT;
    } else {
        return -(((-y - 1) / KisTile::HEIGHT) + 1);
    }
}

// during development the following line helps to check the interface is correct
// it should be safe to keep it here even during normal compilation
#include "kis_datamanager.h"

#endif // KIS_TILEDDATAMANAGER_H_

