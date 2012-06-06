/*
 *  Copyright (c) 2004 C. Boemann <cbo@boemann.dk>
 *            (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
 *            (c) 2010 Cyrille Berger <cberger@cberger.net>
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
#include "swap/kis_legacy_tile_compressor.h"
#include "swap/kis_tile_compressor_factory.h"

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
        : KisShared(),
        m_lock(QReadWriteLock::NonRecursive)
{
    /* See comment in destructor for details */

    /* We do not clone the history of the device, there is no usecase for it */
    m_mementoManager = new KisMementoManager();
    m_mementoManager->setDefaultTileData(dm.m_hashTable->defaultTileData());
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
    setDefaultPixelImpl(defaultPixel);
}

void KisTiledDataManager::setDefaultPixelImpl(const quint8 *defaultPixel)
{
    KisTileData *td = KisTileDataStore::instance()->createDefaultTileData(pixelSize(), defaultPixel);
    m_hashTable->setDefaultTileData(td);
    m_mementoManager->setDefaultTileData(td);

    memcpy(m_defaultPixel, defaultPixel, pixelSize());
}

bool KisTiledDataManager::write(KoStore *store)
{
    QReadLocker locker(&m_lock);
    if (!store) return false;

    if(CURRENT_VERSION == LEGACY_VERSION) {
        char str[80];
        sprintf(str, "%d\n", m_hashTable->numTiles());
        store->write(str, strlen(str));
    }
    else {
        writeTilesHeader(store, m_hashTable->numTiles());
    }


    KisTileHashTableIterator iter(m_hashTable);
    KisTileSP tile;

    KisAbstractTileCompressorSP compressor =
        KisTileCompressorFactory::create(CURRENT_VERSION);

    while ((tile = iter.tile())) {
        compressor->writeTile(tile, store);
        ++iter;
    }

    return true;
}
bool KisTiledDataManager::read(KoStore *store)
{
    if (!store) return false;
    clear();

    QWriteLocker locker(&m_lock);
    KisMementoSP nothing = m_mementoManager->getMemento();

    QIODevice *stream = store->device();
    if (!stream) {
        m_mementoManager->commit();
        return false;
    }

    const qint32 maxLineLength = 79; // Legacy magic
    QByteArray line = stream->readLine(maxLineLength);
    line = line.trimmed();

    quint32 numTiles;
    qint32 tilesVersion = LEGACY_VERSION;

    if (line[0] == 'V') {
        QList<QByteArray> lineItems = line.split(' ');

        QString keyword = lineItems.takeFirst();
        Q_ASSERT(keyword == "VERSION");

        tilesVersion = lineItems.takeFirst().toInt();

        if(!processTilesHeader(stream, numTiles))
            return false;
    }
    else {
        numTiles = line.toUInt();
    }

    KisAbstractTileCompressorSP compressor =
        KisTileCompressorFactory::create(tilesVersion);

    for (quint32 i = 0; i < numTiles; i++) {
        compressor->readTile(store, this);
    }

    m_mementoManager->commit();
    return true;
}

bool KisTiledDataManager::writeTilesHeader(KoStore *store, quint32 numTiles)
{
    QString buffer;

    buffer = QString("VERSION %1\n"
                     "TILEWIDTH %2\n"
                     "TILEHEIGHT %3\n"
                     "PIXELSIZE %4\n"
                     "DATA %5\n")
        .arg(CURRENT_VERSION)
        .arg(KisTileData::WIDTH)
        .arg(KisTileData::HEIGHT)
        .arg(pixelSize())
        .arg(numTiles);

    store->write(buffer.toAscii());
    return true;
}

#define takeOneLine(stream, maxLine, keyword, value)            \
    do {                                                        \
        QByteArray line = stream->readLine(maxLine);            \
        line = line.trimmed();                                  \
        QList<QByteArray> lineItems = line.split(' ');          \
        keyword = lineItems.takeFirst();                        \
        value = lineItems.takeFirst().toInt();                  \
    } while(0)                                                  \


