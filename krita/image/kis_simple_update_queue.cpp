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

#include "kis_image_config.h"
#include "kis_full_refresh_walker.h"


//#define ENABLE_DEBUG_JOIN
//#define ENABLE_ACCUMULATOR

#ifdef ENABLE_DEBUG_JOIN
    #define DEBUG_JOIN(baseRect, newRect, alpha)                     \
        qDebug() << "Two rects were joined:\t"                       \
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
        qDebug() << "Accumulated alpha:" << _newAmount / _baseAmount
#else
    #define DECLARE_ACCUMULATOR()
    #define ACCUMULATOR_ADD(baseAmount, newAmount)
    #define ACCUMULATOR_DEBUG()
#endif /* ENABLE_ACCUMULATOR */


KisSimpleUpdateQueue::KisSimpleUpdateQueue()
{
    updateSettings();
}

KisSimpleUpdateQueue::~KisSimpleUpdateQueue()
{
}

void KisSimpleUpdateQueue::updateSettings()
{
    KisImageConfig config;

    m_patchWidth = config.updatePatchWidth();
    m_patchHeight = config.updatePatchHeight();

    m_maxCollectAlpha = config.maxCollectAlpha();
    m_maxMergeAlpha = config.maxMergeAlpha();
    m_maxMergeCollectAlpha = config.maxMergeCollectAlpha();
}

void KisSimpleUpdateQueue::processQueue(KisUpdaterContext &updaterContext)
{
    updaterContext.lock();

    while(updaterContext.hasSpareThread() &&
          processOneJob(updaterContext));

    updaterContext.unlock();
}

bool KisSimpleUpdateQueue::processOneJob(KisUpdaterContext &updaterContext)
{
    QMutexLocker locker(&m_lock);

    KisBaseRectsWalkerSP item;
    KisMutableWalkersListIterator iter(m_list);
    bool jobAdded = false;

    while(iter.hasNext()) {
        item = iter.next();

        if(!item->checksumValid())
            item->recalculate(item->requestedRect());

        if(updaterContext.isJobAllowed(item)) {
            updaterContext.addMergeJob(item);
            iter.remove();
            jobAdded = true;
            break;
        }
    }

    return jobAdded;
}

void KisSimpleUpdateQueue::addUpdateJob(KisNodeSP node, const QRect& rc, const QRect& cropRect)
{
    addJob(node, rc, cropRect, KisBaseRectsWalker::UPDATE);
}

void KisSimpleUpdateQueue::addFullRefreshJob(KisNodeSP node, const QRect& rc, const QRect& cropRect)
{
    addJob(node, rc, cropRect, KisBaseRectsWalker::FULL_REFRESH);
}

void KisSimpleUpdateQueue::addJob(KisNodeSP node, const QRect& rc,
                                  const QRect& cropRect,
                                  KisBaseRectsWalker::UpdateType type)
{
    if(trySplitJob(node, rc, cropRect, type)) return;
    if(tryMergeJob(node, rc, cropRect, type)) return;

    KisBaseRectsWalkerSP walker;

    if(type == KisBaseRectsWalker::UPDATE) {
        walker = new KisMergeWalker(cropRect);
    }
    else /* if(type == KisBaseRectsWalker::FULL_REFRESH) */ {
        walker = new KisFullRefreshWalker(cropRect);
    }
    /* else if(type == KisBaseRectsWalker::UNSUPPORTED) qFatal(); */

    walker->collectRects(node, rc);

    m_lock.lock();
    m_list.append(walker);
    m_lock.unlock();
}

bool KisSimpleUpdateQueue::isEmpty() const
{
    QMutexLocker locker(&m_lock);
    return m_list.isEmpty();
}

qint32 KisSimpleUpdateQueue::sizeMetric() const
{
    QMutexLocker locker(&m_lock);
    return m_list.size();
}

bool KisSimpleUpdateQueue::trySplitJob(KisNodeSP node, const QRect& rc,
                                       const QRect& cropRect,
                                       KisBaseRectsWalker::UpdateType type)
{
    if(rc.width() <= m_patchWidth || rc.height() <= m_patchHeight)
        return false;

    // a bit of recursive splitting...

    qint32 firstCol = rc.x() / m_patchWidth;
    qint32 firstRow = rc.y() / m_patchHeight;

    qint32 lastCol = (rc.x() + rc.width()) / m_patchWidth;
    qint32 lastRow = (rc.y() + rc.height()) / m_patchHeight;

    for(qint32 i = firstRow; i <= lastRow; i++) {
        for(qint32 j = firstCol; j <= lastCol; j++) {
            QRect maxPatchRect(j * m_patchWidth, i * m_patchHeight,
                               m_patchWidth, m_patchHeight);
            QRect patchRect = rc & maxPatchRect;
            addJob(node, patchRect, cropRect, type);
        }
    }
    return true;
}

bool KisSimpleUpdateQueue::tryMergeJob(KisNodeSP node, const QRect& rc,
                                       const QRect& cropRect,
                                       KisBaseRectsWalker::UpdateType type)
{
    QMutexLocker locker(&m_lock);

    QRect baseRect = rc;

    KisBaseRectsWalkerSP goodCandidate;
    KisBaseRectsWalkerSP item;
    KisWalkersListIterator iter(m_list);

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

        if(joinRects(baseRect, item->requestedRect(), m_maxMergeAlpha)) {
            goodCandidate = item;
            break;
        }
    }

    if(goodCandidate)
        collectJobs(goodCandidate, baseRect, node, m_maxMergeCollectAlpha);

    return (bool)goodCandidate;
}

void KisSimpleUpdateQueue::optimize()
{
    QMutexLocker locker(&m_lock);

    if(m_list.size() <= 1) return;

    KisBaseRectsWalkerSP baseWalker = m_list.first();
    QRect baseRect = baseWalker->requestedRect();
    KisNodeSP baseNode = baseWalker->startNode();

    collectJobs(baseWalker, baseRect, baseNode, m_maxCollectAlpha);
}

void KisSimpleUpdateQueue::collectJobs(KisBaseRectsWalkerSP &baseWalker,
                                       QRect baseRect,
                                       const KisNodeSP &baseNode,
                                       const qreal maxAlpha)
{
    KisBaseRectsWalkerSP item;
    KisMutableWalkersListIterator iter(m_list);

    while(iter.hasNext()) {
        item = iter.next();

        if(item == baseWalker) continue;
        if(item->type() != baseWalker->type()) continue;
        if(item->startNode() != baseNode) continue;
        if(item->cropRect() != baseWalker->cropRect()) continue;

        if(joinRects(baseRect, item->requestedRect(), maxAlpha)) {
            iter.remove();
        }
    }

    if(baseWalker->requestedRect() != baseRect) {
        baseWalker->collectRects(baseNode, baseRect);
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
    return m_list;
}

