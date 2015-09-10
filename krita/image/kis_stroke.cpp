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

#include "kis_stroke_strategy.h"


KisStroke::KisStroke(KisStrokeStrategy *strokeStrategy, Type type, int levelOfDetail)
    : m_strokeStrategy(strokeStrategy),
      m_strokeInitialized(false),
      m_strokeEnded(false),
      m_strokeSuspended(false),
      m_isCancelled(false),
      m_prevJobSequential(false),
      m_worksOnLevelOfDetail(levelOfDetail),
      m_type(type)
{
    m_initStrategy.reset(m_strokeStrategy->createInitStrategy());
    m_dabStrategy.reset(m_strokeStrategy->createDabStrategy());
    m_cancelStrategy.reset(m_strokeStrategy->createCancelStrategy());
    m_finishStrategy.reset(m_strokeStrategy->createFinishStrategy());
    m_suspendStrategy.reset(m_strokeStrategy->createSuspendStrategy());
    m_resumeStrategy.reset(m_strokeStrategy->createResumeStrategy());

    if(!m_initStrategy) {
        m_strokeInitialized = true;
    }
    else {
        enqueue(m_initStrategy.data(), m_strokeStrategy->createInitData());
    }
}

KisStroke::~KisStroke()
{
    Q_ASSERT(m_strokeEnded);
    Q_ASSERT(m_jobsQueue.isEmpty());
}

bool KisStroke::supportsSuspension()
{
    return !m_strokeInitialized || (m_suspendStrategy && m_resumeStrategy);
}

void KisStroke::suspendStroke(KisStrokeSP recipient)
{
    if (!m_strokeInitialized || m_strokeSuspended ||
        (m_strokeEnded && !hasJobs())) {

        return;
    }

    KIS_ASSERT_RECOVER_NOOP(m_suspendStrategy && m_resumeStrategy);

    prepend(m_resumeStrategy.data(),
            m_strokeStrategy->createResumeData(),
            worksOnLevelOfDetail(), false);

    recipient->prepend(m_suspendStrategy.data(),
                       m_strokeStrategy->createSuspendData(),
                       worksOnLevelOfDetail(), false);

    m_strokeSuspended = true;
}

void KisStroke::addJob(KisStrokeJobData *data)
{
    Q_ASSERT(!m_strokeEnded || m_isCancelled);
    enqueue(m_dabStrategy.data(), data);
}

KisStrokeJob* KisStroke::popOneJob()
{
    KisStrokeJob *job = dequeue();

    if(job) {
        m_prevJobSequential = job->isSequential();

        m_strokeInitialized = true;
        m_strokeSuspended = false;
    }

    return job;
}

KUndo2MagicString KisStroke::name() const
{
    return m_strokeStrategy->name();
}

bool KisStroke::hasJobs() const
{
    return !m_jobsQueue.isEmpty();
}

qint32 KisStroke::numJobs() const
{
    return m_jobsQueue.size();
}

void KisStroke::endStroke()
{
    Q_ASSERT(!m_strokeEnded);
    m_strokeEnded = true;

    enqueue(m_finishStrategy.data(), m_strokeStrategy->createFinishData());
}

/**
 * About cancelling the stroke
 * There may be four different states of the stroke, when cancel
 * is requested:
 * 1) Not initialized, has jobs -- just clear the queue
 * 2) Initialized, has jobs, not finished -- clear the queue,
 *    enqueue the cancel job
 * 5) Initialized, no jobs, not finished -- enqueue the cancel job
 * 3) Initialized, has jobs, finished -- clear the queue, enqueue
 *    the cancel job
 * 4) Initialized, no jobs, finished -- it's too late to cancel
 *    anything
 * 6) Initialized, has jobs, cancelled -- cancelling twice is a permitted
 *                                        operation, though it does nothing
 */

