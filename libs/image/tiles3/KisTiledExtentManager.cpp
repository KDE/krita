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

inline bool addTileToMap(int index, QMap<int, QAtomicInt> *map, QReadWriteLock &lock)
{
    bool needsUpdateExtent = false;

    lock.lockForRead();
    auto it = map->find(index);

    if (it == map->end()) {
        lock.unlock();
        lock.lockForWrite();
        map->insert(index, 1);
        lock.unlock();
        needsUpdateExtent = true;
    } else {
        lock.unlock();
        KIS_ASSERT_RECOVER_NOOP(*it > 0);
        (*it)++;
    }

    return needsUpdateExtent;
}

inline bool removeTileFromMap(int index, QMap<int, QAtomicInt> *map, QReadWriteLock &lock)
{
    bool needsUpdateExtent = false;

    lock.lockForRead();
    auto it = map->find(index);

    if (it == map->end()) {
        lock.unlock();
        KIS_ASSERT_RECOVER_NOOP(0 && "sanity check failed: the tile has already been removed!");
    } else {
        lock.unlock();
        KIS_ASSERT_RECOVER_NOOP(*it > 0);
        (*it)--;

        if (*it <= 0) {
            lock.lockForWrite();
            map->erase(it);
            lock.unlock();
            needsUpdateExtent = true;
        }
    }

    return needsUpdateExtent;
}


}

KisTiledExtentManager::KisTiledExtentManager()
{
}

void KisTiledExtentManager::notifyTileAdded(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= addTileToMap(col, &m_colMap, m_colMapGuard);
    needsUpdateExtent |= addTileToMap(row, &m_rowMap, m_rowMapGuard);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::notifyTileRemoved(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= removeTileFromMap(col, &m_colMap, m_colMapGuard);
    needsUpdateExtent |= removeTileFromMap(row, &m_rowMap, m_rowMapGuard);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::replaceTileStats(const QVector<QPoint> &indexes)
{
    m_colMapGuard.lockForWrite();
    m_colMap.clear();
    m_colMapGuard.unlock();

    m_rowMapGuard.lockForWrite();
    m_rowMap.clear();
    m_rowMapGuard.unlock();

    Q_FOREACH (const QPoint &index, indexes) {
        addTileToMap(index.x(), &m_colMap, m_colMapGuard);
        addTileToMap(index.y(), &m_rowMap, m_rowMapGuard);
    }

    updateExtent();
}

void KisTiledExtentManager::clear()
{
    m_colMapGuard.lockForWrite();
    m_colMap.clear();
    m_colMapGuard.unlock();

    m_rowMapGuard.lockForWrite();
    m_rowMap.clear();
    m_rowMapGuard.unlock();

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
    QReadLocker cl(&m_colMapGuard);
    QReadLocker rl(&m_rowMapGuard);
    KIS_ASSERT_RECOVER_RETURN(m_colMap.isEmpty() == m_rowMap.isEmpty());

    // here we check for only one map for efficiency reasons
    if (m_colMap.isEmpty()) {
        QWriteLocker l(&m_mutex);
        m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
    } else {
        const int minX = m_colMap.firstKey() * KisTileData::WIDTH;
        const int maxPlusOneX = (m_colMap.lastKey() + 1) * KisTileData::WIDTH;
        const int minY = m_rowMap.firstKey() * KisTileData::HEIGHT;
        const int maxPlusOneY = (m_rowMap.lastKey() + 1) * KisTileData::HEIGHT;

        QWriteLocker l(&m_mutex);
        m_currentExtent =
            QRect(minX, minY,
                  maxPlusOneX - minX,
                  maxPlusOneY - minY);
    }
}

