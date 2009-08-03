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

#include <QtGlobal>
#include <QVector>
#include <QVector>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "kis_debug.h"
#include "kis_tile.h"
#include "kis_memento.h"
#include "krita_export.h"

class KisTiledDataManager;
typedef KisSharedPtr<KisTiledDataManager> KisTiledDataManagerSP;

class KisDataManager;
typedef KisSharedPtr<KisDataManager> KisDataManagerSP;

class KisTiledIterator;
class KisTiledRandomAccessor;
class KoStore;

class KisTileDataWrapper;

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

class KRITAIMAGE_EXPORT KisTiledDataManager : public KisShared
{

protected:
    KisTiledDataManager(quint32 pixelSize, const quint8 *defPixel);
    ~KisTiledDataManager();
    KisTiledDataManager(const KisTiledDataManager &dm);
    KisTiledDataManager & operator=(const KisTiledDataManager &dm);


protected:
    // Allow the baseclass of iterators access to the interior
    // derived iterator classes must go through KisTiledIterator
    friend class KisTiledIterator;
    friend class KisTiledRandomAccessor;
protected:

    void setDefaultPixel(const quint8 *defPixel);
    const quint8 * defaultPixel() const {
        return m_defPixel;
    }

    KisMementoSP getMemento();
    void rollback(KisMementoSP memento);
    void rollforward(KisMementoSP memento);
    bool hasCurrentMemento() const {
        return m_currentMemento;
    }

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


    /**
     * Copy the bytes in the paint device into a vector of arrays of bytes,
     * where the number of arrays is the number of channels in the
     * paint device. If the specified area is larger than the paint
     * device's extent, the default pixel will be read.
     */
    QVector<quint8*> readPlanarBytes(QVector<qint32> channelsizes, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Write the data in the separate arrays to the channes. If there
     * are less vectors than channels, the remaining channels will not
     * be copied. If any of the arrays points to 0, the channel in
     * that location will not be touched. If the specified area is
     * larger than the paint device, the paint device will be
     * extended. There are no guards: if the area covers more pixels
     * than there are bytes in the arrays, krita will happily fill
     * your paint device with areas of memory you never wanted to be
     * read. Krita may also crash.
     *
     * XXX: what about undo?
     */
    void writePlanarBytes(QVector<quint8*> planes, QVector<qint32> channelsizes, qint32 x, qint32 y, qint32 w, qint32 h);

    /// Get the number of contiguous columns starting at x, valid for all values
    /// of y between minY and maxY.
    qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const;

    /// Get the number of contiguous rows starting at y, valid for all values
    /// of x between minX and maxX.
    qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const;

    /// Get the row stride at pixel (x, y). This is the number of bytes to add to a
    /// pointer to pixel (x, y) to access (x, y + 1).
    qint32 rowStride(qint32 x, qint32 y) const;

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
    KisTileDataWrapper* pixelPtrSafe(qint32 x, qint32 y, bool writable);
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

