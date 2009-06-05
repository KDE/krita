/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
 *            (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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

#include <QtConcurrentMap>
#include <QRect>
#include <QVector>

#include "kis_tile.h"
#include "kis_tiled_data_manager.h"
#include "kis_tiled_data_manager_p.h"

//#include <KoStore.h>

//#include "kis_global.h"
//#include "kis_debug.h"


//#include "kis_memento.h"
//#include "kis_tilemanager.h"

/* The data area is divided into tiles each say 64x64 pixels (defined at compiletime)
 * The tiles are laid out in a matrix that can have negative indexes.
 * The matrix grows automatically if needed (a call for writeacces to a tile 
 * outside the current extent)
 * Even though the matrix has grown it may still not contain tiles at specific positions.
 * They are created on demand
 */

KisTiledDataManager::KisTiledDataManager(quint32 pixelSize, 
                                         const quint8 *defaultPixel)
{
    /* FIXME: dirty hack */
    m_defaultTileData = 0;
    setDefaultTileData(globalTileDataStore.createDefaultTileData(pixelSize, defaultPixel));

    //m_extent.setCoords(qint32_MAX, qint32_MAX, 
    //                   qint32_MIN, qint32_MIN);
    m_extent.setCoords(0,0,0,0);

/* FIXME: What's this?
    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
*/
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager &dm) 
    : KisShared(dm),
      m_extent(dm.m_extent),
      m_hashTable(dm.m_hashTable)
{
    /* FIXME: dirty hack (once more) */
    m_defaultTileData = 0;
    setDefaultTileData(dm.m_defaultTileData);
}

KisTiledDataManager::~KisTiledDataManager()
{
    globalTileDataStore.releaseTileData(m_defaultTileData);
}

void KisTiledDataManager::setDefaultTileData(KisTileData *td)
{
    if(m_defaultTileData) {
        globalTileDataStore.releaseTileData(m_defaultTileData);
        m_defaultTileData=0;
    }
    
    globalTileDataStore.acquireTileData(td);
    m_defaultTileData=td;
    m_hashTable.setDefaultTileData(td);
}

void KisTiledDataManager::setDefaultPixel(const quint8 *defaultPixel)
{
    KisTileData *td = globalTileDataStore.createDefaultTileData(pixelSize(),
                                                                defaultPixel);
    setDefaultTileData(td);
}


bool KisTiledDataManager::write(KoStore *store)
{
    Q_UNUSED(store);
    /* FIXME: */
    return true;
}
bool KisTiledDataManager::read(KoStore *store)
{
    Q_UNUSED(store);
    /* FIXME: */
    return true;
}

void KisTiledDataManager::extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    m_extent.getRect(&x, &y, &w, &h);
}

QRect KisTiledDataManager::extent() const
{
    return m_extent;
}

/*****************************************************************************/
    /* FIXME: */
