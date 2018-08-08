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

#include "kis_simple_update_queue.h"

#include <QMutexLocker>
#include <QVector>

#include "kis_image_config.h"
#include "kis_full_refresh_walker.h"
#include "kis_spontaneous_job.h"


//#define ENABLE_DEBUG_JOIN
//#define ENABLE_ACCUMULATOR

#ifdef ENABLE_DEBUG_JOIN
    #define DEBUG_JOIN(baseRect, newRect, alpha)                     \
        dbgKrita << "Two rects were joined:\t"                       \
                 << (baseRect) << "+" << (newRect) << "->"           \
                 << ((baseRect) | (newRect)) << "(" << alpha << ")"

#else
    #define DEBUG_JOIN(baseRect, newRect, alpha)
#endif /* ENABLE_DEBUG_JOIN */


#ifdef ENABLE_ACCUMULATOR
    #define DECLARE_ACCUMULATOR() static qreal _baseAmount=0, _newAmount=0
    #define ACCUMULATOR_ADD(baseAmount, newAmount) \
        do {_baseAmount += baseAmount; _newAmount += newAmount;} while (0)
    #define ACCUMULATOR_DEBUG() \
        dbgKrita << "Accumulated alpha:" << _newAmount / _baseAmount
#else
    #define DECLARE_ACCUMULATOR()
    #define ACCUMULATOR_ADD(baseAmount, newAmount)
    #define ACCUMULATOR_DEBUG()
#endif /* ENABLE_ACCUMULATOR */


KisSimpleUpdateQueue::KisSimpleUpdateQueue()
    : m_overrideLevelOfDetail(-1)
{
    updateSettings();
}

KisSimpleUpdateQueue::~KisSimpleUpdateQueue()
{
//    QMutexLocker locker(&m_mutex);

    while (!m_spontaneousJobsList.isEmpty()) {
        delete m_spontaneousJobsList.takeLast();
    }
}

void KisSimpleUpdateQueue::updateSettings()
{
//    QMutexLocker locker(&m_mutex);

    KisImageConfig config(true);

    m_patchWidth = config.updatePatchWidth();
    m_patchHeight = config.updatePatchHeight();

    m_maxCollectAlpha = config.maxCollectAlpha();
    m_maxMergeAlpha = config.maxMergeAlpha();
    m_maxMergeCollectAlpha = config.maxMergeCollectAlpha();
}

int KisSimpleUpdateQueue::overrideLevelOfDetail() const
{
    return m_overrideLevelOfDetail;
}

void KisSimpleUpdateQueue::processQueue(KisUpdaterContext &updaterContext)
{
//    QMutexLocker locker(&m_mutex);
    while (updaterContext.hasSpareThread() && processOneJob(updaterContext));
}

bool KisSimpleUpdateQueue::processOneJob(KisUpdaterContext &updaterContext)
{
    KisBaseRectsWalkerSP item;
    KisMutableWalkersListIterator iter(m_updatesList, true);
    bool jobAdded = false;

    int currentLevelOfDetail = updaterContext.currentLevelOfDetail();

    while(iter.hasNext()) {
        item = iter.next();

        if ((currentLevelOfDetail < 0 || currentLevelOfDetail == item->levelOfDetail()) &&
            !item->checksumValid()) {

            m_overrideLevelOfDetail = item->levelOfDetail();
            item->recalculate(item->requestedRect());
            m_overrideLevelOfDetail = -1;
        }

        if ((currentLevelOfDetail < 0 || currentLevelOfDetail == item->levelOfDetail()) &&
            updaterContext.isJobAllowed(item)) {
            jobAdded = updaterContext.addMergeJob(item);

            if (!jobAdded) continue;

            iter.remove(item);
            break;
        }
    }
    iter.unlock();

    if (jobAdded) return true;

    if (!m_spontaneousJobsList.isEmpty()) {
        /**
         * WARNING: Please note that this still doesn't guarantee that
         * the spontaneous jobs are exclusive, since updates and/or
         * strokes can be added after them. The only thing it
         * guarantees that two spontaneous jobs will not be executed
         * in parallel.
         *
         * Right now it works as it is. Probably will need to be fixed
         * in the future.
         */
        qint32 numMergeJobs;
        qint32 numStrokeJobs;
        updaterContext.getJobsSnapshot(numMergeJobs, numStrokeJobs);

        if (!numMergeJobs && !numStrokeJobs) {
            KisSpontaneousJob *job = m_spontaneousJobsList.first();
            jobAdded = updaterContext.addSpontaneousJob(job);

            if (jobAdded) m_spontaneousJobsList.takeFirst();
        }
    }

    return jobAdded;
}

