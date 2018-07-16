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

KisTiledExtentManager::Data::Data()
    : m_min(qint32_MAX), m_max(qint32_MIN), m_count(0)
{
    QWriteLocker l(&m_lock);
    m_capacity = InitialBufferSize;
    m_offset = m_capacity >> 1;
    m_buffer = new QAtomicInt[m_capacity];
}

KisTiledExtentManager::Data::~Data()
{
    QWriteLocker l(&m_lock);
    delete[] m_buffer;
}

inline bool KisTiledExtentManager::Data::add(qint32 index)
{
    QReadLocker l(&m_lock);
    qint32 currentIndex = m_offset + index;

    if (currentIndex < 0 || currentIndex >= m_capacity) {
        l.unlock();
        migrate(index);
        l.relock();
        currentIndex = m_offset + index;
    }

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() >= 0);
    bool needsUpdateExtent = false;

    if (!m_buffer[currentIndex].fetchAndAddOrdered(1)) {
        QReadLocker rl(&m_minMaxLock);
        bool needsCheck = true;

        if (m_min > index) {
            rl.unlock();
            needsCheck = false;
            QWriteLocker wl(&m_minMaxLock);
            m_min = index;
        }

        if (needsCheck) {
            if (m_max < index) {
                rl.unlock();
                QWriteLocker wl(&m_minMaxLock);
                m_max = index;
            }
        }

        needsUpdateExtent = true;
        m_count.ref();
    }

    return needsUpdateExtent;
}

inline bool KisTiledExtentManager::Data::remove(qint32 index)
{
    QReadLocker l(&m_lock);
    qint32 currentIndex = m_offset + index;

    if (currentIndex < 0 || currentIndex >= m_capacity) {
        l.unlock();
        migrate(index);
        l.relock();
        currentIndex = m_offset + index;
    }

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() > 0);
    bool needsUpdateExtent = false;

    if (!m_buffer[currentIndex].deref()) {
        QReadLocker rl(&m_minMaxLock);
        bool needsCheck = true;

        if (m_min == index) {
            rl.unlock();
            needsCheck = false;
            QWriteLocker wl(&m_minMaxLock);
            updateMin();
        }

        if (needsCheck) {
            if (m_max == index) {
                rl.unlock();
                QWriteLocker wl(&m_minMaxLock);
                updateMax();
            }
        }

        needsUpdateExtent = true;
        m_count.deref();
    }

    return needsUpdateExtent;
}

void KisTiledExtentManager::Data::replace(const QVector<qint32> &indexes)
{
    QWriteLocker lock(&m_lock);
    QWriteLocker minMaxLock(&m_minMaxLock);

    for (qint32 i = 0; i < m_capacity; ++i) {
        m_buffer[i].store(0);
    }

    m_min = qint32_MAX;
    m_max = qint32_MIN;
    m_count.store(0);

    Q_FOREACH (const qint32 index, indexes) {
        unsafeAdd(index);
    }
}

void KisTiledExtentManager::Data::clear()
{
    QWriteLocker lock(&m_lock);
    QWriteLocker minMaxLock(&m_minMaxLock);

    for (qint32 i = 0; i < m_capacity; ++i) {
        m_buffer[i].store(0);
    }

    m_min = qint32_MAX;
    m_max = qint32_MIN;
    m_count.store(0);
}

/*
 * We don't take locks in isEmpty(), min() and max(),
 * cause it will be taken in updateExtent()
 */

bool KisTiledExtentManager::Data::isEmpty()
{
    return m_count.loadAcquire() == 0;
}

qint32 KisTiledExtentManager::Data::min()
{
    return m_min;
}

qint32 KisTiledExtentManager::Data::max()
{
    return m_max;
}

void KisTiledExtentManager::Data::unsafeAdd(qint32 index)
{
    qint32 currentIndex = m_offset + index;

    if (currentIndex < 0 || currentIndex >= m_capacity) {
        unsafeMigrate(index);
        currentIndex = m_offset + index;
    }

    if (!m_buffer[currentIndex].fetchAndAddOrdered(1)) {
        if (m_min > index) m_min = index;
        if (m_max < index) m_max = index;
        m_count.ref();
    }
}

