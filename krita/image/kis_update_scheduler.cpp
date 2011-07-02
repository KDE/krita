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

#include "kis_merge_walker.h"
#include "kis_full_refresh_walker.h"
#include "kis_updater_context.h"
#include "kis_simple_update_queue.h"


struct KisUpdateScheduler::Private {
    Private() : workQueue(0), processingBlocked(false) {}

    KisSimpleUpdateQueue *workQueue;
    KisUpdaterContext *updaterContext;
    bool processingBlocked;
};

KisUpdateScheduler::KisUpdateScheduler(KisImageWSP image)
    : m_d(new Private)
{
    updateSettings();

    // The queue will update settings in a constructor itself
    m_d->workQueue = new KisSimpleUpdateQueue();
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
    m_d->workQueue->addJob(node, rc, cropRect);
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

void KisUpdateScheduler::updateSettings()
{
    if(m_d->workQueue) {
        m_d->workQueue->updateSettings();
    }
}

void KisUpdateScheduler::lock()
{
    m_d->processingBlocked = true;
    waitForDone();
}

void KisUpdateScheduler::unlock()
{
    m_d->processingBlocked = false;
    processQueues();
}

void KisUpdateScheduler::waitForDone()
{
    m_d->updaterContext->waitForDone();
}

void KisUpdateScheduler::processQueues()
{
    if(m_d->processingBlocked) return;

    m_d->workQueue->processQueue(*m_d->updaterContext);
}

void KisUpdateScheduler::doSomeUsefulWork()
{
    m_d->workQueue->optimize();
}

void KisUpdateScheduler::spareThreadAppeared()
{
    processQueues();
}

KisTestableUpdateScheduler::KisTestableUpdateScheduler(KisImageWSP image, qint32 threadCount)
{
    updateSettings();

    // The queue will update settings in a constructor itself
    m_d->workQueue = new KisTestableSimpleUpdateQueue();
    m_d->updaterContext = new KisTestableUpdaterContext(threadCount);

    connectImage(image);
}

KisTestableUpdaterContext* KisTestableUpdateScheduler::updaterContext()
{
    return dynamic_cast<KisTestableUpdaterContext*>(m_d->updaterContext);
}

KisTestableSimpleUpdateQueue* KisTestableUpdateScheduler::updateQueue()
{
    return dynamic_cast<KisTestableSimpleUpdateQueue*>(m_d->workQueue);
}
