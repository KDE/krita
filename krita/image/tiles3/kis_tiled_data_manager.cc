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
#include "kis_memento_manager.h"

//#include <KoStore.h>

#include "kis_global.h"
//#include "kis_debug.h"


//#include "kis_memento.h"

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
    /* See comment in destructor for details */
    m_mementoManager = new KisMementoManager();
    m_hashTable = new KisTileHashTable();
    m_hashTable->setMementoManager(m_mementoManager);

    m_pixelSize = pixelSize;
    m_defaultPixel = new quint8[m_pixelSize];
    setDefaultPixel(defaultPixel);

    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager &dm) 
    : KisShared(dm)
{
    /* See comment in destructor for details */
    m_mementoManager = new KisMementoManager(*dm.m_mementoManager);
    m_hashTable = new KisTileHashTable(*dm.m_hashTable);
    m_hashTable->setMementoManager(m_mementoManager);

    m_pixelSize = dm.m_pixelSize;
    m_defaultPixel = new quint8[m_pixelSize];
    /**
     * We don't call setDefaultTileData here, as defaultTileDatas 
     * will be made shared in m_hashTable(dm->m_hashTable)
     */
    memcpy(m_defaultPixel, dm.m_defaultPixel, m_pixelSize);

    m_extentMinX = dm.m_extentMinX;
    m_extentMinY = dm.m_extentMinY;
    m_extentMaxX = dm.m_extentMaxX;
    m_extentMaxY = dm.m_extentMaxY;
}

KisTiledDataManager::~KisTiledDataManager()
{
    /**
     * Here is an  explanation why we use hash table  and The Memento Manager
     * dynamically allocated We need to  destroy them in that very order. The
     * reason is that when hash table destroying all her child tiles they all
     * cry about it  to The Memento Manager using a  pointer.  So The Memento
     * Manager sould  be alive during  that destruction. We could  use shared
     * pointers instead, but they create too much overhead.
     */
    delete m_hashTable;
    delete m_mementoManager;

    delete[] m_defaultPixel;
}

