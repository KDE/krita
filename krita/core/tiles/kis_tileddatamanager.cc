/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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

#include <q3valuevector.h>

#include <kdebug.h>

#include <KoStore.h>

#include "kis_global.h"
#include "kis_debug_areas.h"
#include "kis_tileddatamanager.h"
#include "kis_tilediterator.h"
#include "kis_tile.h"
#include "kis_memento.h"
#include "kis_tilemanager.h"

/* The data area is divided into tiles each say 64x64 pixels (defined at compiletime)
 * The tiles are laid out in a matrix that can have negative indexes.
 * The matrix grows automatically if needed (a call for writeacces to a tile outside the current extent)
 *  Even though the matrix has grown it may still not contain tiles at specific positions. They are created on demand
 */

KisTiledDataManager::KisTiledDataManager(quint32 pixelSize, const quint8 *defPixel)
{
    m_pixelSize = pixelSize;

    m_defPixel = new quint8[m_pixelSize];
    Q_CHECK_PTR(m_defPixel);
    memcpy(m_defPixel, defPixel, m_pixelSize);

    m_defaultTile = new KisTile(pixelSize,0,0, m_defPixel);
    Q_CHECK_PTR(m_defaultTile);

    m_hashTable = new KisTile * [1024];
    Q_CHECK_PTR(m_hashTable);

    for(int i = 0; i < 1024; i++)
        m_hashTable [i] = 0;
    m_numTiles = 0;
    m_currentMemento = 0;
    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager & dm)
    : KShared()
{
    m_pixelSize = dm.m_pixelSize;

    m_defPixel = new quint8[m_pixelSize];
    Q_CHECK_PTR(m_defPixel);
    memcpy(m_defPixel, dm.m_defPixel, m_pixelSize);

    m_defaultTile = new KisTile(*dm.m_defaultTile, dm.m_defaultTile->getCol(), dm.m_defaultTile->getRow());
    Q_CHECK_PTR(m_defaultTile);

    m_hashTable = new KisTile * [1024];
    Q_CHECK_PTR(m_hashTable);

    m_numTiles = 0;
    m_currentMemento = 0;
    m_extentMinX = dm.m_extentMinX;
    m_extentMinY = dm.m_extentMinY;
    m_extentMaxX = dm.m_extentMaxX;
    m_extentMaxY = dm.m_extentMaxY;

    // Deep copy every tile. XXX: Make this copy-on-write!
    for(int i = 0; i < 1024; i++)
    {
        const KisTile *tile = dm.m_hashTable[i];

        m_hashTable[i] = 0;

        while(tile)
        {
            KisTile *newtile = new KisTile(*tile, tile->getCol(), tile->getRow());
            Q_CHECK_PTR(newtile);

            newtile->setNext(m_hashTable[i]);
            m_hashTable[i] = newtile;
            tile = tile->getNext();

            m_numTiles++;
        }
    }

}

KisTiledDataManager::~KisTiledDataManager()
{
    // Deep delete every tile
    for(int i = 0; i < 1024; i++)
    {
        const KisTile *tile = m_hashTable[i];

        while(tile)
        {
            const KisTile *deltile = tile;
            tile = tile->getNext();
            delete deltile;
        }
    }
    delete [] m_hashTable;
    delete m_defaultTile;
    delete [] m_defPixel;
}

void KisTiledDataManager::setDefaultPixel(const quint8 *defPixel)
{
    if (defPixel == 0) return;

    memcpy(m_defPixel, defPixel, m_pixelSize);

    m_defaultTile->setData(m_defPixel);
}

