/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
#include <QRegion>

#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include "config-hash-table-implementaion.h"

//#include "kis_debug.h"
#include "kritaimage_export.h"

#ifdef USE_LOCK_FREE_HASH_TABLE
#include "kis_tile_hash_table2.h"
#else
#include "kis_tile_hash_table.h"
#endif // USE_LOCK_FREE_HASH_TABLE

#include "kis_memento_manager.h"
#include "kis_memento.h"
#include "KisTiledExtentManager.h"

class KisTiledDataManager;
typedef KisSharedPtr<KisTiledDataManager> KisTiledDataManagerSP;

class KisTiledIterator;
class KisTiledRandomAccessor;
class KisPaintDeviceWriter;
class QIODevice;

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
private:
    static const qint32 LEGACY_VERSION = 1;
    static const qint32 CURRENT_VERSION = 2;

protected:
    /*FIXME:*/
public:
    KisTiledDataManager(quint32 pixelSize, const quint8 *defPixel);
    virtual ~KisTiledDataManager();
    KisTiledDataManager(const KisTiledDataManager &dm);
    KisTiledDataManager & operator=(const KisTiledDataManager &dm);


protected:
    // Allow the baseclass of iterators access to the interior
    // derived iterator classes must go through KisTiledIterator
    friend class KisTiledIterator;
    friend class KisBaseIterator;
    friend class KisTiledRandomAccessor;
    friend class KisRandomAccessor2;
    friend class KisStressJob;

public:
    void setDefaultPixel(const quint8 *defPixel);
    const quint8 *defaultPixel() const {
        return m_defaultPixel;
    }

    /**
     * Every iterator fetches both types of tiles all the time: old and new.
     * For projection devices these tiles are **always** the same, but doing
     * two distinct calls makes double pressure on the read-write lock in the
     * hash table.
     *
     * Merging two calls into one allows us to avoid additional tile fetch from
     * the hash table and therefore reduce waiting time.
     */
    inline void getTilesPair(qint32 col, qint32 row, bool writable, KisTileSP *tile, KisTileSP *oldTile) {
        *tile = getTile(col, row, writable);

        bool unused;
        *oldTile = m_mementoManager->getCommitedTile(col, row, unused);

        if (!*oldTile) {
            *oldTile = *tile;
        }
    }

    inline KisTileSP getTile(qint32 col, qint32 row, bool writable) {
        if (writable) {
            bool newTile;
            KisTileSP tile = m_hashTable->getTileLazy(col, row, newTile);
            if (newTile) {
                m_extentManager.notifyTileAdded(col, row);
            }
            return tile;

        } else {
            bool unused;
            return m_hashTable->getReadOnlyTileLazy(col, row, unused);
        }
    }

    inline KisTileSP getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile) {
        return m_hashTable->getReadOnlyTileLazy(col, row, existingTile);
    }

    inline KisTileSP getOldTile(qint32 col, qint32 row, bool &existingTile) {
        KisTileSP tile = m_mementoManager->getCommitedTile(col, row, existingTile);
        return tile ? tile : getReadOnlyTileLazy(col, row, existingTile);
    }

    inline KisTileSP getOldTile(qint32 col, qint32 row) {
        bool unused;
        return getOldTile(col, row, unused);
    }

    KisMementoSP getMemento() {
        QWriteLocker locker(&m_lock);
        KisMementoSP memento = m_mementoManager->getMemento();
        memento->saveOldDefaultPixel(m_defaultPixel, m_pixelSize);
        return memento;
    }

    /**
     * Finishes having already started transaction
     */
    void commit() {
        QWriteLocker locker(&m_lock);

        KisMementoSP memento = m_mementoManager->currentMemento();
        if(memento) {
            memento->saveNewDefaultPixel(m_defaultPixel, m_pixelSize);
        }

        m_mementoManager->commit();
    }

    void rollback(KisMementoSP memento) {
        commit();

        QWriteLocker locker(&m_lock);
        m_mementoManager->rollback(m_hashTable);
        const quint8 *defaultPixel = memento->oldDefaultPixel();
        if(memcmp(m_defaultPixel, defaultPixel, m_pixelSize)) {
            setDefaultPixelImpl(defaultPixel);
        }
        recalculateExtent();
    }
    void rollforward(KisMementoSP memento) {
        commit();

        QWriteLocker locker(&m_lock);
        m_mementoManager->rollforward(m_hashTable);
        const quint8 *defaultPixel = memento->newDefaultPixel();
        if(memcmp(m_defaultPixel, defaultPixel, m_pixelSize)) {
            setDefaultPixelImpl(defaultPixel);
        }
        recalculateExtent();
    }
    bool hasCurrentMemento() const {
        return m_mementoManager->hasCurrentMemento();
        //return true;
    }

    /**
     * Removes all the history that preceds the revision
     * pointed by oldestMemento. That is after calling to
     * purgeHistory(someMemento) you won't be able to do
     * rollback(someMemento) anymore.
     */
    void purgeHistory(KisMementoSP oldestMemento) {
        QWriteLocker locker(&m_lock);
        m_mementoManager->purgeHistory(oldestMemento);
    }

    static void releaseInternalPools();

