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

#include "KisDabRenderingExecutor.h"

#include <QThreadPool>
#include <KisSharedThreadPoolAdapter.h>
#include "KisDabRenderingQueue.h"
#include "KisDabRenderingQueueCache.h"
#include "KisDabRenderingJob.h"
#include "KisRenderedDab.h"

struct KisDabRenderingExecutor::Private
{
    Private()
        : sharedThreadPool(
              new KisSharedThreadPoolAdapter(QThreadPool::globalInstance()))
    {
    }

    QScopedPointer<KisSharedThreadPoolAdapter> sharedThreadPool;
    QScopedPointer<KisDabRenderingQueue> renderingQueue;
};

KisDabRenderingExecutor::KisDabRenderingExecutor(const KoColorSpace *cs, KisDabCacheUtils::ResourcesFactory resourcesFactory)
    : m_d(new Private)
{
    m_d->renderingQueue.reset(
        new KisDabRenderingQueue(cs, resourcesFactory, m_d->sharedThreadPool.data()));
    m_d->renderingQueue->setCacheInterface(new KisDabRenderingQueueCache());
}

KisDabRenderingExecutor::~KisDabRenderingExecutor()
{
}

void KisDabRenderingExecutor::addDab(const KisDabCacheUtils::DabRequestInfo &request)
{
    KisDabRenderingJob *job = m_d->renderingQueue->addDab(request);
    if (job) {
        m_d->sharedThreadPool->start(job);
    }
}

QList<KisRenderedDab> KisDabRenderingExecutor::takeReadyDabs()
{
    return m_d->renderingQueue->takeReadyDabs();
}

bool KisDabRenderingExecutor::hasPreparedDabs() const
{
    return m_d->renderingQueue->hasPreparedDabs();
}

void KisDabRenderingExecutor::waitForDone()
{
    m_d->sharedThreadPool->waitForDone();
}