bool KisTiledDataManager::processTilesHeader(QIODevice *stream, quint32 &numTiles)
{
    /**
     * We assume that there is only one version of this header
     * possible. In case we invent something new, it'll be quite easy
     * to modify the behavior
     */

    const qint32 maxLineLength = 25;
    const qint32 totalNumTests = 4;
    bool foundDataMark = false;
    qint32 testsPassed = 0;

    QString keyword;
    qint32 value;

    while(!foundDataMark && stream->canReadLine()) {
        takeOneLine(stream, maxLineLength, keyword, value);

        if (keyword == "TILEWIDTH") {
            if(value != KisTileData::WIDTH)
                goto wrongString;
        }
        else if (keyword == "TILEHEIGHT") {
            if(value != KisTileData::HEIGHT)
                goto wrongString;
        }
        else if (keyword == "PIXELSIZE") {
            if((quint32)value != pixelSize())
                goto wrongString;
        }
        else if (keyword == "DATA") {
            numTiles = value;
            foundDataMark = true;
        }
        else {
            goto wrongString;
        }

        testsPassed++;
    }

    if(testsPassed != totalNumTests) {
        warnTiles << "Not enough fields of tiles header present"
                  << testsPassed << "of" << totalNumTests;
    }

    return testsPassed == totalNumTests;

wrongString:
    warnTiles << "Wrong string in tiles header:" << keyword << value;
    return false;
}