/*
void KisTiledDataManager::setExtent(qint32 x, qint32 y, qint32 w, qint32 h)
{
    QRect newRect = QRect(x, y, w, h).normalized();
    //printRect("newRect", newRect);
    QRect oldRect = extent();
    //printRect("oldRect", oldRect);

    // Do nothing if the desired size is bigger than we currently are: 
    // that is handled by the autoextending automatically
    if (newRect.contains(oldRect)) return;

    // Loop through all tiles, if a tile is wholly outside the extent, add to the memento, then delete it,
    // if the tile is partially outside the extent, clear the outside pixels to the default pixel.
    for (int tileHash = 0; tileHash < 1024; tileHash++) {
        KisTile *tile = m_hashTable[tileHash];
        KisTile *previousTile = 0;

        while (tile) {
            QRect tileRect = QRect(tile->getCol() * KisTile::WIDTH, tile->getRow() * KisTile::HEIGHT, KisTile::WIDTH, KisTile::HEIGHT);
            //printRect("tileRect", tileRect);

            if (newRect.contains(tileRect)) {
                // Completely inside, do nothing
                previousTile = tile;
                tile = tile->getNext();
            } else {
                ensureTileMementoed(tile->getCol(), tile->getRow(), tileHash, tile);

                if (newRect.intersects(tileRect)) {

                    // Create the intersection of the tile and new rect
                    QRect intersection = newRect.intersect(tileRect);
                    //printRect("intersection", intersection);
                    intersection.setRect(intersection.x() - tileRect.x(), intersection.y() - tileRect.y(), intersection.width(), intersection.height());

                    // This can be done a lot more efficiently, no doubt, by clearing runs of pixels to the left and the right of
                    // the intersecting line.
                    tile->addReader();
                    for (int y = 0; y < KisTile::HEIGHT; ++y) {
                        for (int x = 0; x < KisTile::WIDTH; ++x) {
                            if (!intersection.contains(x, y)) {
                                tile->addReader();
                                quint8 * ptr = tile->data();
                                if (ptr)  {
                                    ptr += m_pixelSize * (y * KisTile::WIDTH + x);
                                    memcpy(ptr, m_defPixel, m_pixelSize);
                                }
                                tile->removeReader();
                            }
                        }
                    }
                    tile->removeReader();
                    previousTile = tile;
                    tile = tile->getNext();
                } else {
                    KisTile *deltile = tile;
                    tile = tile->getNext();

                    m_numTiles--;

                    if (previousTile)
                        previousTile->setNext(tile);
                    else
                        m_hashTable[tileHash] = tile;
                    delete deltile;
                }
            }
        }
    }

    // Set the extent correctly
    m_extentMinX = x;
    m_extentMinY = y;
    m_extentMaxX = x + w - 1;
    m_extentMaxY = y + h - 1;
}

void KisTiledDataManager::recalculateExtent()
{
    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;

    // Loop through all tiles.
    for (int tileHash = 0; tileHash < 1024; tileHash++) {
        const KisTile *tile = m_hashTable[tileHash];

        while (tile) {
            updateExtent(tile->getCol(), tile->getRow());
            tile = tile->getNext();
        }
    }
}

void KisTiledDataManager::clear(qint32 x, qint32 y, qint32 w, qint32 h, quint8 clearValue)
{
    if (w < 1 || h < 1) {
        return;
    }

    qint32 firstColumn = xToCol(x);
    qint32 lastColumn = xToCol(x + w - 1);

    qint32 firstRow = yToRow(y);
    qint32 lastRow = yToRow(y + h - 1);

    QRect clearRect(x, y, w, h);

    const quint32 rowStride = KisTile::WIDTH * m_pixelSize;

    for (qint32 row = firstRow; row <= lastRow; ++row) {
        for (qint32 column = firstColumn; column <= lastColumn; ++column) {

            KisTile *tile = getTile(column, row, true);
            QRect tileRect = tile->extent();

            QRect clearTileRect = clearRect & tileRect;

            tile->addReader();
            if (clearTileRect == tileRect) {
                // Clear whole tile
                memset(tile->data(), clearValue, KisTile::WIDTH * KisTile::HEIGHT * m_pixelSize);
            } else {
                quint8 *dst = tile->data();
                if (dst) {
                    quint32 rowsRemaining = clearTileRect.height();
                    dst += m_pixelSize * ((clearTileRect.y() - tileRect.y()) * KisTile::WIDTH + (clearTileRect.x() - tileRect.x()));
                    while (rowsRemaining > 0) {
                        memset(dst, clearValue, clearTileRect.width() * m_pixelSize);
                        dst += rowStride;
                        --rowsRemaining;
                    }
                }
            }
            tile->removeReader();
        }
    }
}

void KisTiledDataManager::clear(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *clearPixel)
{
    Q_ASSERT(clearPixel != 0);

    if (clearPixel == 0 || w < 1 || h < 1) {
        return;
    }

    bool pixelBytesAreTheSame = true;

    for (quint32 i = 0; i < m_pixelSize; ++i) {
        if (clearPixel[i] != clearPixel[0]) {
            pixelBytesAreTheSame = false;
            break;
        }
    }

    if (pixelBytesAreTheSame) {
        clear(x, y, w, h, clearPixel[0]);
    } else {

        qint32 firstColumn = xToCol(x);
        qint32 lastColumn = xToCol(x + w - 1);

        qint32 firstRow = yToRow(y);
        qint32 lastRow = yToRow(y + h - 1);

        QRect clearRect(x, y, w, h);

        const quint32 rowStride = KisTile::WIDTH * m_pixelSize;

        quint8 *clearPixelData = 0;

        if (w >= KisTile::WIDTH && h >= KisTile::HEIGHT) {

            // There might be a whole tile to be cleared so generate a cleared tile.
            clearPixelData = new quint8[KisTile::WIDTH * KisTile::HEIGHT * m_pixelSize];

            quint8 *dst = clearPixelData;
            quint32 pixelsRemaining = KisTile::WIDTH;

            // Generate one row
            while (pixelsRemaining > 0) {
                memcpy(dst, clearPixel, m_pixelSize);
                dst += m_pixelSize;
                --pixelsRemaining;
            }

            quint32 rowsRemaining = KisTile::HEIGHT - 1;

            // Copy to the rest of the rows.
            while (rowsRemaining > 0) {
                memcpy(dst, clearPixelData, rowStride);
                dst += rowStride;
                --rowsRemaining;
            }

        } else {

            // Generate one row
            quint32 maxRunLength = qMin(w, KisTile::WIDTH);

            clearPixelData = new quint8[maxRunLength * m_pixelSize];

            quint8 *dst = clearPixelData;
            quint32 pixelsRemaining = maxRunLength;

            while (pixelsRemaining > 0) {
                memcpy(dst, clearPixel, m_pixelSize);
                dst += m_pixelSize;
                --pixelsRemaining;
            }
        }

        for (qint32 row = firstRow; row <= lastRow; ++row) {
            for (qint32 column = firstColumn; column <= lastColumn; ++column) {

                KisTile *tile = getTile(column, row, true);
                QRect tileRect = tile->extent();

                QRect clearTileRect = clearRect & tileRect;

                if (clearTileRect == tileRect) {

                    // Clear whole tile
                    tile->addReader();
                    memcpy(tile->data(), clearPixelData, KisTile::WIDTH * KisTile::HEIGHT * m_pixelSize);
                    tile->removeReader();
                } else {

                    quint32 rowsRemaining = clearTileRect.height();
                    tile->addReader();
                    quint8 *dst = tile->data();
                    if (dst)
                        dst += m_pixelSize * ((clearTileRect.y() - tileRect.y()) * KisTile::WIDTH + (clearTileRect.x() - tileRect.x()));


                    while (rowsRemaining > 0) {
                        memcpy(dst, clearPixelData, clearTileRect.width() * m_pixelSize);
                        dst += rowStride;
                        --rowsRemaining;
                    }
                    tile->removeReader();
                }
            }
        }

        delete [] clearPixelData;
    }
}

void KisTiledDataManager::clear()
{
    // Loop through all tiles, add to the memento, then delete it,
    for (int tileHash = 0; tileHash < 1024; tileHash++) {
        const KisTile *tile = m_hashTable[tileHash];

        while (tile) {
            ensureTileMementoed(tile->getCol(), tile->getRow(), tileHash, tile);

            const KisTile *deltile = tile;
            tile = tile->getNext();

            delete deltile;
        }
        m_hashTable[tileHash] = 0;
    }

    m_numTiles = 0;

    // Set the extent correctly
    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}
*/

