/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Andrey Kamakin <a.kamakin@icloud.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    class KRITAIMAGE_EXPORT Data
    {
    public:
        Data();
        ~Data();

        bool add(qint32 index);
        bool remove(qint32 index);
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
    friend class KisTiledDataManagerTest;

private:
    mutable QReadWriteLock m_extentLock;
    QRect m_currentExtent;
    Data m_colsData;
    Data m_rowsData;
};

#endif // KISTILEDEXTENTMANAGER_H
