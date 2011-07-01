/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_stroke.h"

#include <QMutexLocker>
#include "kis_stroke_strategy.h"


KisStroke::KisStroke(KisStrokeStrategy *strokeStrategy)
    : m_strokeStrategy(strokeStrategy),
      m_strokeInitialized(false),
      m_strokeEnded(false)
{
    m_initStrategy = m_strokeStrategy->createInitStrategy();
    m_dabStrategy = m_strokeStrategy->createDabStrategy();
    m_cancelStrategy = m_strokeStrategy->createCancelStrategy();
    m_finishStrategy = m_strokeStrategy->createFinishStrategy();

    enqueue(new KisStrokeJob(m_initStrategy, 0));
}

KisStroke::~KisStroke()
{
    Q_ASSERT(m_strokeEnded);
    Q_ASSERT(m_jobsQueue.isEmpty());

    delete m_initStrategy;
    delete m_dabStrategy;
    delete m_cancelStrategy;
    delete m_finishStrategy;
    delete m_strokeStrategy;
}

void KisStroke::addJob(KisDabProcessingStrategy::DabProcessingData *data)
{
    Q_ASSERT(!m_strokeEnded);
    enqueue(new KisStrokeJob(m_dabStrategy, data));
}

KisStrokeJob* KisStroke::popOneJob()
{
    KisStrokeJob *job = dequeue();

    if(!m_strokeInitialized && job) {
        m_strokeInitialized = true;
    }

    return job;
}

bool KisStroke::hasJobs() const
{
    return !m_jobsQueue.isEmpty();
}

void KisStroke::endStroke()
{
    Q_ASSERT(!m_strokeEnded);
    m_strokeEnded = true;

    enqueue(new KisStrokeJob(m_finishStrategy, 0));
}

/**
 * About cancelling the stroke
 * There may be four different states of the stroke, when cancel
 * is requested:
 * 1) Not initialized, has jobs -- just clear the queue
 * 2) Initialized, has jobs, not finished -- clear the queue,
 *    enqueue the cancel job
 * 3) Initialized, has jobs, finished -- clear the queue, enqueue
 *    the cancel job
 * 4) Initialized, no jobs, finished -- it's too late to cancel
 *    anything
 */

void KisStroke::cancelStroke()
{
    QMutexLocker locker(&m_mutex);

    if(!m_strokeInitialized) {
        clearQueue();
    }
    else if(m_strokeInitialized && !m_jobsQueue.isEmpty()) {
        clearQueue();
        m_jobsQueue.enqueue(new KisStrokeJob(m_cancelStrategy, 0));
    }
    // else {
    //     too late ...
    // }

    m_strokeEnded = true;
}

void KisStroke::clearQueue()
{
    foreach(KisStrokeJob *item, m_jobsQueue) {
        delete item;
    }
    m_jobsQueue.clear();
}

bool KisStroke::isEnded() const
{
    return m_strokeEnded;
}

bool KisStroke::isSequential() const
{
    return m_strokeStrategy->isSequential();
}

bool KisStroke::isExclusive() const
{
    return m_strokeStrategy->isExclusive();
}

void KisStroke::enqueue(KisStrokeJob* job)
{
    // factory methods can return null, if no action is needed
    if(!job) return;

    QMutexLocker locker(&m_mutex);
    m_jobsQueue.enqueue(job);
}

KisStrokeJob* KisStroke::dequeue()
{
    QMutexLocker locker(&m_mutex);
    return !m_jobsQueue.isEmpty() ? m_jobsQueue.dequeue() : 0;
}
