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

#ifndef KISTILEDEXTENTMANAGER_H
#define KISTILEDEXTENTMANAGER_H

#include <QMutex>
#include <QReadWriteLock>
#include <QMap>
#include <QRect>
#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisTiledExtentManager
{
    static const qint32 InitialBufferSize = 256;

    class Data
    {
    public:
        Data();
        ~Data();

        inline bool add(qint32 index);
        inline bool remove(qint32 index);
        void replace(const QVector<qint32> &indexes);
        void clear();
        bool isEmpty();
        qint32 min();
        qint32 max();

    public:
        QReadWriteLock m_extentLock;

    private:
        inline void unsafeAdd(qint32 index);
        inline void unsafeMigrate(qint32 index);
        inline void migrate(qint32 index);
        inline void updateMin();
        inline void updateMax();

    private:
        qint32 m_min;
        qint32 m_max;
        qint32 m_offset;
        qint32 m_capacity;
        qint32 m_count;
        QAtomicInt *m_buffer;
        QReadWriteLock m_migrationLock;
    };

public:
    KisTiledExtentManager();

    void notifyTileAdded(qint32 col, qint32 row);
    void notifyTileRemoved(qint32 col, qint32 row);
    void replaceTileStats(const QVector<QPoint> &indexes);
    void clear();
    QRect extent() const;

private:
    void updateExtent();

private:
    mutable QReadWriteLock m_extentLock;
    QRect m_currentExtent;
    Data m_colsData;
    Data m_rowsData;
};

#endif // KISTILEDEXTENTMANAGER_H