/*
KisMementoSP KisTiledDataManager::getMemento()
{
    m_currentMemento = new KisMemento(m_pixelSize);
    Q_CHECK_PTR(m_currentMemento);

    memcpy(m_currentMemento->m_defPixel, m_defPixel, m_pixelSize);

    return m_currentMemento;
}

void KisTiledDataManager::rollback(KisMementoSP memento)
{
    if (!memento) return;
    //Q_ASSERT(memento != 0);

    if (m_currentMemento) {
        // Undo means our current memento is no longer valid so remove it.
        m_currentMemento = 0;
    }

    // Rollback means restoring all of the tiles in the memento to our hashtable.

    // But first clear the memento redo hashtable.
    // This is necessary as new changes might have been done since last rollback (automatic filters)
    for (int i = 0; i < 1024; i++) {
        memento->deleteAll(memento->m_redoHashTable[i]);
        memento->m_redoHashTable[i] = 0;
    }

    // Also clear the table of deleted tiles
    memento->clearTilesToDeleteOnRedo();

    // Now on to the real rollback

    memcpy(memento->m_redoDefPixel, m_defPixel, m_pixelSize);
    setDefaultPixel(memento->m_defPixel);

    for (int i = 0; i < 1024; i++) {
        KisTile *tile = memento->m_hashTable[i];

        while (tile) {
            // The memento has a tile stored that we need to roll back
            // Now find the corresponding one in our hashtable
            KisTile *curTile = m_hashTable[i];
            KisTile *preTile = 0;
            while (curTile) {
                if (curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol()) {
                    break;
                }
                preTile = curTile;
                curTile = curTile->getNext();
            }

            if (curTile) {
                // Remove it from our hashtable
                if (preTile)
                    preTile->setNext(curTile->getNext());
                else
                    m_hashTable[i] = curTile->getNext();

                m_numTiles--;

                // And put it in the redo hashtable of the memento
                curTile->setNext(memento->m_redoHashTable[i]);
                memento->m_redoHashTable[i] = curTile;
            } else {
                memento->addTileToDeleteOnRedo(tile->getCol(), tile->getRow());
                // As we are pratically adding a new tile we need to update the extent
                updateExtent(tile->getCol(), tile->getRow());
            }

            // Put a copy of the memento tile into our hashtable
            curTile = new KisTile(*tile);
            Q_CHECK_PTR(curTile);
            m_numTiles++;

            curTile->setNext(m_hashTable[i]);
            m_hashTable[i] = curTile;

            tile = tile->getNext();
        }
    }

    if (memento->tileListToDeleteOnUndo() != 0) {
        // XXX: We currently add these tiles above, only to delete them again here.
        deleteTiles(memento->tileListToDeleteOnUndo());
    }
}

void KisTiledDataManager::rollforward(KisMementoSP memento)
{
    if (memento.isNull()) return;
    //Q_ASSERT(memento != 0);

    if (!m_currentMemento.isNull()) {
        // Redo means our current memento is no longer valid so remove it.
        m_currentMemento = 0;
    }

    // Rollforward means restoring all of the tiles in the memento's redo to our hashtable.

    setDefaultPixel(memento->m_redoDefPixel);

    for (int i = 0; i < 1024; i++) {
        KisTile *tile = memento->m_redoHashTable[i];

        while (tile) {
            // The memento has a tile stored that we need to roll forward
            // Now find the corresponding one in our hashtable
            KisTile *curTile = m_hashTable[i];
            KisTile *preTile = 0;
            while (curTile) {
                if (curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol()) {
                    break;
                }
                preTile = curTile;
                curTile = curTile->getNext();
            }

            if (curTile) {
                // Remove it from our hashtable
                if (preTile)
                    preTile->setNext(curTile->getNext());
                else
                    m_hashTable[i] = curTile->getNext();

                // And delete it (it's equal to the one stored in the memento's undo)
                m_numTiles--;
                delete curTile;
            }

            // Put a copy of the memento tile into our hashtable
            curTile = new KisTile(*tile);
            Q_CHECK_PTR(curTile);

            curTile->setNext(m_hashTable[i]);
            m_hashTable[i] = curTile;
            m_numTiles++;
            updateExtent(curTile->getCol(), curTile->getRow());

            tile = tile->getNext();
        }
    }

    // Roll forward also means re-deleting the tiles that was deleted but restored by the undo
    if (memento->tileListToDeleteOnRedo() != 0) {
        deleteTiles(memento->tileListToDeleteOnRedo());
    }
}
*/
/*
void KisTiledDataManager::deleteTiles(const KisMemento::DeletedTile *d)
{
    while (d) {
        quint32 tileHash = calcTileHash(d->col(), d->row());
        KisTile *curTile = m_hashTable[tileHash];
        KisTile *preTile = 0;
        while (curTile) {
            if (curTile->getRow() == d->row() && curTile->getCol() == d->col()) {
                break;
            }
            preTile = curTile;
            curTile = curTile->getNext();
        }
        if (curTile) {
            // Remove it from our hashtable
            if (preTile)
                preTile->setNext(curTile->getNext());
            else
                m_hashTable[tileHash] = curTile->getNext();

            // And delete it (it's equal to the one stored in the memento's undo)
            m_numTiles--;
            delete curTile;
        }
        d = d->next();
    }

    recalculateExtent();
}
*/


