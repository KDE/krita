/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2018 Andrey Kamakin <a.kamakin@icloud.com>
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
    : m_min(qint32_MAX), m_max(qint32_MIN), m_count(0)
{
    QWriteLocker lock(&m_migrationLock);
    m_capacity = InitialBufferSize;
    m_offset = 1;
    m_buffer = new QAtomicInt[m_capacity];
}

KisTiledExtentManager::Data::~Data()
{
    QWriteLocker lock(&m_migrationLock);
    delete[] m_buffer;
}

inline bool KisTiledExtentManager::Data::add(qint32 index)
{
    QReadLocker lock(&m_migrationLock);
    qint32 currentIndex = m_offset + index;

    if (currentIndex < 0 || currentIndex >= m_capacity) {
        lock.unlock();
        migrate(index);
        lock.relock();
        currentIndex = m_offset + index;
    }

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() >= 0);
    bool needsUpdateExtent = false;
    QReadLocker rl(&m_extentLock);

    if (!m_buffer[currentIndex].loadAcquire()) {
        rl.unlock();
        QWriteLocker wl(&m_extentLock);

        if (!m_buffer[currentIndex].load()) {
            m_buffer[currentIndex].store(1);

            if (m_min > index) m_min = index;
            if (m_max < index) m_max = index;

            ++m_count;
            needsUpdateExtent = true;
        } else {
            m_buffer[currentIndex].ref();
        }
    } else {
        m_buffer[currentIndex].ref();
    }

    return needsUpdateExtent;
}

inline bool KisTiledExtentManager::Data::remove(qint32 index)
{
    QReadLocker lock(&m_migrationLock);
    qint32 currentIndex = m_offset + index;

    KIS_ASSERT_RECOVER_NOOP(m_buffer[currentIndex].loadAcquire() > 0);
    bool needsUpdateExtent = false;
    QReadLocker rl(&m_extentLock);

    if (m_buffer[currentIndex].loadAcquire() == 1) {
        rl.unlock();
        QWriteLocker wl(&m_extentLock);

        if (m_buffer[currentIndex].load() == 1) {
            m_buffer[currentIndex].store(0);

            if (m_min == index) updateMin();
            if (m_max == index) updateMax();

            --m_count;
            needsUpdateExtent = true;
        } else {
            m_buffer[currentIndex].deref();
        }
    } else {
        m_buffer[currentIndex].deref();
    }

    return needsUpdateExtent;
}

void KisTiledExtentManager::Data::replace(const QVector<qint32> &indexes)
{
    QWriteLocker lock(&m_migrationLock);
    QWriteLocker l(&m_extentLock);

    for (qint32 i = 0; i < m_capacity; ++i) {
        m_buffer[i].store(0);
    }

    m_min = qint32_MAX;
    m_max = qint32_MIN;
    m_count = 0;

    Q_FOREACH (const qint32 index, indexes) {
        unsafeAdd(index);
    }
}

void KisTiledExtentManager::Data::clear()
{
    QWriteLocker lock(&m_migrationLock);
    QWriteLocker l(&m_extentLock);

    for (qint32 i = 0; i < m_capacity; ++i) {
        m_buffer[i].store(0);
    }

    m_min = qint32_MAX;
    m_max = qint32_MIN;
    m_count = 0;
}

bool KisTiledExtentManager::Data::isEmpty()
{
    return m_count == 0;
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

    if (!m_buffer[currentIndex].fetchAndAddRelaxed(1)) {
        if (m_min > index) m_min = index;
        if (m_max < index) m_max = index;
        ++m_count;
    }
}

void KisTiledExtentManager::Data::unsafeMigrate(qint32 index)
{
    qint32 oldCapacity = m_capacity;
    qint32 oldOffset = m_offset;
    qint32 currentIndex = m_offset + index;

    while (currentIndex < 0 || currentIndex >= m_capacity) {
        m_capacity <<= 1;

        if (currentIndex < 0) {
            m_offset <<= 1;
            currentIndex = m_offset + index;
        }
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
    QWriteLocker lock(&m_migrationLock);
    unsafeMigrate(index);
}

void KisTiledExtentManager::Data::updateMin()
{
    qint32 start = m_min + m_offset;

    for (qint32 i = start; i < m_capacity; ++i) {
        qint32 current = m_buffer[i].load();

        if (current > 0) {
            m_min = i - m_offset;
            break;
        }
    }
}

void KisTiledExtentManager::Data::updateMax()
{
    qint32 start = m_max + m_offset;

    for (qint32 i = start; i >= 0; --i) {
        qint32 current = m_buffer[i].load();

        if (current > 0) {
            m_max = i - m_offset;
            break;
        }
    }
}

KisTiledExtentManager::KisTiledExtentManager()
{
    QWriteLocker l(&m_extentLock);
    m_currentExtent = QRect();
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

    QWriteLocker lock(&m_extentLock);
    m_currentExtent = QRect();
}

QRect KisTiledExtentManager::extent() const
{
    QReadLocker lock(&m_extentLock);
    return m_currentExtent;
}

void KisTiledExtentManager::updateExtent()
{
    qint32 minX, width, minY, height;

    {
        QReadLocker cl(&m_colsData.m_extentLock);

        if (m_colsData.isEmpty()) {
            minX = 0;
            width = 0;
        } else {
            minX = m_colsData.min() * KisTileData::WIDTH;
            width = (m_colsData.max() + 1) * KisTileData::WIDTH - minX;
        }
    }

    {
        QReadLocker rl(&m_rowsData.m_extentLock);

        if (m_rowsData.isEmpty()) {
            minY = 0;
            height = 0;
        } else {
            minY = m_rowsData.min() * KisTileData::HEIGHT;
            height = (m_rowsData.max() + 1) * KisTileData::HEIGHT - minY;
        }
    }

    QWriteLocker lock(&m_extentLock);
    m_currentExtent = QRect(minX, minY, width, height);
}
