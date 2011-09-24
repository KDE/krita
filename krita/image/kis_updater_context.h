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

#ifndef __KIS_UPDATER_CONTEXT_H
#define __KIS_UPDATER_CONTEXT_H

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>
#include <QThreadPool>

#include "kis_stroke_job.h"
#include "kis_base_rects_walker.h"
#include "kis_async_merger.h"

class KisUpdateJobItem;

class KRITAIMAGE_EXPORT KisUpdaterContext : public QObject
{
    Q_OBJECT

public:
    KisUpdaterContext(qint32 threadCount = -1);
    virtual ~KisUpdaterContext();


    /**
     * Returns the number of currently running jobs of each type.
     * To use this information you should lock the context beforehand.
     *
     * \see lock()
     */
    void getJobsSnapshot(qint32 &numMergeJobs, qint32 &numStrokeJobs);

    /**
     * Check whether there is a spare thread for running
     * one more job
     */
    bool hasSpareThread();

    /**
     * Checks whether the walker intersects with any
     * of currently executing walkers. If it does,
     * it is not allowed to go in. It should be called
     * with the lock held.
     *
     * \see lock()
     */
    bool isJobAllowed(KisBaseRectsWalkerSP walker);

    /**
     * Registers the job and starts executing it.
     * The caller must ensure that the context is locked
     * with lock(), job is allowed with isWalkerAllowed() and
     * there is a spare thread for running it with hasSpareThread()
     *
     * \see lock()
     * \see isWalkerAllowed()
     * \see hasSpareThread()
     */
    virtual void addMergeJob(KisBaseRectsWalkerSP walker);

    /**
     * Adds a stroke job to the context. The prerequisites are
     * the same as for addMergeJob()
     * \see addMergeJob()
     */
    virtual void addStrokeJob(KisStrokeJob *strokeJob);

    /**
     * Block execution of the caller until all the jobs are finished
     */
    void waitForDone();

    /**
     * Locks the context to guarantee an exclusive access
     * to the context
     */
    void lock();

    /**
     * Unlocks the context
     *
     * \see lock()
     */
    void unlock();

signals:
    void sigContinueUpdate(const QRect& rc);
    void sigDoSomeUsefulWork();
    void sigSpareThreadAppeared();

protected slots:
    void slotJobFinished();

protected:
    static bool walkerIntersectsJob(KisBaseRectsWalkerSP walker,
                                    const KisUpdateJobItem* job);
    qint32 findSpareThread();

protected:
    /**
     * The lock is shared by all the child update job items.
     * When an item wants to run a usual (non-exclusive) job,
     * it locks the lock for read access. When an exclusive
     * access is requested, it locks it for write
     */
    QReadWriteLock m_exclusiveJobLock;

    QMutex m_lock;
    QVector<KisUpdateJobItem*> m_jobs;
    QThreadPool m_threadPool;
};

class KRITAIMAGE_EXPORT KisTestableUpdaterContext : public KisUpdaterContext
{
public:
    /**
     * Creates an explicit number of threads
     */
    KisTestableUpdaterContext(qint32 threadCount);
    ~KisTestableUpdaterContext();

    /**
     * The only difference - it doesn't start execution
     * of the job
     */
    void addMergeJob(KisBaseRectsWalkerSP walker);
    void addStrokeJob(KisStrokeJob *strokeJob);

    const QVector<KisUpdateJobItem*> getJobs();
    void clear();
};


#endif /* __KIS_UPDATER_CONTEXT_H */