void KisTiledDataManager::setDefaultPixel(const quint8 *defaultPixel)
{
    KisTileData *td = globalTileDataStore.createDefaultTileData(pixelSize(),
                                                                defaultPixel);
    m_hashTable->setDefaultTileData(td);    
    m_mementoManager->setDefaultTileData(td);    

    memcpy(m_defaultPixel, defaultPixel, pixelSize());
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

quint8* KisTiledDataManager::duplicatePixel(qint32 num, const quint8 *pixel)
{
    const qint32 pixelSize = this->pixelSize();
    /*FIXME:  Make a fun filling here */
    quint8 *dstBuf = new quint8[num * pixelSize];
    quint8 *dstIt = dstBuf;
    for(qint32 i=0; i<num; i++) {
	memcpy(dstIt, pixel, pixelSize);
	dstIt+=pixelSize;
    }
    return dstBuf;
}

void KisTiledDataManager::clear(QRect clearRect, const quint8 *clearPixel)
{
    if(clearPixel == 0)
	clearPixel = m_defaultPixel;

    if(clearRect.isEmpty())
        return;

    const qint32 pixelSize = this->pixelSize();

    bool pixelBytesAreDefault = !memcmp(clearPixel, m_defaultPixel, pixelSize);

    bool pixelBytesAreTheSame = true;
    for (qint32 i = 0; i < pixelSize; ++i) {
        if (clearPixel[i] != clearPixel[0]) {
            pixelBytesAreTheSame = false;
            break;
        }
    }

    qint32 firstColumn = xToCol(clearRect.left());
    qint32 lastColumn = xToCol(clearRect.right());
    
    qint32 firstRow = yToRow(clearRect.top());
    qint32 lastRow = yToRow(clearRect.bottom());
    
    const quint32 rowStride = KisTileData::WIDTH * pixelSize;
    
    // Generate one row
    quint8 *clearPixelData = 0;
    quint32 maxRunLength = qMin(clearRect.width(), KisTileData::WIDTH);
    clearPixelData = duplicatePixel(maxRunLength, clearPixel);
    
    KisTileData *td = 0;
    if (clearRect.width() >= KisTileData::WIDTH && 
	clearRect.height() >= KisTileData::HEIGHT) {
	if(pixelBytesAreDefault)
	    /**
	     * FIXME: Theoretical race condition 
	     *        if setDefaultPixel has been called first
	     */
	    td = m_hashTable->defaultTileData();
	else
	    td = globalTileDataStore.createDefaultTileData(pixelSize,
							   clearPixel);
	globalTileDataStore.acquireTileData(td);
    } 
    
    for (qint32 row = firstRow; row <= lastRow; ++row) {
	for (qint32 column = firstColumn; column <= lastColumn; ++column) {
	    
	    QRect tileRect(column*KisTileData::WIDTH, row*KisTileData::HEIGHT,
			   KisTileData::WIDTH, KisTileData::WIDTH);
	    QRect clearTileRect = clearRect & tileRect;
	    
	    KisTileDataWrapper tw = pixelPtr(clearTileRect.left(),
					     clearTileRect.top(),
					     KisTileDataWrapper::WRITE);
	    quint8* tileIt = tw.data();
	    
	    if (clearTileRect == tileRect) {
		// Clear whole tile
		m_hashTable->deleteTile(column, row);
		m_hashTable->addTile(new KisTile(column,row,td, m_mementoManager));
	    } 
	    else {
		const qint32 lineSize = clearTileRect.width() * pixelSize;
		qint32 rowsRemaining = clearTileRect.height();
		
		if(pixelBytesAreTheSame) {
		    while (rowsRemaining > 0) {
			memset(tileIt, *clearPixelData, lineSize);
			tileIt += rowStride;
			rowsRemaining--;
		    }
		}
		else {
		    while (rowsRemaining > 0) {
			memcpy(tileIt, clearPixelData, lineSize);
			tileIt += rowStride;
			rowsRemaining--;
		    }
		}
	    }
	}
    }
    if(td) globalTileDataStore.releaseTileData(td);
    delete[] clearPixelData;
}

void KisTiledDataManager::clear(QRect clearRect, quint8 clearValue)
{
    quint8 *buf = new quint8[pixelSize()];
    memset(buf, clearValue, pixelSize());
    clear(clearRect, buf);
    delete[] buf;
}

void KisTiledDataManager::clear(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *clearPixel)
{
    clear(QRect(x,y,w,h), clearPixel);
}
void KisTiledDataManager::clear(qint32 x, qint32 y, qint32 w, qint32 h, quint8 clearValue)
{
    clear(QRect(x,y,w,h), clearValue);
}

void KisTiledDataManager::clear()
{
    m_hashTable->clear();

    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}


void KisTiledDataManager::setExtent(qint32 x, qint32 y, qint32 w, qint32 h)
{
    setExtent(QRect(x,y,w,h));
}

void KisTiledDataManager::setExtent(QRect newRect)
{
    QRect oldRect = extent();
    newRect = newRect.normalized();

    // Do nothing if the desired size is bigger than we currently are: 
    // that is handled by the autoextending automatically
    if (newRect.contains(oldRect)) return;

    KisTileSP tile;
    QRect tileRect;
    KisTileHashTableIterator iter(m_hashTable);

    while(!iter.isDone()) {
	tile=iter.tile();
	
	tileRect = tile->extent();
	if(newRect.contains(tileRect)) {
	    //do nothing
	    ++iter;
	} 
	else if(newRect.intersects(tileRect)) {
	    QRect intersection = newRect & tileRect;
	    intersection.translate(- tileRect.topLeft());
	    
	    const qint32 pixelSize = this->pixelSize();

	    tile->lockForWrite();
	    quint8* data = tile->data();
	    quint8* ptr;

	    /* FIXME: make it faster */
	    for (int y = 0; y < KisTileData::HEIGHT; y++) {
		for (int x = 0; x < KisTileData::WIDTH; x++) {
		    if (!intersection.contains(x, y)) {
			ptr = data + pixelSize * (y * KisTileData::WIDTH + x);
			memcpy(ptr, m_defaultPixel, pixelSize);
		    }
		}
	    }
	    tile->unlock();
	    ++iter;
	}
	else {
	    iter.deleteCurrent();
	}
    }

    recalculateExtent();
}

void KisTiledDataManager::recalculateExtent()
{
    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
    
    KisTileHashTableIterator iter(m_hashTable);
    KisTileSP tile;
    
    while(! (tile = iter.tile()) ) {
	updateExtent(tile->col(), tile->row());
	++iter;
    }
}

void KisTiledDataManager::updateExtent(qint32 col, qint32 row)
{
    const qint32 tileMinX = col * KisTileData::WIDTH;
    const qint32 tileMinY = row * KisTileData::HEIGHT;
    const qint32 tileMaxX = tileMinX + KisTileData::WIDTH - 1;
    const qint32 tileMaxY = tileMinY + KisTileData::HEIGHT - 1;

    m_extentMinX = qMin(m_extentMinX, tileMinX);
    m_extentMaxX = qMax(m_extentMaxX, tileMaxX);
    m_extentMinY = qMin(m_extentMinY, tileMinY);
    m_extentMaxY = qMax(m_extentMaxY, tileMaxY);
}

void KisTiledDataManager::extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    x = m_extentMinX;
    y = m_extentMinY;
    w = m_extentMaxX - m_extentMinX + 1;
    h = m_extentMaxY - m_extentMinY + 1;
}

QRect KisTiledDataManager::extent() const
{
    qint32 x,y,w,h;
    extent(x,y,w,h);

    return QRect(x, y, w, h);
}



KisTileDataWrapper KisTiledDataManager::pixelPtr(qint32 x, qint32 y, 
			    enum KisTileDataWrapper::accessType type)
{
    const qint32 col = xToCol(x);
    const qint32 row = yToRow(y);
    
    /* FIXME: Always positive? */
    const qint32 xInTile = x - col * KisTileData::WIDTH;
    const qint32 yInTile = y - row * KisTileData::HEIGHT;

    const qint32 pixelIndex = xInTile + yInTile * KisTileData::WIDTH;

    bool newTile;
    KisTileSP tile = m_hashTable->getTileLazy(col, row, newTile);
    if(newTile)
	updateExtent(tile->col(), tile->row());

    return KisTileDataWrapper(tile,
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




/*****************************************************************************/
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





