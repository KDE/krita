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


KisUpdaterContext::KisUpdaterContext(qint32 threadCount)
{
    if(threadCount <= 0) {
        threadCount = QThread::idealThreadCount();
        threadCount = threadCount > 0 ? threadCount : 1;
    }

    m_jobs.resize(threadCount);
    for(qint32 i = 0; i < m_jobs.size(); i++) {
        m_jobs[i] = new KisUpdateJobItem(&m_exclusiveJobLock);
        connect(m_jobs[i], SIGNAL(sigContinueUpdate(const QRect&)),
                SIGNAL(sigContinueUpdate(const QRect&)),
                Qt::DirectConnection);

        connect(m_jobs[i], SIGNAL(sigDoSomeUsefulWork()),
                SIGNAL(sigDoSomeUsefulWork()), Qt::DirectConnection);

        connect(m_jobs[i], SIGNAL(sigJobFinished()),
                SLOT(slotJobFinished()), Qt::DirectConnection);
    }
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

    foreach(const KisUpdateJobItem *item, m_jobs) {
        if(item->type() == KisUpdateJobItem::MERGE ||
           item->type() == KisUpdateJobItem::SPONTANEOUS) {
            numMergeJobs++;
        }
        else if(item->type() == KisUpdateJobItem::STROKE) {
            numStrokeJobs++;
        }
    }
}

bool KisUpdaterContext::hasSpareThread()
{
    bool found = false;

    foreach(const KisUpdateJobItem *item, m_jobs) {
        if(!item->isRunning()) {
            found = true;
            break;
        }
    }
    return found;
}

bool KisUpdaterContext::isJobAllowed(KisBaseRectsWalkerSP walker)
{
    bool intersects = false;

    foreach(const KisUpdateJobItem *item, m_jobs) {
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
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setWalker(walker);
    m_threadPool.start(m_jobs[jobIndex]);
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addMergeJob(KisBaseRectsWalkerSP walker)
{
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setWalker(walker);
    // HINT: Not calling start() here
}

void KisUpdaterContext::addStrokeJob(KisStrokeJob *strokeJob)
{
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setStrokeJob(strokeJob);
    m_threadPool.start(m_jobs[jobIndex]);
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addStrokeJob(KisStrokeJob *strokeJob)
{
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setStrokeJob(strokeJob);
    // HINT: Not calling start() here
}

void KisUpdaterContext::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setSpontaneousJob(spontaneousJob);
    m_threadPool.start(m_jobs[jobIndex]);
}

/**
 * This variant is for use in a testing suite only
 */
void KisTestableUpdaterContext::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    qint32 jobIndex = findSpareThread();
    Q_ASSERT(jobIndex >= 0);

    m_jobs[jobIndex]->setSpontaneousJob(spontaneousJob);
    // HINT: Not calling start() here
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

void KisUpdaterContext::slotJobFinished()
{
    // Be careful. This slot can be called asynchronously without locks.
    emit sigSpareThreadAppeared();
}

void KisUpdaterContext::lock()
{
    m_lock.lock();
}

void KisUpdaterContext::unlock()
{
    m_lock.unlock();
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
    foreach(KisUpdateJobItem *item, m_jobs) {
        item->testingSetDone();
    }
}