KisTileDataWrapper KisTiledDataManager::pixelPtr(qint32 x, qint32 y, 
			    enum KisTileDataWrapper::accessType type)
{
    const qint32 col = xToCol(x);
    const qint32 row = yToRow(y);
    
    /* FIXME: Always positive? */
    const qint32 xInTile = x - col * KisTileData::WIDTH;
    const qint32 yInTile = y - row * KisTileData::HEIGHT;

    const qint32 pixelIndex = xInTile + yInTile * KisTileData::WIDTH;

    return KisTileDataWrapper(m_hashTable.getTileLazy(col, row),
			      pixelIndex*pixelSize(),
			      type);
}

void KisTiledDataManager::setPixel(qint32 x, qint32 y, const quint8 * data)
{
    KisTileDataWrapper tw = pixelPtr(x, y, KisTileDataWrapper::WRITE);
    memcpy(tw.data(), data, pixelSize());
}

void KisTiledDataManager::writeBytes(const quint8 *data,
				     qint32 x, qint32 y,
				     qint32 width, qint32 height)
{
    // Actial bytes reading/writing is done in private header 
    writeBytesBody(data, x, y, width, height);
}

void KisTiledDataManager::readBytes(quint8 *data,
                                    qint32 x, qint32 y,
                                    qint32 width, qint32 height)
{
    // Actial bytes reading/writing is done in private header 
    readBytesBody(data, x, y, width, height);
}

