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

typedef QList<KisBaseRectsWalkerSP> KisWalkersList;
typedef QListIterator<KisBaseRectsWalkerSP> KisWalkersListIterator;
typedef QMutableListIterator<KisBaseRectsWalkerSP> KisMutableWalkersListIterator;


class KRITAIMAGE_EXPORT KisSimpleUpdateQueue
{
public:
    KisSimpleUpdateQueue();
    virtual ~KisSimpleUpdateQueue();

    void processQueue(KisUpdaterContext &updaterContext);

    void addUpdateJob(KisNodeSP node, const QRect& rc, const QRect& cropRect);
    void addFullRefreshJob(KisNodeSP node, const QRect& rc, const QRect& cropRect);

    void optimize();

    bool isEmpty() const;
    qint32 sizeMetric() const;

    void updateSettings();

protected:
    void addJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, KisBaseRectsWalker::UpdateType type);

    bool processOneJob(KisUpdaterContext &updaterContext);

    bool trySplitJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, KisBaseRectsWalker::UpdateType type);
    bool tryMergeJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, KisBaseRectsWalker::UpdateType type);

    void collectJobs(KisBaseRectsWalkerSP &baseWalker, QRect baseRect,
                     const KisNodeSP &baseNode, const qreal maxAlpha);
    bool joinRects(QRect& baseRect, const QRect& newRect, qreal maxAlpha);

protected:

    mutable QMutex m_lock;
    KisWalkersList m_list;

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
};

class KRITAIMAGE_EXPORT KisTestableSimpleUpdateQueue : public KisSimpleUpdateQueue
{
public:
    KisWalkersList& getWalkersList();
};

#endif /* __KIS_SIMPLE_UPDATE_QUEUE_H */