void KisTiledExtentManager::Data::unsafeMigrate(qint32 index)
{
    qint32 oldCapacity = m_capacity;
    qint32 oldOffset = m_offset;
    qint32 currentIndex = m_offset + index;

    while (currentIndex < 0 || currentIndex >= m_capacity) {
        m_capacity <<= 1;
        m_offset <<= 1;
        currentIndex = m_offset + index;
    }

    if (m_capacity != oldCapacity) {
        QAtomicInt *newBuffer = new QAtomicInt[m_capacity];
        qint32 start = m_offset - oldOffset;

        for (qint32 i = 0; i < oldCapacity; ++i) {
            newBuffer[start + i].store(m_buffer[i].load());
        }

        delete[] m_buffer;
        m_buffer = newBuffer;
    }
}

void KisTiledExtentManager::Data::migrate(qint32 index)
{
    QWriteLocker l(&m_lock);
    unsafeMigrate(index);
}

/*
 * We don't take locks in updateMin() and updateMax(),
 * cause it will be taken in add/remove when incrementing from 0 to 1
 * or decrementing from 1 to 0
 */

void KisTiledExtentManager::Data::updateMin()
{
    for (qint32 i = 0; i < m_capacity; ++i) {
        qint32 current = m_buffer[i].load();

        if (current > 0) {
            m_min = current;
            break;
        }
    }
}

void KisTiledExtentManager::Data::updateMax()
{
    for (qint32 i = m_capacity - 1; i >= 0; ++i) {
        qint32 current = m_buffer[i].load();

        if (current > 0) {
            m_max = current;
            break;
        }
    }
}

KisTiledExtentManager::KisTiledExtentManager()
{
}

void KisTiledExtentManager::notifyTileAdded(qint32 col, qint32 row)
{
    bool needsUpdateExtent = false;

    needsUpdateExtent |= m_colsData.add(col);
    needsUpdateExtent |= m_rowsData.add(row);

    if (needsUpdateExtent) {
        updateExtent();
    }
}

void KisTiledExtentManager::notifyTileRemoved(qint32 col, qint32 row)
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
    QVector<qint32> colsIndexes;
    QVector<qint32> rowsIndexes;

    Q_FOREACH (const QPoint &index, indexes) {
        colsIndexes.append(index.x());
        rowsIndexes.append(index.y());
    }

    m_colsData.replace(colsIndexes);
    m_rowsData.replace(rowsIndexes);
    updateExtent();
}

void KisTiledExtentManager::clear()
{
    m_colsData.clear();
    m_rowsData.clear();

    QWriteLocker writeLock(&m_extentLock);
    m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
}

QRect KisTiledExtentManager::extent() const
{
    QReadLocker readLock(&m_extentLock);
    return m_currentExtent;
}

void KisTiledExtentManager::updateExtent()
{
    bool colsEmpty = m_colsData.isEmpty();
    bool rowsEmpty = m_rowsData.isEmpty();
    KIS_ASSERT_RECOVER_RETURN(colsEmpty == rowsEmpty);

    if (colsEmpty && rowsEmpty) {
        QWriteLocker writeLock(&m_extentLock);
        m_currentExtent = QRect(qint32_MAX, qint32_MAX, 0, 0);
    } else {
        QReadLocker cl(&m_colsData.m_minMaxLock);
        QReadLocker rl(&m_rowsData.m_minMaxLock);

        const qint32 minX = m_colsData.min() * KisTileData::WIDTH;
        const qint32 maxPlusOneX = (m_colsData.max() + 1) * KisTileData::WIDTH;
        const qint32 minY = m_rowsData.min() * KisTileData::HEIGHT;
        const qint32 maxPlusOneY = (m_rowsData.max() + 1) * KisTileData::HEIGHT;

        QWriteLocker writeLock(&m_extentLock);
        m_currentExtent =
            QRect(minX, minY,
                  maxPlusOneX - minX,
                  maxPlusOneY - minY);
    }
}