protected:
    /**
     * Reads and writes the tiles 
     */
    bool write(KisPaintDeviceWriter &store);
    bool read(QIODevice *stream);

    void purge(const QRect& area);

    inline quint32 pixelSize() const {
        return m_pixelSize;
    }

    /* FIXME:*/
public:


    void  extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const;
    void  setExtent(qint32 x, qint32 y, qint32 w, qint32 h);
    QRect extent() const;
    void  setExtent(QRect newRect);

    QRegion region() const;

    void clear(QRect clearRect, quint8 clearValue);
    void clear(QRect clearRect, const quint8 *clearPixel);
    void clear(qint32 x, qint32 y, qint32 w, qint32 h, quint8 clearValue);
    void clear(qint32 x, qint32 y,  qint32 w, qint32 h, const quint8 *clearPixel);
    void clear();

    /**
     * Clones rect from another datamanager. The cloned area will be
     * shared between both datamanagers as much as possible using
     * copy-on-write. Parts of the rect that cannot be shared
     * (cross tiles) are deep-copied,
     */
    void bitBlt(KisTiledDataManager *srcDM, const QRect &rect);

    /**
     * The same as \ref bitBlt(), but reads old data
     */
    void bitBltOldData(KisTiledDataManager *srcDM, const QRect &rect);

    /**
     * Clones rect from another datamanager in a rough and fast way.
     * All the tiles touched by rect will be shared, between both
     * managers, that means it will copy a bigger area than was
     * requested. This method is supposed to be used for bitBlt'ing
     * into temporary paint devices.
     */
    void bitBltRough(KisTiledDataManager *srcDM, const QRect &rect);

    /**
     * The same as \ref bitBltRough(), but reads old data
     */
    void bitBltRoughOldData(KisTiledDataManager *srcDM, const QRect &rect);

    /**
     * write the specified data to x, y. There is no checking on pixelSize!
     */
    void setPixel(qint32 x, qint32 y, const quint8 * data);


    /**
     * Copy the bytes in the specified rect to a vector. The caller is responsible
     * for managing the vector.
     *
     * \param dataRowStride is the step (in bytes) which should be
     *                      added to \p bytes pointer to get to the
     *                      next row
     */
    void readBytes(quint8 * bytes,
                   qint32 x, qint32 y,
                   qint32 w, qint32 h,
                   qint32 dataRowStride = -1) const;
    /**
     * Copy the bytes in the vector to the specified rect. If there are bytes left
     * in the vector after filling the rect, they will be ignored. If there are
     * not enough bytes, the rest of the rect will be filled with the default value
     * given (by default, 0);
     *
     * \param dataRowStride is the step (in bytes) which should be
     *                      added to \p bytes pointer to get to the
     *                      next row
     */
    void writeBytes(const quint8 * bytes,
                    qint32 x, qint32 y,
                    qint32 w, qint32 h,
                    qint32 dataRowStride = -1);

    /**
     * Copy the bytes in the paint device into a vector of arrays of bytes,
     * where the number of arrays is the number of channels in the
     * paint device. If the specified area is larger than the paint
     * device's extent, the default pixel will be read.
     */
    QVector<quint8*> readPlanarBytes(QVector<qint32> channelsizes, qint32 x, qint32 y, qint32 w, qint32 h) const;

    /**
     * Write the data in the separate arrays to the channels. If there
     * are less vectors than channels, the remaining channels will not
     * be copied. If any of the arrays points to 0, the channel in
     * that location will not be touched. If the specified area is
     * larger than the paint device, the paint device will be
     * extended. There are no guards: if the area covers more pixels
     * than there are bytes in the arrays, krita will happily fill
     * your paint device with areas of memory you never wanted to be
     * read. Krita may also crash.
     */
    void writePlanarBytes(QVector<quint8*> planes, QVector<qint32> channelsizes, qint32 x, qint32 y, qint32 w, qint32 h);

    /**
     * Get the number of contiguous columns starting at x, valid for all values
     * of y between minY and maxY.
     */
    qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const;

    /**
     * Get the number of contiguous rows starting at y, valid for all values
     * of x between minX and maxX.
     */
    qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const;

    /**
     * Get the row stride at pixel (x, y). This is the number of bytes to add to a
     * pointer to pixel (x, y) to access (x, y + 1).
     */
    qint32 rowStride(qint32 x, qint32 y) const;

