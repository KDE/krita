/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SIMPLE_UPDATE_QUEUE_H
#define __KIS_SIMPLE_UPDATE_QUEUE_H

#include <QMutex>
#include "kis_updater_context.h"

//typedef QList<KisBaseRectsWalkerSP> KisWalkersList;
//typedef QListIterator<KisBaseRectsWalkerSP> KisWalkersListIterator;
//typedef QMutableListIterator<KisBaseRectsWalkerSP> KisMutableWalkersListIterator;

//typedef QList<KisSpontaneousJob*> KisSpontaneousJobsList;
//typedef QListIterator<KisSpontaneousJob*> KisSpontaneousJobsListIterator;
//typedef QMutableListIterator<KisSpontaneousJob*> KisMutableSpontaneousJobsListIterator;

template <class T>
class ConcurrentListIterator;

template <class T>
class ConcurrentList
{
public:
    void append(T &item)
    {
        QWriteLocker locker(&m_rwLock);
        m_list.append(item);
    }

    void append(QList<T> &list)
    {
        QWriteLocker locker(&m_rwLock);
        m_list.append(list);
    }

    T &first()
    {
        QReadLocker locker(&m_rwLock);
        return m_list.first();
    }

    int size() const
    {
        QReadLocker locker(&m_rwLock);
        return m_list.size();
    }

    bool isEmpty() const
    {
        QReadLocker locker(&m_rwLock);
        return m_list.isEmpty();
    }

    T takeFirst()
    {
        QWriteLocker locker(&m_rwLock);
        return m_list.takeFirst();
    }

    T takeLast()
    {
        QWriteLocker locker(&m_rwLock);
        return m_list.takeLast();
    }

    T &operator[](int i)
    {
        return m_list[i];
    }

    ConcurrentList<T> &operator=(const ConcurrentList<T> &l)
    {
         m_list = l.m_list;
         return *this;
    }

    friend class ConcurrentListIterator<T>;

private:
    QList<T> m_list;
    mutable QReadWriteLock m_rwLock;
};

template <class T>
class ConcurrentListIterator
{
public:
    ConcurrentListIterator(ConcurrentList<T> &list, bool writeLock = false) :
        m_pos(0), m_reversed(false), m_writeLock(writeLock), m_list(list)
    {
        if (m_writeLock) {
            m_list.m_rwLock.lockForWrite();
        } else {
            m_list.m_rwLock.lockForRead();
        }
    }

    void unlock()
    {
        m_list.m_rwLock.unlock();
    }

    bool hasNext() const
    {
        return m_pos < m_list.m_list.size();
    }

    bool hasPrevious() const
    {
        return m_pos > -1;
    }

    T &next()
    {
        return m_list.m_list[m_pos++];
    }

    T &previous()
    {
        return m_list.m_list[m_pos--];
    }

    void toBack()
    {
        m_pos = m_list.m_list.size() - 1;
    }

    bool remove(T &item)
    {
        bool result = true;

        if (m_writeLock) {
            if (!m_reversed) {
                if (m_list.m_list[m_pos - 1] == item) {
                    m_list.m_list.removeAt(--m_pos);
                    result = true;
                }
            } else {
                if (m_list.m_list[m_pos + 1] == item) {
                    m_list.m_list.removeAt(m_pos + 1);
                    result = true;
                }
            }
        }

        return result;
    }

private:
    int m_pos;
    bool m_reversed;
    bool m_writeLock;
    ConcurrentList<T> &m_list;
};

typedef ConcurrentList<KisBaseRectsWalkerSP> KisWalkersList;
typedef ConcurrentListIterator<KisBaseRectsWalkerSP> KisWalkersListIterator;
typedef ConcurrentListIterator<KisBaseRectsWalkerSP> KisMutableWalkersListIterator;

typedef ConcurrentList<KisSpontaneousJob*> KisSpontaneousJobsList;
typedef ConcurrentListIterator<KisSpontaneousJob*> KisMutableSpontaneousJobsListIterator;

class KRITAIMAGE_EXPORT KisSimpleUpdateQueue
{
public:
    KisSimpleUpdateQueue();
    virtual ~KisSimpleUpdateQueue();

    void processQueue(KisUpdaterContext &updaterContext);

    void addUpdateJob(KisNodeSP node, const QVector<QRect> &rects, const QRect& cropRect, int levelOfDetail);
    void addUpdateJob(KisNodeSP node, const QRect &rc, const QRect& cropRect, int levelOfDetail);
    void addUpdateNoFilthyJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail);
    void addFullRefreshJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail);
    void addSpontaneousJob(KisSpontaneousJob *spontaneousJob);


    void optimize();

    bool isEmpty() const;
    qint32 sizeMetric() const;

    void updateSettings();

    int overrideLevelOfDetail() const;

protected:
    void addJob(KisNodeSP node, const QVector<QRect> &rects, const QRect& cropRect, int levelOfDetail, KisBaseRectsWalker::UpdateType type);

    bool processOneJob(KisUpdaterContext &updaterContext);

    bool trySplitJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail, KisBaseRectsWalker::UpdateType type);
    bool tryMergeJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail, KisBaseRectsWalker::UpdateType type);

    void collectJobs(KisBaseRectsWalkerSP &baseWalker, QRect baseRect,
                     const qreal maxAlpha);
    bool joinRects(QRect& baseRect, const QRect& newRect, qreal maxAlpha);

protected:

//    mutable QMutex m_mutex;
    KisWalkersList m_updatesList;
    KisSpontaneousJobsList m_spontaneousJobsList;

    /**
     * Parameters of optimization
     * (loaded from a configuration file)
     */

    /**
     * Big update areas are split into a set of smaller
     * ones, m_patchWidth and m_patchHeight represent the
     * size of these areas.
     */
    qint32 m_patchWidth;
    qint32 m_patchHeight;

    /**
     * Maximum coefficient of work while regular optimization()
     */
    qreal m_maxCollectAlpha;

    /**
     * Maximum coefficient of work when to rects are considered
     * similar and are merged in tryMergeJob()
     */
    qreal m_maxMergeAlpha;

    /**
     * The coefficient of work used while collecting phase of tryToMerge()
     */
    qreal m_maxMergeCollectAlpha;

    int m_overrideLevelOfDetail;
};

class KRITAIMAGE_EXPORT KisTestableSimpleUpdateQueue : public KisSimpleUpdateQueue
{
public:
    KisWalkersList& getWalkersList();
    KisSpontaneousJobsList& getSpontaneousJobsList();
};

#endif /* __KIS_SIMPLE_UPDATE_QUEUE_H */