void KisStroke::cancelStroke()
{
    // case 6
    if (m_isCancelled) return;

    if(!m_strokeInitialized) {
        /**
         * FIXME: this assert is probably a bit too optimistic,
         *        because the LODN stroke that suspends the other one
         *        can be easily non-initialized
         */
        KIS_ASSERT_RECOVER_NOOP(sanityCheckAllJobsAreCancellable());
        clearQueueOnCancel();
    }
    else if(m_strokeInitialized &&
            (!m_jobsQueue.isEmpty() || !m_strokeEnded)) {

        clearQueueOnCancel();
        enqueue(m_cancelStrategy.data(),
                m_strokeStrategy->createCancelData());
    }
    // else {
    //     too late ...
    // }

    m_isCancelled = true;
    m_strokeEnded = true;
}

bool KisStroke::sanityCheckAllJobsAreCancellable() const
{
    foreach(KisStrokeJob *item, m_jobsQueue) {
        if (!item->isCancellable()) {
            return false;
        }
    }
    return true;
}

void KisStroke::clearQueueOnCancel()
{
    QQueue<KisStrokeJob*>::iterator it = m_jobsQueue.begin();
    QQueue<KisStrokeJob*>::iterator end = m_jobsQueue.end();

    while (it != end) {
        if ((*it)->isCancellable()) {
            delete (*it);
            it = m_jobsQueue.erase(it);
        } else {
            ++it;
        }
    }
}

bool KisStroke::isInitialized() const
{
    return m_strokeInitialized;
}

bool KisStroke::isEnded() const
{
    return m_strokeEnded;
}

bool KisStroke::isExclusive() const
{
    return m_strokeStrategy->isExclusive();
}

bool KisStroke::supportsWrapAroundMode() const
{
    return m_strokeStrategy->supportsWrapAroundMode();
}

int KisStroke::worksOnLevelOfDetail() const
{
    return m_worksOnLevelOfDetail;
}

bool KisStroke::prevJobSequential() const
{
    return m_prevJobSequential;
}

bool KisStroke::nextJobSequential() const
{
    return !m_jobsQueue.isEmpty() ?
        m_jobsQueue.head()->isSequential() : false;
}

bool KisStroke::nextJobBarrier() const
{
    return !m_jobsQueue.isEmpty() ?
        m_jobsQueue.head()->isBarrier() : false;
}

void KisStroke::enqueue(KisStrokeJobStrategy *strategy,
                        KisStrokeJobData *data)
{
    // factory methods can return null, if no action is needed
    if(!strategy) {
        delete data;
        return;
    }

    m_jobsQueue.enqueue(new KisStrokeJob(strategy, data, worksOnLevelOfDetail(), true));
}

void KisStroke::prepend(KisStrokeJobStrategy *strategy,
                        KisStrokeJobData *data,
                        int levelOfDetail,
                        bool isCancellable)
{
    // factory methods can return null, if no action is needed
    if(!strategy) {
        delete data;
        return;
    }

    // LOG_MERGE_FIXME:
    Q_UNUSED(levelOfDetail);

    m_jobsQueue.prepend(new KisStrokeJob(strategy, data, worksOnLevelOfDetail(), isCancellable));
}

KisStrokeJob* KisStroke::dequeue()
{
    return !m_jobsQueue.isEmpty() ? m_jobsQueue.dequeue() : 0;
}

void KisStroke::setLodBuddy(KisStrokeSP buddy)
{
    m_lodBuddy = buddy;
}

KisStrokeSP KisStroke::lodBuddy() const
{
    return m_lodBuddy;
}

KisStroke::Type KisStroke::type() const
{
    if (m_type == LOD0) {
        KIS_ASSERT_RECOVER_NOOP(m_lodBuddy && "LOD0 strokes must always have a buddy");
    } else if (m_type == LODN) {
        KIS_ASSERT_RECOVER_NOOP(m_worksOnLevelOfDetail > 0 && "LODN strokes must work on LOD > 0!");
    } else if (m_type == LEGACY) {
        KIS_ASSERT_RECOVER_NOOP(m_worksOnLevelOfDetail == 0 && "LEGACY strokes must work on LOD == 0!");
    }

    return m_type;
}