void KisSimpleUpdateQueue::addUpdateJob(KisNodeSP node, const QVector<QRect> &rects, const QRect& cropRect, int levelOfDetail)
{
    addJob(node, rects, cropRect, levelOfDetail, KisBaseRectsWalker::UPDATE);
}

void KisSimpleUpdateQueue::addUpdateJob(KisNodeSP node, const QRect &rc, const QRect& cropRect, int levelOfDetail)
{
    addJob(node, {rc}, cropRect, levelOfDetail, KisBaseRectsWalker::UPDATE);
}


void KisSimpleUpdateQueue::addUpdateNoFilthyJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail)
{
    addJob(node, {rc}, cropRect, levelOfDetail, KisBaseRectsWalker::UPDATE_NO_FILTHY);
}

void KisSimpleUpdateQueue::addFullRefreshJob(KisNodeSP node, const QRect& rc, const QRect& cropRect, int levelOfDetail)
{
    addJob(node, {rc}, cropRect, levelOfDetail, KisBaseRectsWalker::FULL_REFRESH);
}

void KisSimpleUpdateQueue::addJob(KisNodeSP node, const QVector<QRect> &rects,
                                  const QRect& cropRect,
                                  int levelOfDetail,
                                  KisBaseRectsWalker::UpdateType type)
{
    QList<KisBaseRectsWalkerSP> walkers;

    Q_FOREACH (const QRect &rc, rects) {
        if (rc.isEmpty()) continue;

        KisBaseRectsWalkerSP walker;

        if(trySplitJob(node, rc, cropRect, levelOfDetail, type)) continue;
        if(tryMergeJob(node, rc, cropRect, levelOfDetail, type)) continue;

        if (type == KisBaseRectsWalker::UPDATE) {
            walker = new KisMergeWalker(cropRect, KisMergeWalker::DEFAULT);
        }
        else if (type == KisBaseRectsWalker::FULL_REFRESH)  {
            walker = new KisFullRefreshWalker(cropRect);
        }
        else if (type == KisBaseRectsWalker::UPDATE_NO_FILTHY) {
            walker = new KisMergeWalker(cropRect, KisMergeWalker::NO_FILTHY);
        }
        /* else if(type == KisBaseRectsWalker::UNSUPPORTED) fatalKrita; */

        walker->collectRects(node, rc);
        walkers.append(walker);
    }

    if (!walkers.isEmpty()) {
//        m_mutex.lock();
        m_updatesList.append(walkers);
//        m_mutex.unlock();
    }
}

void KisSimpleUpdateQueue::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
//    QMutexLocker locker(&m_mutex);

    KisSpontaneousJob *item;
    KisMutableSpontaneousJobsListIterator iter(m_spontaneousJobsList, true);

    iter.toBack();

    while(iter.hasPrevious()) {
        item = iter.previous();

        if (spontaneousJob->overrides(item)) {
            if (iter.remove(item)) delete item;
        }
    }

    iter.unlock();
    m_spontaneousJobsList.append(spontaneousJob);
}

bool KisSimpleUpdateQueue::isEmpty() const
{
    return m_updatesList.isEmpty() && m_spontaneousJobsList.isEmpty();
}

qint32 KisSimpleUpdateQueue::sizeMetric() const
{
    return m_updatesList.size() + m_spontaneousJobsList.size();
}

