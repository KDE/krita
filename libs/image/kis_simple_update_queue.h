/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SIMPLE_UPDATE_QUEUE_H
#define __KIS_SIMPLE_UPDATE_QUEUE_H

#include <QMutex>
#include "kis_updater_context.h"

typedef QList<KisBaseRectsWalkerSP> KisWalkersList;
typedef QListIterator<KisBaseRectsWalkerSP> KisWalkersListIterator;
typedef QMutableListIterator<KisBaseRectsWalkerSP> KisMutableWalkersListIterator;

typedef QList<KisSpontaneousJob*> KisSpontaneousJobsList;
typedef QListIterator<KisSpontaneousJob*> KisSpontaneousJobsListIterator;
typedef QMutableListIterator<KisSpontaneousJob*> KisMutableSpontaneousJobsListIterator;


class KRITAIMAGE_EXPORT KisSimpleUpdateQueue
{
public:
    KisSimpleUpdateQueue();
    virtual ~KisSimpleUpdateQueue();

    void processQueue(KisUpdaterContext &updaterContext);

    void addUpdateJob(KisNodeSP node, const QVector<QRect> &rects, const QRect& cropRect, int levelOfDetail);
    void addUpdateJob(KisNodeSP node, const QRect &rc, const QRect& cropRect, int levelOfDetail);
    void addUpdateNoFilthyJob(KisNodeSP node, const QVector<QRect>& rc, const QRect& cropRect, int levelOfDetail);
    void addUpdateNoFilthyJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail);
    void addFullRefreshJob(KisNodeSP node, const QRect &rc, const QRect& cropRect, int levelOfDetail);
    void addFullRefreshJob(KisNodeSP node, const QVector<QRect> &rects, const QRect& cropRect, int levelOfDetail);
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

    mutable QMutex m_lock;
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