bool KisTiledDataManager::write(KoStore *store)
{

    if (store == 0) return false;
    //Q_ASSERT(store != 0);

    char str[80];

    sprintf(str, "%d\n", m_numTiles);
    store->write(str,strlen(str));

    for(int i = 0; i < 1024; i++)
    {
        const KisTile *tile = m_hashTable[i];

        while(tile)
        {
            sprintf(str, "%d,%d,%d,%d\n", tile->getCol() * KisTile::WIDTH,
                            tile->getRow() * KisTile::HEIGHT,
                            KisTile::WIDTH, KisTile::HEIGHT);
            store->write(str,strlen(str));

            store->write((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_pixelSize);

            tile = tile->getNext();
        }
    }

    return true;
}
bool KisTiledDataManager::read(KoStore *store)
{
    if (store == 0) return false;
    //Q_ASSERT(store != 0);

    char str[80];
    qint32 x,y,w,h;

    QIODevice *stream = store->device();
    if (stream == 0) return false;
    //Q_ASSERT(stream != 0);

    stream->readLine(str, 79);

    sscanf(str,"%u",&m_numTiles);

    for(quint32 i = 0; i < m_numTiles; i++)
    {
        stream->readLine(str, 79);
        sscanf(str,"%d,%d,%d,%d",&x,&y,&w,&h);

        // the following is only correct as long as tile size is not changed
        // The first time we change tilesize the dimensions just read needs to be respected
        // but for now we just assume that tiles are the same size as ever.
        qint32 row = yToRow(y);
        qint32 col = xToCol(x);
        quint32 tileHash = calcTileHash(col, row);

        KisTile *tile = new KisTile(m_pixelSize, col, row, m_defPixel);
        Q_CHECK_PTR(tile);

        updateExtent(col,row);

        store->read((char *)tile->m_data, KisTile::HEIGHT * KisTile::WIDTH * m_pixelSize);

        tile->setNext(m_hashTable[tileHash]);
        m_hashTable[tileHash] = tile;
    }
    return true;
}

void KisTiledDataManager::extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    x = m_extentMinX;
    y = m_extentMinY;

    if (m_extentMaxX >= m_extentMinX) {
        w = m_extentMaxX - m_extentMinX + 1;
    } else {
        w = 0;
    }

    if (m_extentMaxY >= m_extentMinY) {
        h = m_extentMaxY - m_extentMinY + 1;
    } else {
        h = 0;
    }
}

QRect KisTiledDataManager::extent() const
{
    qint32 x;
    qint32 y;
    qint32 w;
    qint32 h;

    extent(x, y, w, h);

    return QRect(x, y, w, h);
}

