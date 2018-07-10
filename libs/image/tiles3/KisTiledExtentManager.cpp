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
#include "kis_debug.h"

KisTiledExtentManager::Data::Data()
    : m_min(INT_MAX), m_max(INT_MIN)
{
    QWriteLocker l(&m_lock);
    m_capacity = InitialBufferSize;
    m_offset = m_capacity >> 1;
    m_buffer = new QAtomicInt[m_capacity];
}

KisTiledExtentManager::Data::~Data()
{
    QWriteLocker l(&m_lock);
    delete m_buffer;
}

inline bool KisTiledExtentManager::Data::add(int index)
{
    QReadLocker l(&m_lock);
    int currentIndex = m_offset + index;

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() >= 0);
    bool needsUpdateExtent = false;

    if (m_min > index) m_min.storeRelease(index);
    if (m_max < index) m_max.storeRelease(index);

    if (!m_buffer[currentIndex].fetchAndAddOrdered(1)) {
        m_count.ref();
        needsUpdateExtent = true;
    }

    return needsUpdateExtent;
}

inline bool KisTiledExtentManager::Data::remove(int index)
{
    QReadLocker l(&m_lock);
    int currentIndex = m_offset + index;

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() >= 0);
    bool needsUpdateExtent = false;

    if (!m_buffer[currentIndex].fetchAndSubOrdered(1)) {
        if (m_min == index) updateMin();
        if (m_max == index) updateMax();

        m_count.deref();
        needsUpdateExtent = true;
    }

    return needsUpdateExtent;
}

void KisTiledExtentManager::Data::clear()
{
    m_count.storeRelease(0);
    QWriteLocker l(&m_lock);

    for (int i = 0; i < m_capacity; ++i) {
        m_buffer[i] = 0;
    }
}

bool KisTiledExtentManager::Data::isEmpty()
{
    return m_count.loadAcquire() == 0;
}

int KisTiledExtentManager::Data::min()
{
    return m_min.loadAcquire();
}

int KisTiledExtentManager::Data::max()
{
    return m_max.loadAcquire();
}

void KisTiledExtentManager::Data::updateMin()
{
    for (int i = 0; i < m_capacity; ++i) {
        if (m_buffer[i] > 0) {
            m_min.storeRelease(m_buffer[i]);
            break;
        }
    }
}

void KisTiledExtentManager::Data::updateMax()
{
    for (int i = m_capacity - 1; i >= 0; ++i) {
        if (m_buffer[i] > 0) {
            m_max.storeRelease(m_buffer[i]);
            break;
        }
    }
}

KisTiledExtentManager::KisTiledExtentManager()
{
}

void KisTiledExtentManager::notifyTileAdded(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= m_colsData.add(col);
    needsUpdateExtent |= m_rowsData.add(row);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::notifyTileRemoved(int col, int row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= m_colsData.remove(col);
    needsUpdateExtent |= m_rowsData.remove(row);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::replaceTileStats(const QVector<QPoint> &indexes)
{
    m_colsData.clear();
    m_rowsData.clear();

    Q_FOREACH (const QPoint &index, indexes) {
        m_colsData.add(index.x());
        m_rowsData.add(index.y());
    }

    updateExtent();
}

void KisTiledExtentManager::clear()
{
    m_colsData.clear();
    m_rowsData.clear();

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
    bool colsEmpty = m_colsData.isEmpty();
    bool rowsEmpty = m_rowsData.isEmpty();

    KIS_ASSERT_RECOVER_RETURN(colsEmpty == rowsEmpty);

    if (colsEmpty && rowsEmpty) {
        QWriteLocker l(&m_mutex);
        m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
    } else {
        const int minX = m_colsData.min() * KisTileData::WIDTH;
        const int maxPlusOneX = (m_colsData.max() + 1) * KisTileData::WIDTH;
        const int minY = m_rowsData.min() * KisTileData::HEIGHT;
        const int maxPlusOneY = (m_rowsData.max() + 1) * KisTileData::HEIGHT;

        QWriteLocker l(&m_mutex);
        m_currentExtent =
            QRect(minX, minY,
                  maxPlusOneX - minX,
                  maxPlusOneY - minY);
    }
}
