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

#include "kis_updater_context.h"

#include <QThread>
#include <QThreadPool>

#include "kis_update_job_item.h"
#include "kis_stroke_job.h"

const int KisUpdaterContext::useIdealThreadCountTag = -1;

KisUpdaterContext::KisUpdaterContext(qint32 threadCount, QObject *parent)
    : QObject(parent), m_scheduler(qobject_cast<KisUpdateScheduler *>(parent))
{
    if(threadCount <= 0) {
        threadCount = QThread::idealThreadCount();
        threadCount = threadCount > 0 ? threadCount : 1;
    }

    setThreadsLimit(threadCount);
}

KisUpdaterContext::~KisUpdaterContext()
{
    m_threadPool.waitForDone();
    for(qint32 i = 0; i < m_jobs.size(); i++)
        delete m_jobs[i];
}

void KisUpdaterContext::getJobsSnapshot(qint32 &numMergeJobs,
                                        qint32 &numStrokeJobs)
{
    numMergeJobs = 0;
    numStrokeJobs = 0;

    Q_FOREACH (const KisUpdateJobItem *item, m_jobs) {
        if(item->type() == KisUpdateJobItem::Type::MERGE ||
           item->type() == KisUpdateJobItem::Type::SPONTANEOUS) {
            numMergeJobs++;
        }
        else if(item->type() == KisUpdateJobItem::Type::STROKE) {
            numStrokeJobs++;
        }
    }
}

KisUpdaterContextSnapshotEx KisUpdaterContext::getContextSnapshotEx() const
{
    KisUpdaterContextSnapshotEx state = ContextEmpty;

    Q_FOREACH (const KisUpdateJobItem *item, m_jobs) {
        if (item->type() == KisUpdateJobItem::Type::MERGE ||
            item->type() == KisUpdateJobItem::Type::SPONTANEOUS) {
            state |= HasMergeJob;
        } else if(item->type() == KisUpdateJobItem::Type::STROKE) {
            switch (item->strokeJobSequentiality()) {
            case KisStrokeJobData::SEQUENTIAL:
                state |= HasSequentialJob;
                break;
            case KisStrokeJobData::CONCURRENT:
                state |= HasConcurrentJob;
                break;
            case KisStrokeJobData::BARRIER:
                state |= HasBarrierJob;
                break;
            case KisStrokeJobData::UNIQUELY_CONCURRENT:
                state |= HasUniquelyConcurrentJob;
                break;
            }
        }
    }

    return state;
}

int KisUpdaterContext::currentLevelOfDetail() const
{
    return m_lodCounter.readLod();
}

bool KisUpdaterContext::hasSpareThread()
{
    bool found = false;

    Q_FOREACH (const KisUpdateJobItem *item, m_jobs) {
        if(!item->isRunning()) {
            found = true;
            break;
        }
    }
    return found;
}

bool KisUpdaterContext::isJobAllowed(KisBaseRectsWalkerSP walker)
{
    int lod = this->currentLevelOfDetail();
    if (lod >= 0 && walker->levelOfDetail() != lod) return false;

    bool intersects = false;

    Q_FOREACH (const KisUpdateJobItem *item, m_jobs) {
        if(item->isRunning() && walkerIntersectsJob(walker, item)) {
            intersects = true;
            break;
        }
    }

    return !intersects;
}

/**
 * NOTE: In theory, isJobAllowed() and addMergeJob() should be merged into
 * one atomic method like `bool push()`, because this implementation
 * of KisUpdaterContext will not work in case of multiple
 * producers. But currently we have only one producer (one thread
 * in a time), that is guaranteed by the lock()/unlock() pair in
 * KisAbstractUpdateQueue::processQueue.
 */
