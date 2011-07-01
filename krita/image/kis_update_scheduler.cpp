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
#include "kis_simple_update_queue.h"

struct KisUpdateScheduler::Private {
    Private() : workQueue(0) {}

    KisAbstractUpdateQueue* workQueue;
    KisUpdaterContext updaterContext;
};

KisUpdateScheduler::KisUpdateScheduler(KisImageWSP image)
    : m_d(new Private)
{
    updateSettings();

    // The queue will update settings in a constructor itself
    m_d->workQueue = new KisSimpleUpdateQueue();

    connect(&m_d->updaterContext, SIGNAL(sigContinueUpdate(const QRect&)),
            image, SLOT(slotProjectionUpdated(const QRect&)),
            Qt::DirectConnection);

    connect(&m_d->updaterContext, SIGNAL(sigDoSomeUsefulWork()),
            SLOT(doSomeUsefulWork()), Qt::DirectConnection);

    connect(&m_d->updaterContext, SIGNAL(sigSpareThreadAppeared()),
            SLOT(spareThreadAppeared()), Qt::DirectConnection);

}

KisUpdateScheduler::~KisUpdateScheduler()
{
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

    m_d->workQueue->executeJobSync(walker, m_d->updaterContext);
}

void KisUpdateScheduler::updateSettings()
{
    if(m_d->workQueue) {
        m_d->workQueue->updateSettings();
    }
}

void KisUpdateScheduler::lock()
{
    m_d->workQueue->blockProcessing(m_d->updaterContext);
}

void KisUpdateScheduler::unlock()
{
    m_d->workQueue->startProcessing(m_d->updaterContext);
}

void KisUpdateScheduler::waitForDone()
{
    m_d->updaterContext.waitForDone();
    Q_ASSERT(m_d->workQueue->isEmpty());
}

void KisUpdateScheduler::processQueues()
{
    m_d->workQueue->processQueue(m_d->updaterContext);
}

void KisUpdateScheduler::doSomeUsefulWork()
{
    m_d->workQueue->optimize();
}

void KisUpdateScheduler::spareThreadAppeared()
{
    processQueues();
}

