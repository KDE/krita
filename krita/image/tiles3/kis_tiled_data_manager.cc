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

#include <KoStore.h>

#include "kis_global.h"
//#include "kis_debug.h"


/* The data area is divided into tiles each say 64x64 pixels (defined at compiletime)
 * The tiles are laid out in a matrix that can have negative indexes.
 * The matrix grows automatically if needed (a call for writeacces to a tile
 * outside the current extent)
 * Even though the matrix has grown it may still not contain tiles at specific positions.
 * They are created on demand
 */

KisTiledDataManager::KisTiledDataManager(quint32 pixelSize,
        const quint8 *defaultPixel)
        : m_lock(QReadWriteLock::NonRecursive)
{
    /* See comment in destructor for details */
    m_mementoManager = new KisMementoManager();
    m_hashTable = new KisTileHashTable(m_mementoManager);

    m_pixelSize = pixelSize;
    m_defaultPixel = new quint8[m_pixelSize];
    setDefaultPixel(defaultPixel);

    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}

KisTiledDataManager::KisTiledDataManager(const KisTiledDataManager &dm)
        : KisShared(dm),
        m_lock(QReadWriteLock::NonRecursive)
{
    /* See comment in destructor for details */
    m_mementoManager = new KisMementoManager(*dm.m_mementoManager);
    m_hashTable = new KisTileHashTable(*dm.m_hashTable, m_mementoManager);

    m_pixelSize = dm.m_pixelSize;
    m_defaultPixel = new quint8[m_pixelSize];
    /**
     * We won't call setDefaultTileData here, as defaultTileDatas
     * has already been made shared in m_hashTable(dm->m_hashTable)
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
    QWriteLocker locker(&m_lock);

    KisTileData *td = globalTileDataStore.createDefaultTileData(pixelSize(),
                      defaultPixel);
    m_hashTable->setDefaultTileData(td);
    m_mementoManager->setDefaultTileData(td);

    memcpy(m_defaultPixel, defaultPixel, pixelSize());
}


bool KisTiledDataManager::write(KoStore *store)
{
    QReadLocker locker(&m_lock);
    if (!store) return false;

    char str[80];

    sprintf(str, "%d\n", m_hashTable->numTiles());
    store->write(str, strlen(str));


    KisTileHashTableIterator iter(m_hashTable);
    KisTileSP tile;
    qint32 x, y;
    qint32 width, height;

    const qint32 tileDataSize = KisTileData::HEIGHT * KisTileData::WIDTH * pixelSize();

    while (tile = iter.tile()) {
        tile->extent().getRect(&x, &y, &width, &height);
        sprintf(str, "%d,%d,%d,%d\n", x, y, width, height);
        store->write(str, strlen(str));

        tile->lockForRead();
        store->write((char *)tile->data(), tileDataSize);
        tile->unlock();

        ++iter;
    }

    printf("*** KisTiledDataManager::write called \n");
    return true;
}
bool KisTiledDataManager::read(KoStore *store)
{
    QWriteLocker locker(&m_lock);
    if (!store) return false;

    //clear(); - needed?

    /*nothing*/ m_mementoManager->getMemento();

    KisTileSP tile;
    const qint32 tileDataSize = KisTileData::HEIGHT * KisTileData::WIDTH * pixelSize();
    qint32 x, y;
    qint32 width, height;
    char str[80];

    QIODevice *stream = store->device();
    if (!stream) return false;

    quint32 numTiles;
    stream->readLine(str, 79);
    sscanf(str, "%u", &numTiles);

    for (quint32 i = 0; i < numTiles; i++) {
        stream->readLine(str, 79);
        sscanf(str, "%d,%d,%d,%d", &x, &y, &width, &height);

        // the following is only correct as long as tile size is not changed
        // The first time we change tilesize the dimensions just read needs to be respected
        // but for now we just assume that tiles are the same size as ever.
        qint32 row = yToRow(y);
        qint32 col = xToCol(x);
        bool created;

        tile = m_hashTable->getTileLazy(col, row, created);
        if (created)
            updateExtent(col, row);

        tile->lockForWrite();
        store->read((char *)tile->data(), tileDataSize);
        tile->unlock();
    }

    return true;
}

quint8* KisTiledDataManager::duplicatePixel(qint32 num, const quint8 *pixel)
{
    const qint32 pixelSize = this->pixelSize();
    /* FIXME:  Make a fun filling here */
    quint8 *dstBuf = new quint8[num * pixelSize];
    quint8 *dstIt = dstBuf;
    for (qint32 i = 0; i < num; i++) {
        memcpy(dstIt, pixel, pixelSize);
        dstIt += pixelSize;
    }
    return dstBuf;
}