void KisUpdaterContext::addMergeJob(KisBaseRectsWalkerSP walker)
{
    m_lodCounter.addLod(walker->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setWalker(walker);

    // it might happen that we call this function from within
    // the thread itself, right when it finished its work
    if (shouldStartThread) {
        m_threadPool.start(m_jobs[jobIndex]);
    }
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addMergeJob(KisBaseRectsWalkerSP walker)
{
    m_lodCounter.addLod(walker->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setWalker(walker);

    // HINT: Not calling start() here
    Q_UNUSED(shouldStartThread);
}

void KisUpdaterContext::addStrokeJob(KisStrokeJob *strokeJob)
{
    m_lodCounter.addLod(strokeJob->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setStrokeJob(strokeJob);

    // it might happen that we call this function from within
    // the thread itself, right when it finished its work
    if (shouldStartThread) {
        m_threadPool.start(m_jobs[jobIndex]);
    }
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addStrokeJob(KisStrokeJob *strokeJob)
{
    m_lodCounter.addLod(strokeJob->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setStrokeJob(strokeJob);

    // HINT: Not calling start() here
    Q_UNUSED(shouldStartThread);
}

void KisUpdaterContext::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    m_lodCounter.addLod(spontaneousJob->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setSpontaneousJob(spontaneousJob);

    // it might happen that we call this function from within
    // the thread itself, right when it finished its work
    if (shouldStartThread) {
        m_threadPool.start(m_jobs[jobIndex]);
    }
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    m_lodCounter.addLod(spontaneousJob->levelOfDetail());
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    const bool shouldStartThread = m_jobs[jobIndex]->setSpontaneousJob(spontaneousJob);

    // HINT: Not calling start() here
    Q_UNUSED(shouldStartThread);
}

void KisUpdaterContext::waitForDone()
{
    m_threadPool.waitForDone();
}

bool KisUpdaterContext::walkerIntersectsJob(KisBaseRectsWalkerSP walker,
                                            const KisUpdateJobItem* job)
{
    return (walker->accessRect().intersects(job->changeRect())) ||
        (job->accessRect().intersects(walker->changeRect()));
}

qint32 KisUpdaterContext::findSpareThread()
{
    for(qint32 i=0; i < m_jobs.size(); i++)
        if(!m_jobs[i]->isRunning())
            return i;

    return -1;
}

void KisUpdaterContext::lock()
{
    m_lock.lock();
}

void KisUpdaterContext::unlock()
{
    m_lock.unlock();
}

void KisUpdaterContext::setThreadsLimit(int value)
{
    m_threadPool.setMaxThreadCount(value);

    for (int i = 0; i < m_jobs.size(); i++) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(!m_jobs[i]->isRunning());
        // don't delete the jobs until all of them are checked!
    }

    for (int i = 0; i < m_jobs.size(); i++) {
        delete m_jobs[i];
    }

    m_jobs.resize(value);

    for(qint32 i = 0; i < m_jobs.size(); i++) {
        m_jobs[i] = new KisUpdateJobItem(this);
    }
}

int KisUpdaterContext::threadsLimit() const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_jobs.size() == m_threadPool.maxThreadCount());
    return m_jobs.size();
}

void KisUpdaterContext::continueUpdate(const QRect& rc)
{
    if (m_scheduler) m_scheduler->continueUpdate(rc);
}

void KisUpdaterContext::doSomeUsefulWork()
{
    if (m_scheduler) m_scheduler->doSomeUsefulWork();
}

void KisUpdaterContext::jobFinished()
{
    m_lodCounter.removeLod();
    if (m_scheduler) m_scheduler->spareThreadAppeared();
}

KisTestableUpdaterContext::KisTestableUpdaterContext(qint32 threadCount)
    : KisUpdaterContext(threadCount)
{
}

KisTestableUpdaterContext::~KisTestableUpdaterContext() {
    clear();
}

const QVector<KisUpdateJobItem*> KisTestableUpdaterContext::getJobs()
{
    return m_jobs;
}

void KisTestableUpdaterContext::clear()
{
    Q_FOREACH (KisUpdateJobItem *item, m_jobs) {
        item->testingSetDone();
    }

    m_lodCounter.testingClear();
}