void KisTiledDataManager::purge(const QRect& area)
{
    QWriteLocker locker(&m_lock);

    QList<KisTileSP> tilesToDelete;
    {
        const qint32 tileDataSize = KisTileData::HEIGHT * KisTileData::WIDTH * pixelSize();
        const quint8 *defaultData = m_hashTable->defaultTileData()->data();

        KisTileHashTableIterator iter(m_hashTable);
        KisTileSP tile;

        while ((tile = iter.tile())) {
            if (tile->extent().intersects(area)) {
                tile->lockForRead();
                if(memcmp(defaultData, tile->data(), tileDataSize) == 0) {
                    tilesToDelete.push_back(tile);
                }
                tile->unlock();
            }
            ++iter;
        }
    }
    foreach(KisTileSP tile, tilesToDelete) {
        m_hashTable->deleteTile(tile);
    }
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
            td = KisTileDataStore::instance()->createDefaultTileData(pixelSize, clearPixel);

        td->acquire();
    }

    for (qint32 row = firstRow; row <= lastRow; ++row) {
        for (qint32 column = firstColumn; column <= lastColumn; ++column) {

            QRect tileRect(column*KisTileData::WIDTH, row*KisTileData::HEIGHT,
                           KisTileData::WIDTH, KisTileData::HEIGHT);
            QRect clearTileRect = clearRect & tileRect;

            if (clearTileRect == tileRect) {
                 // Clear whole tile
                 m_hashTable->deleteTile(column, row);
                 KisTileSP clearedTile = new KisTile(column, row, td, m_mementoManager);
                 m_hashTable->addTile(clearedTile);
                 updateExtent(column, row);
            } else {
                const qint32 lineSize = clearTileRect.width() * pixelSize;
                qint32 rowsRemaining = clearTileRect.height();

                KisTileDataWrapper tw(this,
                                      clearTileRect.left(),
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
    if (td) td->release();
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


template<bool useOldSrcData>
void KisTiledDataManager::bitBltImpl(KisTiledDataManager *srcDM, const QRect &rect)
{
    QWriteLocker locker(&m_lock);

    if (rect.isEmpty()) return;

    const qint32 pixelSize = this->pixelSize();
    const quint32 rowStride = KisTileData::WIDTH * pixelSize;

    qint32 firstColumn = xToCol(rect.left());
    qint32 lastColumn = xToCol(rect.right());

    qint32 firstRow = yToRow(rect.top());
    qint32 lastRow = yToRow(rect.bottom());

    for (qint32 row = firstRow; row <= lastRow; ++row) {
        for (qint32 column = firstColumn; column <= lastColumn; ++column) {

            // this is the only variation in the template
            KisTileSP srcTile = useOldSrcData ?
                srcDM->getOldTile(column, row) :
                srcDM->getTile(column, row, false);

            QRect tileRect(column*KisTileData::WIDTH, row*KisTileData::HEIGHT,
                           KisTileData::WIDTH, KisTileData::HEIGHT);
            QRect cloneTileRect = rect & tileRect;

            if (cloneTileRect == tileRect) {
                 // Clone whole tile
                 m_hashTable->deleteTile(column, row);

                 srcTile->lockForRead();
                 KisTileData *td = srcTile->tileData();
                 KisTileSP clonedTile = new KisTile(column, row, td, m_mementoManager);
                 srcTile->unlock();

                 m_hashTable->addTile(clonedTile);
                 updateExtent(column, row);
            } else {
                const qint32 lineSize = cloneTileRect.width() * pixelSize;
                qint32 rowsRemaining = cloneTileRect.height();

                KisTileDataWrapper tw(this,
                                      cloneTileRect.left(),
                                      cloneTileRect.top(),
                                      KisTileDataWrapper::WRITE);
                srcTile->lockForRead();
                // We suppose that the shift in both tiles is the same
                const quint8* srcTileIt = srcTile->data() + tw.offset();
                quint8* dstTileIt = tw.data();

                while (rowsRemaining > 0) {
                    memcpy(dstTileIt, srcTileIt, lineSize);
                    srcTileIt += rowStride;
                    dstTileIt += rowStride;
                    rowsRemaining--;
                }

                srcTile->unlock();
            }
        }
    }
}

template<bool useOldSrcData>
void KisTiledDataManager::bitBltRoughImpl(KisTiledDataManager *srcDM, const QRect &rect)
{
    QWriteLocker locker(&m_lock);

    if (rect.isEmpty()) return;

    qint32 firstColumn = xToCol(rect.left());
    qint32 lastColumn = xToCol(rect.right());

    qint32 firstRow = yToRow(rect.top());
    qint32 lastRow = yToRow(rect.bottom());

    for (qint32 row = firstRow; row <= lastRow; ++row) {
        for (qint32 column = firstColumn; column <= lastColumn; ++column) {

            /**
             * We are cloning whole tiles here so let's not be so boring
             * to check any borders :)
             */

            // this is the only variation in the template
            KisTileSP srcTile = useOldSrcData ?
                srcDM->getOldTile(column, row) :
                srcDM->getTile(column, row, false);

            m_hashTable->deleteTile(column, row);

            srcTile->lockForRead();
            KisTileData *td = srcTile->tileData();
            KisTileSP clonedTile = new KisTile(column, row, td, m_mementoManager);
            srcTile->unlock();

            m_hashTable->addTile(clonedTile);
            updateExtent(column, row);
        }
    }
}

void KisTiledDataManager::bitBlt(KisTiledDataManager *srcDM, const QRect &rect)
{
    bitBltImpl<false>(srcDM, rect);
}

void KisTiledDataManager::bitBltOldData(KisTiledDataManager *srcDM, const QRect &rect)
{
    bitBltImpl<true>(srcDM, rect);
}

void KisTiledDataManager::bitBltRough(KisTiledDataManager *srcDM, const QRect &rect)
{
    bitBltRoughImpl<false>(srcDM, rect);
}

void KisTiledDataManager::bitBltRoughOldData(KisTiledDataManager *srcDM, const QRect &rect)
{
    bitBltRoughImpl<true>(srcDM, rect);
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
    {
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

    while ((tile = iter.tile())) {
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

QRegion KisTiledDataManager::region() const
{
    QRegion region;

    KisTileHashTableIterator iter(m_hashTable);
    KisTileSP tile;

    while ((tile = iter.tile())) {
        region += tile->extent();
        ++iter;
    }
    return region;
}

void KisTiledDataManager::setPixel(qint32 x, qint32 y, const quint8 * data)
{
    QWriteLocker locker(&m_lock);
    KisTileDataWrapper tw(this, x, y, KisTileDataWrapper::WRITE);
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
    // Actual bytes reading/writing is done in private header
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