QVector<quint8*> 
KisTiledDataManager::readPlanarBytes(QVector<qint32> channelSizes,
				     qint32 x, qint32 y,
				     qint32 width, qint32 height)
{
    // Actial bytes reading/writing is done in private header 
    return readPlanarBytesBody(channelSizes, x, y, width, height);
}


void KisTiledDataManager::writePlanarBytes(QVector<quint8*> planes,
					   QVector<qint32> channelSizes,
					   qint32 x, qint32 y,
					   qint32 width, qint32 height)
{
    // Actial bytes reading/writing is done in private header 
    writePlanarBytesBody(planes, channelSizes, x, y, width, height);
}

qint32 KisTiledDataManager::numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const
{
    qint32 numColumns;

    Q_UNUSED(minY);
    Q_UNUSED(maxY);

    if (x >= 0) {
        numColumns = KisTileData::WIDTH - (x % KisTileData::WIDTH);
    } else {
        numColumns = ((-x - 1) % KisTileData::WIDTH) + 1;
    }

    return numColumns;
}

qint32 KisTiledDataManager::numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const
{
    qint32 numRows;

    Q_UNUSED(minX);
    Q_UNUSED(maxX);

    if (y >= 0) {
        numRows = KisTileData::HEIGHT - (y % KisTileData::HEIGHT);
    } else {
        numRows = ((-y - 1) % KisTileData::HEIGHT) + 1;
    }

    return numRows;
}

qint32 KisTiledDataManager::rowStride(qint32 x, qint32 y) const
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    return KisTileData::WIDTH * pixelSize();
}


