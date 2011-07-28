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

#include "kis_update_scheduler.h"

#include "kis_image_config.h"
#include "kis_merge_walker.h"
#include "kis_full_refresh_walker.h"
#include "kis_updater_context.h"
#include "kis_simple_update_queue.h"
#include "kis_strokes_queue.h"


struct KisUpdateScheduler::Private {
    Private() : updatesQueue(0), strokesQueue(0),
                updaterContext(0), processingBlocked(false),
                balancingRatio(1.0) {}

    KisSimpleUpdateQueue *updatesQueue;
    KisStrokesQueue *strokesQueue;
    KisUpdaterContext *updaterContext;
    bool processingBlocked;
    qreal balancingRatio; // updates-queue-size/strokes-queue-size
};

KisUpdateScheduler::KisUpdateScheduler(KisImageWSP image)
    : m_d(new Private)
{
    updateSettings();

    // The queue will update settings in a constructor itself
    m_d->updatesQueue = new KisSimpleUpdateQueue();
    m_d->strokesQueue = new KisStrokesQueue();
    m_d->updaterContext = new KisUpdaterContext();

    connectImage(image);
}

KisUpdateScheduler::KisUpdateScheduler()
    : m_d(new Private)
{
    updateSettings();
}

KisUpdateScheduler::~KisUpdateScheduler()
{
    delete m_d->updaterContext;
    delete m_d->updatesQueue;
    delete m_d->strokesQueue;
}

void KisUpdateScheduler::connectImage(KisImageWSP image)
{
    connect(m_d->updaterContext, SIGNAL(sigContinueUpdate(const QRect&)),
            image, SLOT(slotProjectionUpdated(const QRect&)),
            Qt::DirectConnection);

    connect(m_d->updaterContext, SIGNAL(sigDoSomeUsefulWork()),
            SLOT(doSomeUsefulWork()), Qt::DirectConnection);

    connect(m_d->updaterContext, SIGNAL(sigSpareThreadAppeared()),
            SLOT(spareThreadAppeared()), Qt::DirectConnection);
}

void KisUpdateScheduler::updateProjection(KisNodeSP node, const QRect& rc, const QRect &cropRect)
{
    m_d->updatesQueue->addUpdateJob(node, rc, cropRect);
    processQueues();
}

void KisUpdateScheduler::fullRefreshAsync(KisNodeSP root, const QRect& rc, const QRect &cropRect)
{
    m_d->updatesQueue->addFullRefreshJob(root, rc, cropRect);
    processQueues();
}

void KisUpdateScheduler::fullRefresh(KisNodeSP root, const QRect& rc, const QRect &cropRect)
{
    KisBaseRectsWalkerSP walker = new KisFullRefreshWalker(cropRect);
    walker->collectRects(root, rc);

    m_d->updaterContext->lock();
    lock();

    Q_ASSERT(m_d->updaterContext->isJobAllowed(walker));
    m_d->updaterContext->addMergeJob(walker);
    m_d->updaterContext->waitForDone();

    m_d->updaterContext->unlock();
    unlock();
}

KisStrokeId KisUpdateScheduler::startStroke(KisStrokeStrategy *strokeStrategy)
{
    KisStrokeId id  = m_d->strokesQueue->startStroke(strokeStrategy);
    processQueues();
    return id;
}

void KisUpdateScheduler::addJob(KisStrokeId id, KisStrokeJobData *data)
{
    m_d->strokesQueue->addJob(id, data);
    processQueues();
}

void KisUpdateScheduler::endStroke(KisStrokeId id)
{
    m_d->strokesQueue->endStroke(id);
    processQueues();
}

bool KisUpdateScheduler::cancelStroke(KisStrokeId id)
{
    bool result = m_d->strokesQueue->cancelStroke(id);
    processQueues();
    return result;
}

void KisUpdateScheduler::updateSettings()
{
    if(m_d->updatesQueue) {
        m_d->updatesQueue->updateSettings();
    }

    KisImageConfig config;
    m_d->balancingRatio = config.schedulerBalancingRatio();
}

void KisUpdateScheduler::lock()
{
    m_d->processingBlocked = true;
    m_d->updaterContext->waitForDone();
}

void KisUpdateScheduler::unlock()
{
    m_d->processingBlocked = false;
    processQueues();
}

void KisUpdateScheduler::waitForDone()
{
    while(!m_d->updatesQueue->isEmpty() || !m_d->strokesQueue->isEmpty()) {
        processQueues();
        m_d->updaterContext->waitForDone();
    }
}

void KisUpdateScheduler::barrierLock()
{
    do {
        processQueues();
        m_d->updaterContext->waitForDone();
        m_d->processingBlocked = true;
    } while(!m_d->updatesQueue->isEmpty() || !m_d->strokesQueue->isEmpty());
}

void KisUpdateScheduler::processQueues()
{
    if(m_d->processingBlocked) return;

    if(m_d->strokesQueue->needsExclusiveAccess()) {
        m_d->strokesQueue->processQueue(*m_d->updaterContext,
                                        !m_d->updatesQueue->isEmpty());

        if(!m_d->strokesQueue->needsExclusiveAccess()) {
            m_d->updatesQueue->processQueue(*m_d->updaterContext);
        }
    }
    else if(m_d->balancingRatio * m_d->strokesQueue->sizeMetric() > m_d->updatesQueue->sizeMetric()) {
        m_d->strokesQueue->processQueue(*m_d->updaterContext,
                                        !m_d->updatesQueue->isEmpty());
        m_d->updatesQueue->processQueue(*m_d->updaterContext);
    }
    else {
        m_d->updatesQueue->processQueue(*m_d->updaterContext);
        m_d->strokesQueue->processQueue(*m_d->updaterContext,
                                        !m_d->updatesQueue->isEmpty());

    }
}

void KisUpdateScheduler::doSomeUsefulWork()
{
    m_d->updatesQueue->optimize();
}

void KisUpdateScheduler::spareThreadAppeared()
{
    processQueues();
}

KisTestableUpdateScheduler::KisTestableUpdateScheduler(KisImageWSP image, qint32 threadCount)
{
    updateSettings();

    // The queue will update settings in a constructor itself
    m_d->updatesQueue = new KisTestableSimpleUpdateQueue();
    m_d->strokesQueue = new KisStrokesQueue();
    m_d->updaterContext = new KisTestableUpdaterContext(threadCount);

    connectImage(image);
}

KisTestableUpdaterContext* KisTestableUpdateScheduler::updaterContext()
{
    return dynamic_cast<KisTestableUpdaterContext*>(m_d->updaterContext);
}

KisTestableSimpleUpdateQueue* KisTestableUpdateScheduler::updateQueue()
{
    return dynamic_cast<KisTestableSimpleUpdateQueue*>(m_d->updatesQueue);
}