void KisTiledDataManager::clear(QRect clearRect, const quint8 *clearPixel)
{
    QWriteLocker locker(&m_lock);

    if (clearPixel == 0)
        clearPixel = m_defaultPixel;

    if (clearRect.isEmpty())
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
        if (pixelBytesAreDefault)
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

            if (clearTileRect == tileRect) {
                // Clear whole tile
                m_hashTable->deleteTile(column, row);

                KisTileSP clearedTile = new KisTile(column, row, td, m_mementoManager);
                m_hashTable->addTile(clearedTile);
                updateExtent(column, row);

                /**
                 * Emulate like we've written something to the tile
                 * to keep the history clean
                 */
                KisTileDataWrapper tw(clearedTile, 0,
                                      KisTileDataWrapper::WRITE);
            } else {
                const qint32 lineSize = clearTileRect.width() * pixelSize;
                qint32 rowsRemaining = clearTileRect.height();

                KisTileDataWrapper tw = pixelPtr(clearTileRect.left(),
                                                 clearTileRect.top(),
                                                 KisTileDataWrapper::WRITE);
                quint8* tileIt = tw.data();

                if (pixelBytesAreTheSame) {
                    while (rowsRemaining > 0) {
                        memset(tileIt, *clearPixelData, lineSize);
                        tileIt += rowStride;
                        rowsRemaining--;
                    }
                } else {
                    while (rowsRemaining > 0) {
                        memcpy(tileIt, clearPixelData, lineSize);
                        tileIt += rowStride;
                        rowsRemaining--;
                    }
                }
            }
        }
    }
    if (td) globalTileDataStore.releaseTileData(td);
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
    clear(QRect(x, y, w, h), clearPixel);
}
void KisTiledDataManager::clear(qint32 x, qint32 y, qint32 w, qint32 h, quint8 clearValue)
{
    clear(QRect(x, y, w, h), clearValue);
}

void KisTiledDataManager::clear()
{
    QWriteLocker locker(&m_lock);

    m_hashTable->clear();

    m_extentMinX = qint32_MAX;
    m_extentMinY = qint32_MAX;
    m_extentMaxX = qint32_MIN;
    m_extentMaxY = qint32_MIN;
}


void KisTiledDataManager::setExtent(qint32 x, qint32 y, qint32 w, qint32 h)
{
    setExtent(QRect(x, y, w, h));
}

void KisTiledDataManager::setExtent(QRect newRect)
{
    QRect oldRect = extent();
    newRect = newRect.normalized();

    // Do nothing if the desired size is bigger than we currently are:
    // that is handled by the autoextending automatically
    if (newRect.contains(oldRect)) return;

    QWriteLocker locker(&m_lock);

    KisTileSP tile;
    QRect tileRect;
    KisTileHashTableIterator iter(m_hashTable);

    while (!iter.isDone()) {
        tile = iter.tile();

        tileRect = tile->extent();
        if (newRect.contains(tileRect)) {
            //do nothing
            ++iter;
        } else if (newRect.intersects(tileRect)) {
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
        } else {
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

    while (!(tile = iter.tile())) {
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
    QReadLocker locker(&m_lock);

    x = m_extentMinX;
    y = m_extentMinY;
    w = (m_extentMaxX >= m_extentMinX) ? m_extentMaxX - m_extentMinX + 1 : 0;
    h = (m_extentMaxY >= m_extentMinY) ? m_extentMaxY - m_extentMinY + 1 : 0;
}

QRect KisTiledDataManager::extent() const
{
    qint32 x, y, w, h;
    extent(x, y, w, h);

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

    /*    bool newTile;
          KisTileSP tile = m_hashTable->getTileLazy(col, row, newTile);
          if(newTile)
          updateExtent(tile->col(), tile->row());
    */
    KisTileSP tile = getTile(col, row);

    return KisTileDataWrapper(tile,
                              pixelIndex*pixelSize(),
                              type);
}

void KisTiledDataManager::setPixel(qint32 x, qint32 y, const quint8 * data)
{
    QWriteLocker locker(&m_lock);
    KisTileDataWrapper tw = pixelPtr(x, y, KisTileDataWrapper::WRITE);
    memcpy(tw.data(), data, pixelSize());
}

void KisTiledDataManager::writeBytes(const quint8 *data,
                                     qint32 x, qint32 y,
                                     qint32 width, qint32 height)
{
    QWriteLocker locker(&m_lock);
    // Actial bytes reading/writing is done in private header
    writeBytesBody(data, x, y, width, height);
}

void KisTiledDataManager::readBytes(quint8 *data,
                                    qint32 x, qint32 y,
                                    qint32 width, qint32 height) const
{
    QReadLocker locker(&m_lock);
    // Actial bytes reading/writing is done in private header
    readBytesBody(data, x, y, width, height);
}

QVector<quint8*>
KisTiledDataManager::readPlanarBytes(QVector<qint32> channelSizes,
                                     qint32 x, qint32 y,
                                     qint32 width, qint32 height)
{
    QReadLocker locker(&m_lock);
    // Actial bytes reading/writing is done in private header
    return readPlanarBytesBody(channelSizes, x, y, width, height);
}


void KisTiledDataManager::writePlanarBytes(QVector<quint8*> planes,
        QVector<qint32> channelSizes,
        qint32 x, qint32 y,
        qint32 width, qint32 height)
{
    QWriteLocker locker(&m_lock);
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