void KisTiledDataManager::setExtent(qint32 x, qint32 y, qint32 w, qint32 h)
{
    QRect newRect = QRect(x, y, w, h).normalize();
    //printRect("newRect", newRect);
    QRect oldRect = QRect(m_extentMinX, m_extentMinY, m_extentMaxX - m_extentMinX + 1, m_extentMaxY - m_extentMinY + 1).normalize();
    //printRect("oldRect", oldRect);

    // Do nothing if the desired size is bigger than we currently are: that is handled by the autoextending automatically
    if (newRect.contains(oldRect)) return;

    // Loop through all tiles, if a tile is wholly outside the extent, add to the memento, then delete it,
    // if the tile is partially outside the extent, clear the outside pixels to the default pixel.
    for(int tileHash = 0; tileHash < 1024; tileHash++)
    {
        KisTile *tile = m_hashTable[tileHash];
        KisTile *previousTile = 0;

        while(tile)
        {
            QRect tileRect = QRect(tile->getCol() * KisTile::WIDTH, tile->getRow() * KisTile::HEIGHT, KisTile::WIDTH, KisTile::HEIGHT);
            //printRect("tileRect", tileRect);

            if (newRect.contains(tileRect)) {
                // Completely inside, do nothing
                previousTile = tile;
                tile = tile->getNext();
            }
            else {
                ensureTileMementoed(tile->getCol(), tile->getRow(), tileHash, tile);

                if (newRect.intersects(tileRect)) {

                    // Create the intersection of the tile and new rect
                    QRect intersection = newRect.intersect(tileRect);
                    //printRect("intersection", intersection);
                    intersection.setRect(intersection.x() - tileRect.x(), intersection.y() - tileRect.y(), intersection.width(), intersection.height());

                    // This can be done a lot more efficiently, no doubt, by clearing runs of pixels to the left and the right of
                    // the intersecting line.
                    for (int y = 0; y < KisTile::HEIGHT; ++y) {
                        for (int x = 0; x < KisTile::WIDTH; ++x) {
                            if (!intersection.contains(x,y)) {
                                quint8 * ptr = tile->data(x, y);
                                memcpy(ptr, m_defPixel, m_pixelSize);
                            }
                        }
                    }
                    previousTile = tile;
                    tile = tile->getNext();
                }
                else {
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
    for (int tileHash = 0; tileHash < 1024; tileHash++)
    {
        const KisTile *tile = m_hashTable[tileHash];

        while (tile)
        {
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

            if (clearTileRect == tileRect) {

                // Clear whole tile
                memset(tile->data(), clearValue, KisTile::WIDTH * KisTile::HEIGHT * m_pixelSize);

            } else {

                quint32 rowsRemaining = clearTileRect.height();
                quint8 *dst = tile->data(clearTileRect.x() - tileRect.x(), clearTileRect.y() - tileRect.y());

                while (rowsRemaining > 0) {
                    memset(dst, clearValue, clearTileRect.width() * m_pixelSize);
                    dst += rowStride;
                    --rowsRemaining;
                }
            }
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
                    memcpy(tile->data(), clearPixelData, KisTile::WIDTH * KisTile::HEIGHT * m_pixelSize);

                } else {

                    quint32 rowsRemaining = clearTileRect.height();
                    quint8 *dst = tile->data(clearTileRect.x() - tileRect.x(), clearTileRect.y() - tileRect.y());

                    while (rowsRemaining > 0) {
                        memcpy(dst, clearPixelData, clearTileRect.width() * m_pixelSize);
                        dst += rowStride;
                        --rowsRemaining;
                    }
                }
            }
        }

        delete [] clearPixelData;
    }
}

void KisTiledDataManager::clear()
{
    // Loop through all tiles, add to the memento, then delete it,
    for(int tileHash = 0; tileHash < 1024; tileHash++)
    {
        const KisTile *tile = m_hashTable[tileHash];

        while(tile)
        {
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

void KisTiledDataManager::paste(KisDataManagerSP data,  qint32 sx, qint32 sy, qint32 dx, qint32 dy,
             qint32 w, qint32 h)
{
    //CBR_MISSING
    sx=sy=dx=dy=w=h;data=0;
}


quint32 KisTiledDataManager::calcTileHash(qint32 col, qint32 row)
{
    return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

KisMementoSP KisTiledDataManager::getMemento()
{
    m_currentMemento = new KisMemento(m_pixelSize);
    Q_CHECK_PTR(m_currentMemento);

    memcpy(m_currentMemento->m_defPixel, m_defPixel, m_pixelSize);

    return m_currentMemento;
}

void KisTiledDataManager::rollback(KisMementoSP memento)
{
    if (memento == 0) return;
    //Q_ASSERT(memento != 0);

    if (m_currentMemento != 0) {
        // Undo means our current memento is no longer valid so remove it.
        m_currentMemento = 0;
    }

    // Rollback means restoring all of the tiles in the memento to our hashtable.

    // But first clear the memento redo hashtable.
    // This is nessesary as new changes might have been done since last rollback (automatic filters)
    for(int i = 0; i < 1024; i++)
    {
        memento->deleteAll(memento->m_redoHashTable[i]);
        memento->m_redoHashTable[i]=0;
    }

    // Also clear the table of deleted tiles
    memento->clearTilesToDeleteOnRedo();

    // Now on to the real rollback

    memcpy(memento->m_redoDefPixel, m_defPixel, m_pixelSize);
    setDefaultPixel(memento->m_defPixel);

    for(int i = 0; i < 1024; i++)
    {
        KisTile *tile = memento->m_hashTable[i];

        while(tile)
        {
            // The memento has a tile stored that we need to roll back
            // Now find the corresponding one in our hashtable
            KisTile *curTile = m_hashTable[i];
            KisTile *preTile = 0;
            while(curTile)
            {
                if(curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol())
                {
                    break;
                }
                preTile = curTile;
                curTile = curTile->getNext();
            }

            if(curTile)
            {
                // Remove it from our hashtable
                if(preTile)
                    preTile->setNext(curTile->getNext());
                else
                    m_hashTable[i]= curTile->getNext();

                m_numTiles--;

                // And put it in the redo hashtable of the memento
                curTile->setNext(memento->m_redoHashTable[i]);
                memento->m_redoHashTable[i] = curTile;
            }
            else
            {
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
    if (memento == 0) return;
    //Q_ASSERT(memento != 0);

    if (m_currentMemento != 0) {
        // Redo means our current memento is no longer valid so remove it.
        m_currentMemento = 0;
    }

    // Rollforward means restoring all of the tiles in the memento's redo to our hashtable.

    setDefaultPixel(memento->m_redoDefPixel);

    for(int i = 0; i < 1024; i++)
    {
        KisTile *tile = memento->m_redoHashTable[i];

        while(tile)
        {
            // The memento has a tile stored that we need to roll forward
            // Now find the corresponding one in our hashtable
            KisTile *curTile = m_hashTable[i];
            KisTile *preTile = 0;
            while(curTile)
            {
                if(curTile->getRow() == tile->getRow() && curTile->getCol() == tile->getCol())
                {
                    break;
                }
                preTile = curTile;
                curTile = curTile->getNext();
            }

            if (curTile)
            {
                // Remove it from our hashtable
                if(preTile)
                    preTile->setNext(curTile->getNext());
                else
                    m_hashTable[i]= curTile->getNext();

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

void KisTiledDataManager::deleteTiles(const KisMemento::DeletedTile *d)
{
    while (d)
    {
        quint32 tileHash = calcTileHash(d->col(), d->row());
        KisTile *curTile = m_hashTable[tileHash];
        KisTile *preTile = 0;
        while(curTile)
        {
            if(curTile->getRow() == d->row() && curTile->getCol() == d->col())
            {
                break;
            }
            preTile = curTile;
            curTile = curTile->getNext();
        }
        if (curTile) {
            // Remove it from our hashtable
            if(preTile)
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

void KisTiledDataManager::ensureTileMementoed(qint32 col, qint32 row, quint32 tileHash, const KisTile *refTile)
{
    if (refTile == 0) return;
    //Q_ASSERT(refTile != 0);

    // Basically we search for the tile in the current memento, and if it's already there we do nothing, otherwise
    //  we make a copy of the tile and put it in the current memento

    if(!m_currentMemento)
        return;

    KisTile *tile = m_currentMemento->m_hashTable[tileHash];
    while(tile != 0)
    {
        if(tile->getRow() == row && tile->getCol() == col)
            break;

        tile = tile->getNext();
    }
    if(tile)
        return; // it has allready been stored

    tile = new KisTile(*refTile);
    Q_CHECK_PTR(tile);

    tile->setNext(m_currentMemento->m_hashTable[tileHash]);
    m_currentMemento->m_hashTable[tileHash] = tile;
    m_currentMemento->m_numTiles++;
}

void KisTiledDataManager::updateExtent(qint32 col, qint32 row)
{
    if(m_extentMinX > col * KisTile::WIDTH)
        m_extentMinX = col * KisTile::WIDTH;
    if(m_extentMaxX < (col+1) * KisTile::WIDTH - 1)
        m_extentMaxX = (col+1) * KisTile::WIDTH - 1;
    if(m_extentMinY > row * KisTile::HEIGHT)
        m_extentMinY = row * KisTile::HEIGHT;
    if(m_extentMaxY < (row+1) * KisTile::HEIGHT - 1)
        m_extentMaxY = (row+1) * KisTile::HEIGHT - 1;
}

KisTile *KisTiledDataManager::getTile(qint32 col, qint32 row, bool writeAccess)
{
    quint32 tileHash = calcTileHash(col, row);

    // Lookup tile in hash table
    KisTile *tile = m_hashTable[tileHash];
    while(tile != 0)
    {
        if(tile->getRow() == row && tile->getCol() == col)
            break;

        tile = tile->getNext();
    }

    // Might not have been created yet
    if(!tile)
    {
        if(writeAccess)
        {
            // Create a new tile
            tile = new KisTile(*m_defaultTile, col, row);
            Q_CHECK_PTR(tile);

            tile->setNext(m_hashTable[tileHash]);
            m_hashTable[tileHash] = tile;
            m_numTiles++;
            updateExtent(col, row);

            if (m_currentMemento && !m_currentMemento->containsTile(col, row, tileHash)) {
                m_currentMemento->addTileToDeleteOnUndo(col, row);
            }
        }
        else
            // If only read access then it's enough to share a default tile
            tile = m_defaultTile;
    }

    if(writeAccess)
        ensureTileMementoed(col, row, tileHash, tile);

    return tile;
}

KisTile *KisTiledDataManager::getOldTile(qint32 col, qint32 row, KisTile *def)
{
    KisTile *tile = 0;

    // Lookup tile in hash table of current memento
    if (m_currentMemento)
    {
        if (!m_currentMemento->valid()) return def;
        //Q_ASSERT(m_currentMemento->valid());

        quint32 tileHash = calcTileHash(col, row);
        tile = m_currentMemento->m_hashTable[tileHash];
        while (tile != 0)
        {
            if (tile->getRow() == row && tile->getCol() == col)
                break;

            tile = tile->getNext();
        }
    }

    if (!tile)
        tile = def;

    return tile;
}

quint8* KisTiledDataManager::pixelPtr(qint32 x, qint32 y, bool writable)
{
    qint32 row = yToRow(y);
    qint32 col = xToCol(x);

    // calc limits within the tile
    qint32 yInTile = y - row * KisTile::HEIGHT;
    qint32 xInTile = x - col * KisTile::WIDTH;
    qint32 offset = m_pixelSize * (yInTile * KisTile::WIDTH + xInTile);

    KisTile *tile = getTile(col, row, writable);

    return tile->data() + offset;
}

const quint8* KisTiledDataManager::pixel(qint32 x, qint32 y)
{
    return pixelPtr(x, y, false);
}

quint8* KisTiledDataManager::writablePixel(qint32 x, qint32 y)
{
    return pixelPtr(x, y, true);
}

void KisTiledDataManager::setPixel(qint32 x, qint32 y, const quint8 * data)
{
    quint8 *pixel = pixelPtr(x, y, true);
    memcpy(pixel, data, m_pixelSize);
}


void KisTiledDataManager::readBytes(quint8 * data,
                    qint32 x, qint32 y,
                    qint32 w, qint32 h)
{
    if (data == 0) return;
    //Q_ASSERT(data != 0);
     if (w < 0)
         w = 0;

     if (h < 0)
        h = 0;

    qint32 dstY = 0;
    qint32 srcY = y;
    qint32 rowsRemaining = h;

    while (rowsRemaining > 0) {

        qint32 dstX = 0;
        qint32 srcX = x;
        qint32 columnsRemaining = w;
        qint32 numContiguousSrcRows = numContiguousRows(srcY, srcX, srcX + w - 1);

        qint32 rows = qMin(numContiguousSrcRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousSrcColumns = numContiguousColumns(srcX, srcY, srcY + rows - 1);

            qint32 columns = qMin(numContiguousSrcColumns, columnsRemaining);

            const quint8 *srcData = pixel(srcX, srcY);
            qint32 srcRowStride = rowStride(srcX, srcY);

            quint8 *dstData = data + ((dstX + (dstY * w)) * m_pixelSize);
            qint32 dstRowStride = w * m_pixelSize;

            for (qint32 row = 0; row < rows; row++) {
                memcpy(dstData, srcData, columns * m_pixelSize);
                dstData += dstRowStride;
                srcData += srcRowStride;
            }

            srcX += columns;
            dstX += columns;
            columnsRemaining -= columns;
        }

        srcY += rows;
        dstY += rows;
        rowsRemaining -= rows;
    }

}


void KisTiledDataManager::writeBytes(const quint8 * bytes,
                     qint32 x, qint32 y,
                     qint32 w, qint32 h)
{
    if (bytes == 0) return;
    //Q_ASSERT(bytes != 0);

    // XXX: Is this correct?
    if (w < 0)
        w = 0;

    if (h < 0)
        h = 0;

    qint32 srcY = 0;
    qint32 dstY = y;
    qint32 rowsRemaining = h;

    while (rowsRemaining > 0) {

        qint32 srcX = 0;
        qint32 dstX = x;
        qint32 columnsRemaining = w;
        qint32 numContiguousdstRows = numContiguousRows(dstY, dstX, dstX + w - 1);

        qint32 rows = qMin(numContiguousdstRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousdstColumns = numContiguousColumns(dstX, dstY, dstY + rows - 1);

            qint32 columns = qMin(numContiguousdstColumns, columnsRemaining);

            quint8 *dstData = writablePixel(dstX, dstY);
            qint32 dstRowStride = rowStride(dstX, dstY);

            const quint8 *srcData = bytes + ((srcX + (srcY * w)) * m_pixelSize);
            qint32 srcRowStride = w * m_pixelSize;

            for (qint32 row = 0; row < rows; row++) {
                memcpy(dstData, srcData, columns * m_pixelSize);
                srcData += srcRowStride;
                dstData += dstRowStride;
            }

            dstX += columns;
            srcX += columns;
            columnsRemaining -= columns;
        }

        dstY += rows;
        srcY += rows;
        rowsRemaining -= rows;
    }
}

qint32 KisTiledDataManager::numContiguousColumns(qint32 x, qint32 minY, qint32 maxY)
{
    qint32 numColumns;

    Q_UNUSED(minY);
    Q_UNUSED(maxY);

    if (x >= 0) {
        numColumns = KisTile::WIDTH - (x % KisTile::WIDTH);
    } else {
        numColumns = ((-x - 1) % KisTile::WIDTH) + 1;
    }

    return numColumns;
}

qint32 KisTiledDataManager::numContiguousRows(qint32 y, qint32 minX, qint32 maxX)
{
    qint32 numRows;

    Q_UNUSED(minX);
    Q_UNUSED(maxX);

    if (y >= 0) {
        numRows = KisTile::HEIGHT - (y % KisTile::HEIGHT);
    } else {
        numRows = ((-y - 1) % KisTile::HEIGHT) + 1;
    }

    return numRows;
}

qint32 KisTiledDataManager::rowStride(qint32 x, qint32 y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    return KisTile::WIDTH * m_pixelSize;
}

qint32 KisTiledDataManager::numTiles(void) const
{
    return m_numTiles;
}

