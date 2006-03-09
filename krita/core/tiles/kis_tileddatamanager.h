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
#include <qvaluevector.h>

#include <ksharedptr.h>

#include "kis_tile_global.h"
#include "kis_tile.h"
#include "kis_memento.h"

class KisTiledDataManager;
typedef KSharedPtr<KisTiledDataManager> KisTiledDataManagerSP;

class KisDataManager;
typedef KSharedPtr<KisDataManager> KisDataManagerSP;

class KisTiledIterator;
class KoStore;

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
 * how many Q_UINT8's a single pixel takes.
 */

class KisTiledDataManager : public KShared {

protected:
    KisTiledDataManager(Q_UINT32 pixelSize, const Q_UINT8 *defPixel);
    ~KisTiledDataManager();
    KisTiledDataManager(const KisTiledDataManager &dm);
    KisTiledDataManager & operator=(const KisTiledDataManager &dm);


protected:
    // Allow the baseclass of iterators acces to the interior
    // derived iterator classes must go through KisTiledIterator
    friend class KisTiledIterator;

protected:

    void setDefaultPixel(const Q_UINT8 *defPixel);
    const Q_UINT8 * defaultPixel() const { return m_defPixel;};

    KisMementoSP getMemento();
    void rollback(KisMementoSP memento);
    void rollforward(KisMementoSP memento);

    // For debugging use.
    bool hasCurrentMemento() const { return m_currentMemento != 0; }

protected:
    /**
     * Reads and writes the tiles from/onto a KoStore (which is simply a file within a zip file)
     *
     */
    bool write(KoStore *store);
    bool read(KoStore *store);

protected:

    Q_UINT32 pixelSize();

    void extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const;
    QRect extent() const;

    void setExtent(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

protected:

    void clear(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h, Q_UINT8 clearValue);
    void clear(Q_INT32 x, Q_INT32 y,  Q_INT32 w, Q_INT32 h, const Q_UINT8 *clearPixel);
    void clear();


protected:

    void paste(KisDataManagerSP data,  Q_INT32 sx, Q_INT32 sy, Q_INT32 dx, Q_INT32 dy,
                Q_INT32 w, Q_INT32 h);


protected:


    /**
     * Get a read-only pointer to pixel (x, y).
     */
    const Q_UINT8* pixel(Q_INT32 x, Q_INT32 y);

    /**
     * Get a read-write pointer to pixel (x, y).
     */
    Q_UINT8* writablePixel(Q_INT32 x, Q_INT32 y);

    /**
     * write the specified data to x, y. There is no checking on pixelSize!
     */
    void setPixel(Q_INT32 x, Q_INT32 y, const Q_UINT8 * data);


    /**
     * Copy the bytes in the specified rect to a vector. The caller is responsible
     * for managing the vector.
     */
    void readBytes(Q_UINT8 * bytes,
               Q_INT32 x, Q_INT32 y,
               Q_INT32 w, Q_INT32 h);
    /**
     * Copy the bytes in the vector to the specified rect. If there are bytes left
     * in the vector after filling the rect, they will be ignored. If there are
     * not enough bytes, the rest of the rect will be filled with the default value
     * given (by default, 0);
     */
    void writeBytes(const Q_UINT8 * bytes,
            Q_INT32 x, Q_INT32 y,
            Q_INT32 w, Q_INT32 h);

    /// Get the number of contiguous columns starting at x, valid for all values
    /// of y between minY and maxY.
    Q_INT32 numContiguousColumns(Q_INT32 x, Q_INT32 minY, Q_INT32 maxY);

    /// Get the number of contiguous rows starting at y, valid for all values
    /// of x between minX and maxX.
    Q_INT32 numContiguousRows(Q_INT32 y, Q_INT32 minX, Q_INT32 maxX);

    /// Get the row stride at pixel (x, y). This is the number of bytes to add to a
    /// pointer to pixel (x, y) to access (x, y + 1).
    Q_INT32 rowStride(Q_INT32 x, Q_INT32 y);

    // For debugging use
    Q_INT32 numTiles() const;

private:

    Q_UINT32 m_pixelSize;
    Q_UINT32 m_numTiles;
    KisTile *m_defaultTile;
    KisTile **m_hashTable;
    KisMementoSP m_currentMemento;
    Q_INT32 m_extentMinX;
    Q_INT32 m_extentMinY;
    Q_INT32 m_extentMaxX;
    Q_INT32 m_extentMaxY;
    Q_UINT8 *m_defPixel;

private:

    void ensureTileMementoed(Q_INT32 col, Q_INT32 row, Q_UINT32 tileHash, const KisTile *refTile);
    KisTile *getOldTile(Q_INT32 col, Q_INT32 row, KisTile *def);
    KisTile *getTile(Q_INT32 col, Q_INT32 row, bool writeAccess);
    Q_UINT32 calcTileHash(Q_INT32 col, Q_INT32 row);
    void updateExtent(Q_INT32 col, Q_INT32 row);
    void recalculateExtent();
    void deleteTiles(const KisMemento::DeletedTile *deletedTileList);
    Q_INT32 xToCol(Q_INT32 x) const;
    Q_INT32 yToRow(Q_INT32 y) const;
    void getContiguousColumnsAndRows(Q_INT32 x, Q_INT32 y, Q_INT32 *columns, Q_INT32 *rows);
    Q_UINT8* pixelPtr(Q_INT32 x, Q_INT32 y, bool writable);
};


inline Q_UINT32 KisTiledDataManager::pixelSize()
{
    return m_pixelSize;
}

inline Q_INT32 KisTiledDataManager::xToCol(Q_INT32 x) const
{
    if (x >= 0) {
        return x / KisTile::WIDTH;
    } else {
        return -(((-x - 1) / KisTile::WIDTH) + 1);
    }
}

inline Q_INT32 KisTiledDataManager::yToRow(Q_INT32 y) const
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

