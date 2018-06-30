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

inline bool addTileToMap(int index, QMap<int, int> *map)
{
    bool needsUpdateExtent = false;

    auto it = map->find(index);

    if (it == map->end()) {
        map->insert(index, 1);
        needsUpdateExtent = true;
    } else {
        KIS_ASSERT_RECOVER_NOOP(*it > 0);
        (*it)++;
    }

    return needsUpdateExtent;
}

inline bool removeTileFromMap(int index, QMap<int, int> *map)
{
    bool needsUpdateExtent = false;

    auto it = map->find(index);

    if (it == map->end()) {
        KIS_ASSERT_RECOVER_NOOP(0 && "sanity check failed: the tile has already been removed!");
    } else {
        KIS_ASSERT_RECOVER_NOOP(*it > 0);
        (*it)--;

        if (*it <= 0) {
            map->erase(it);
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
    QMutexLocker l(&m_mutex);

    bool needsUpdateExtent = false;

    needsUpdateExtent |= addTileToMap(col, &m_colMap);
    needsUpdateExtent |= addTileToMap(row, &m_rowMap);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::notifyTileRemoved(int col, int row)
{
    QMutexLocker l(&m_mutex);

    bool needsUpdateExtent = false;

    needsUpdateExtent |= removeTileFromMap(col, &m_colMap);
    needsUpdateExtent |= removeTileFromMap(row, &m_rowMap);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::replaceTileStats(const QVector<QPoint> &indexes)
{
    QMutexLocker l(&m_mutex);

    m_colMap.clear();
    m_rowMap.clear();

    Q_FOREACH (const QPoint &index, indexes) {
        addTileToMap(index.x(), &m_colMap);
        addTileToMap(index.y(), &m_rowMap);
    }

    updateExtent();
}

void KisTiledExtentManager::clear()
{
    QMutexLocker l(&m_mutex);

    m_colMap.clear();
    m_rowMap.clear();
    m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);

}

QRect KisTiledExtentManager::extent() const
{
    QMutexLocker l(&m_mutex);
    return m_currentExtent;
}

void KisTiledExtentManager::updateExtent()
{
    KIS_ASSERT_RECOVER_RETURN(m_colMap.isEmpty() == m_rowMap.isEmpty());

    // here we check for only one map for efficiency reasons
    if (m_colMap.isEmpty()) {
        m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
    } else {
        const int minX = m_colMap.firstKey() * KisTileData::WIDTH;
        const int maxPlusOneX = (m_colMap.lastKey() + 1) * KisTileData::WIDTH;
        const int minY = m_rowMap.firstKey() * KisTileData::HEIGHT;
        const int maxPlusOneY = (m_rowMap.lastKey() + 1) * KisTileData::HEIGHT;

        m_currentExtent =
            QRect(minX, minY,
                  maxPlusOneX - minX,
                  maxPlusOneY - minY);
    }
}