private:
    KisTileHashTable *m_hashTable;
    KisMementoManager *m_mementoManager;
    quint8* m_defaultPixel;
    qint32 m_pixelSize;
    KisTiledExtentManager m_extentManager;

    mutable QReadWriteLock m_lock;

private:
    // Allow compression routines to calculate (col,row) coordinates
    // and pixel size
    friend class KisAbstractTileCompressor;
    friend class KisTileDataWrapper;
    qint32 xToCol(qint32 x) const;
    qint32 yToRow(qint32 y) const;

private:
    void setDefaultPixelImpl(const quint8 *defPixel);

    bool writeTilesHeader(KisPaintDeviceWriter &store, quint32 numTiles);
    bool processTilesHeader(QIODevice *stream, quint32 &numTiles);

    qint32 divideRoundDown(qint32 x, const qint32 y) const;

    void recalculateExtent();

    quint8* duplicatePixel(qint32 num, const quint8 *pixel);

    template<bool useOldSrcData>
        void bitBltImpl(KisTiledDataManager *srcDM, const QRect &rect);
    template<bool useOldSrcData>
        void bitBltRoughImpl(KisTiledDataManager *srcDM, const QRect &rect);

    void writeBytesBody(const quint8 *data,
                        qint32 x, qint32 y,
                        qint32 width, qint32 height,
                        qint32 dataRowStride = -1);
    void readBytesBody(quint8 *data,
                       qint32 x, qint32 y,
                       qint32 width, qint32 height,
                       qint32 dataRowStride = -1) const;

    template <bool allChannelsPresent>
    void writePlanarBytesBody(QVector<quint8*> planes,
                              QVector<qint32> channelsizes,
                              qint32 x, qint32 y, qint32 w, qint32 h);
    QVector<quint8*> readPlanarBytesBody(QVector<qint32> channelsizes,
                                         qint32 x, qint32 y,
                                         qint32 w, qint32 h) const;
public:
    void debugPrintInfo() {
        m_mementoManager->debugPrintInfo();
    }

};

inline qint32 KisTiledDataManager::divideRoundDown(qint32 x, const qint32 y) const
{
    /**
     * Equivalent to the following:
     * -(( -x + (y-1) ) / y)
     */

    return x >= 0 ?
           x / y :
           -(((-x - 1) / y) + 1);
}

inline qint32 KisTiledDataManager::xToCol(qint32 x) const
{
    return divideRoundDown(x, KisTileData::WIDTH);
}

inline qint32 KisTiledDataManager::yToRow(qint32 y) const
{
    return divideRoundDown(y, KisTileData::HEIGHT);
}

// during development the following line helps to check the interface is correct
// it should be safe to keep it here even during normal compilation
//#include "kis_datamanager.h"

#endif // KIS_TILEDDATAMANAGER_H_

