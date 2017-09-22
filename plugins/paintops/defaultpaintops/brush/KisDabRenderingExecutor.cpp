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

    QScopedPointer<KisDabRenderingQueue> renderingQueue;
    QScopedPointer<KisSharedThreadPoolAdapter> sharedThreadPool;
};

KisDabRenderingExecutor::KisDabRenderingExecutor(const KoColorSpace *cs,
                                                 KisDabCacheUtils::ResourcesFactory resourcesFactory,
                                                 KisPressureMirrorOption *mirrorOption,
                                                 KisPrecisionOption *precisionOption)
    : m_d(new Private)
{
    m_d->renderingQueue.reset(
        new KisDabRenderingQueue(cs, resourcesFactory, m_d->sharedThreadPool.data()));

    KisDabRenderingQueueCache *cache = new KisDabRenderingQueueCache();
    cache->setMirrorPostprocessing(mirrorOption);
    cache->setPrecisionOption(precisionOption);

    m_d->renderingQueue->setCacheInterface(cache);
}

KisDabRenderingExecutor::~KisDabRenderingExecutor()
{
    // explicitly wait for the pool to end, because we might be mistaken about our
    // assumptions about object destruction order in ~Private()

    m_d->sharedThreadPool->waitForDone();
}

void KisDabRenderingExecutor::addDab(const KisDabCacheUtils::DabRequestInfo &request,
                                     qreal opacity, qreal flow)
{
    KisDabRenderingJob *job = m_d->renderingQueue->addDab(request, opacity, flow);
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

int KisDabRenderingExecutor::averageDabRenderingTime() const
{
    return m_d->renderingQueue->averageExecutionTime();
}

bool KisDabRenderingExecutor::dabsHaveSeparateOriginal() const
{
    return m_d->renderingQueue->dabsHaveSeparateOriginal();
}

void KisDabRenderingExecutor::waitForDone()
{
    m_d->sharedThreadPool->waitForDone();
}

