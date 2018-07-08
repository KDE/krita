/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisTiledExtentManager.h"

#include <QMutex>
#include <QVector>
#include "kis_tile_data_interface.h"
#include "kis_assert.h"
#include "kis_global.h"

namespace {

inline bool addTileToMap(int index, QAtomicInt array[],
                         QAtomicInt &min, QAtomicInt &max, QReadWriteLock &lock)
{
    QReadLocker l(&lock);
    KIS_ASSERT_RECOVER_NOOP(array[512 + index].loadAcquire() >= 0);

    bool needsUpdateExtent = false;
    if (min > index) min.storeRelease(index);
    if (max < index) max.storeRelease(index);

    if (!array[512 + index].fetchAndAddOrdered(1)) {
        array[0].ref();
        needsUpdateExtent = true;
    }

    return needsUpdateExtent;
}

void updateMin(QAtomicInt array[], QAtomicInt &min)
{
    for (int i = 1; i < 1024; ++i) {
        if (array[i] > 0) {
            min.storeRelease(array[i]);
            break;
        }
    }
}

void updateMax(QAtomicInt array[], QAtomicInt &max)
{
    for (int i = 1023; i > 0; ++i) {
        if (array[i] > 0) {
            max.storeRelease(array[i]);
            break;
        }
    }
}

inline bool removeTileFromMap(int index, QAtomicInt array[],
                              QAtomicInt &min, QAtomicInt &max, QReadWriteLock &lock)
{
    QReadLocker l(&lock);
    KIS_ASSERT_RECOVER_NOOP(array[512 + index].loadAcquire() >= 0);

    bool needsUpdateExtent = false;
    array[512 + index].deref();

    if (!array[512 + index].loadAcquire()) {
        if (min == index) {
            updateMin(array, min);
        }

        if (max == index) {
            updateMax(array, max);
        }

        array[0].deref();
        needsUpdateExtent = true;
    }

    return needsUpdateExtent;
}


}

KisTiledExtentManager::KisTiledExtentManager()
    : m_minCol(INT_MAX),
      m_maxCol(INT_MIN),
      m_minRow(INT_MAX),
      m_maxRow(INT_MIN)
{
}

void KisTiledExtentManager::notifyTileAdded(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= addTileToMap(col, m_colArray, m_minCol, m_maxCol, m_colArrayLock);
    needsUpdateExtent |= addTileToMap(row, m_rowArray, m_minRow, m_maxRow, m_rowArrayLock);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::notifyTileRemoved(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= removeTileFromMap(col, m_colArray, m_minCol, m_maxCol, m_colArrayLock);
    needsUpdateExtent |= removeTileFromMap(row, m_rowArray, m_minRow, m_maxRow, m_rowArrayLock);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void clearArray(QAtomicInt array[], QReadWriteLock &lock)
{
    QWriteLocker l(&lock);

    for (int i = 0; i < 1024; ++i) {
        array[i] = 0;
    }
}

void KisTiledExtentManager::replaceTileStats(const QVector<QPoint> &indexes)
{
    clearArray(m_colArray, m_colArrayLock);
    clearArray(m_rowArray, m_rowArrayLock);

    Q_FOREACH (const QPoint &index, indexes) {
        addTileToMap(index.x(), m_colArray, m_minCol, m_maxCol, m_colArrayLock);
        addTileToMap(index.y(), m_rowArray, m_minRow, m_maxRow, m_rowArrayLock);
    }

    updateExtent();
}

void KisTiledExtentManager::clear()
{
    clearArray(m_colArray, m_colArrayLock);
    clearArray(m_rowArray, m_rowArrayLock);

    QWriteLocker l(&m_mutex);
    m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);

}

QRect KisTiledExtentManager::extent() const
{
    QReadLocker l(&m_mutex);
    return m_currentExtent;
}

void KisTiledExtentManager::updateExtent()
{
    bool colArrayEmpty = false;
    bool rowArrayEmpty = false;
    {
        QReadLocker l(&m_colArrayLock);
        colArrayEmpty = m_colArray[0] == 0;
    }
    {
        QReadLocker l(&m_rowArrayLock);
        rowArrayEmpty = m_rowArray[0] == 0;
    }

    KIS_ASSERT_RECOVER_RETURN(colArrayEmpty == rowArrayEmpty);

    // here we check for only one map for efficiency reasons
    if (colArrayEmpty && rowArrayEmpty) {
        QWriteLocker l(&m_mutex);
        m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
    } else {
        const int minX = m_minCol.loadAcquire() * KisTileData::WIDTH;
        const int maxPlusOneX = (m_maxCol.loadAcquire() + 1) * KisTileData::WIDTH;
        const int minY = m_minRow.loadAcquire() * KisTileData::HEIGHT;
        const int maxPlusOneY = (m_maxRow.loadAcquire() + 1) * KisTileData::HEIGHT;

        QWriteLocker l(&m_mutex);
        m_currentExtent =
            QRect(minX, minY,
                  maxPlusOneX - minX,
                  maxPlusOneY - minY);
    }
}