bool KisSimpleUpdateQueue::trySplitJob(KisNodeSP node, const QRect& rc,
                                       const QRect& cropRect,
                                       int levelOfDetail,
                                       KisBaseRectsWalker::UpdateType type)
{
    if(rc.width() <= m_patchWidth || rc.height() <= m_patchHeight)
        return false;

    // a bit of recursive splitting...

    qint32 firstCol = rc.x() / m_patchWidth;
    qint32 firstRow = rc.y() / m_patchHeight;

    qint32 lastCol = (rc.x() + rc.width()) / m_patchWidth;
    qint32 lastRow = (rc.y() + rc.height()) / m_patchHeight;

    QVector<QRect> splitRects;

    for(qint32 i = firstRow; i <= lastRow; i++) {
        for(qint32 j = firstCol; j <= lastCol; j++) {
            QRect maxPatchRect(j * m_patchWidth, i * m_patchHeight,
                               m_patchWidth, m_patchHeight);
            QRect patchRect = rc & maxPatchRect;
            splitRects.append(patchRect);
        }
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(!splitRects.isEmpty());
    addJob(node, splitRects, cropRect, levelOfDetail, type);

    return true;
}

bool KisSimpleUpdateQueue::tryMergeJob(KisNodeSP node, const QRect& rc,
                                       const QRect& cropRect,
                                       int levelOfDetail,
                                       KisBaseRectsWalker::UpdateType type)
{
//    QMutexLocker locker(&m_mutex);

    QRect baseRect = rc;

    KisBaseRectsWalkerSP goodCandidate;
    KisBaseRectsWalkerSP item;
    KisWalkersListIterator iter(m_updatesList);

    /**
     * We add new jobs to the tail of the list,
     * so it's more probable to find a good candidate here.
     */

    iter.toBack();

    while(iter.hasPrevious()) {
        item = iter.previous();

        if(item->startNode() != node) continue;
        if(item->type() != type) continue;
        if(item->cropRect() != cropRect) continue;
        if(item->levelOfDetail() != levelOfDetail) continue;

        if(joinRects(baseRect, item->requestedRect(), m_maxMergeAlpha)) {
            goodCandidate = item;
            break;
        }
    }
    iter.unlock();

    if(goodCandidate)
        collectJobs(goodCandidate, baseRect, m_maxMergeCollectAlpha);

    return (bool)goodCandidate;
}

void KisSimpleUpdateQueue::optimize()
{
//    QMutexLocker locker(&m_mutex);

    if(m_updatesList.size() <= 1) {
        return;
    }

    KisBaseRectsWalkerSP baseWalker = m_updatesList.first();
    QRect baseRect = baseWalker->requestedRect();

    collectJobs(baseWalker, baseRect, m_maxCollectAlpha);
}

void KisSimpleUpdateQueue::collectJobs(KisBaseRectsWalkerSP &baseWalker,
                                       QRect baseRect,
                                       const qreal maxAlpha)
{
    KisBaseRectsWalkerSP item;
    KisMutableWalkersListIterator iter(m_updatesList, true);

    while(iter.hasNext()) {
        item = iter.next();

        if(item == baseWalker) continue;
        if(item->type() != baseWalker->type()) continue;
        if(item->startNode() != baseWalker->startNode()) continue;
        if(item->cropRect() != baseWalker->cropRect()) continue;
        if(item->levelOfDetail() != baseWalker->levelOfDetail()) continue;

        if(joinRects(baseRect, item->requestedRect(), maxAlpha)) {
            iter.remove(item);
        }
    }
    iter.unlock();

    if(baseWalker->requestedRect() != baseRect) {
        baseWalker->collectRects(baseWalker->startNode(), baseRect);
    }
}

bool KisSimpleUpdateQueue::joinRects(QRect& baseRect,
                                     const QRect& newRect, qreal maxAlpha)
{
    QRect unitedRect = baseRect | newRect;
    if(unitedRect.width() > m_patchWidth || unitedRect.height() > m_patchHeight)
        return false;

    bool result = false;
    qint64 baseWork = baseRect.width() * baseRect.height() +
        newRect.width() * newRect.height();

    qint64 newWork = unitedRect.width() * unitedRect.height();

    qreal alpha = qreal(newWork) / baseWork;

    if(alpha < maxAlpha) {
        DEBUG_JOIN(baseRect, newRect, alpha);

        DECLARE_ACCUMULATOR();
        ACCUMULATOR_ADD(baseWork, newWork);
        ACCUMULATOR_DEBUG();

        baseRect = unitedRect;
        result = true;
    }

    return result;
}

KisWalkersList& KisTestableSimpleUpdateQueue::getWalkersList()
{
    return m_updatesList;
}

KisSpontaneousJobsList& KisTestableSimpleUpdateQueue::getSpontaneousJobsList()
{
    return m_spontaneousJobsList;
}
