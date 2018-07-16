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

#ifndef KISTILEDEXTENTMANAGER_H
#define KISTILEDEXTENTMANAGER_H

#include <QMutex>
#include <QReadWriteLock>
#include <QMap>
#include <QRect>
#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisTiledExtentManager
{
    static const int InitialBufferSize = 8;

    class Data
    {
    public:
        Data();
        ~Data();

        inline bool add(int index);
        inline bool remove(int index);
        void clear();
        bool isEmpty();
        int min();
        int max();

    public:
        mutable QReadWriteLock m_minMaxLock;

    private:
        void migrate(int index);
        void updateMin();
        void updateMax();

    private:
        int m_min;
        int m_max;
        int m_offset;
        int m_capacity;
        QAtomicInt m_count;
        QAtomicInt *m_buffer;
        QReadWriteLock m_lock;
    };

public:
    KisTiledExtentManager();

    void notifyTileAdded(int col, int row);
    void notifyTileRemoved(int col, int row);
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
