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

#include "kis_base_rects_walker.h"
#include "kis_async_merger.h"
#include "kis_lock_free_lod_counter.h"

#include "KisUpdaterContextSnapshotEx.h"
<<<<<<< HEAD
#include "tiles3/kis_lockless_stack.h"
=======
>>>>>>> master
#include "kis_update_scheduler.h"

class KisUpdateJobItem;
class KisSpontaneousJob;
class KisStrokeJob;

class KRITAIMAGE_EXPORT KisUpdaterContext : public QObject
{
    Q_OBJECT
public:
    static const int useIdealThreadCountTag;

public:
    KisUpdaterContext(qint32 threadCount = useIdealThreadCountTag, QObject *parent = 0);
    ~KisUpdaterContext() override;


    /**
     * Returns the number of currently running jobs of each type.
     * To use this information you should lock the context beforehand.
     *
     * \see lock()
     */
    void getJobsSnapshot(qint32 &numMergeJobs, qint32 &numStrokeJobs);
    void getJobsSnapshotImpl(qint32 &numMergeJobs, qint32 &numStrokeJobs);

    KisUpdaterContextSnapshotEx getContextSnapshotEx() const;

    /**
     * Returns the current level of detail of all the running jobs in the
     * context. If there are no jobs, returns -1.
     */
    int currentLevelOfDetail() const;

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
    bool isJobAllowedImpl(KisBaseRectsWalkerSP walker);

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
    virtual bool addMergeJob(KisBaseRectsWalkerSP walker);

    /**
     * Adds a stroke job to the context. The prerequisites are
     * the same as for addMergeJob()
     * \see addMergeJob()
     */
    virtual bool addStrokeJob(KisStrokeJob *strokeJob);


    /**
     * Adds a spontaneous job to the context. The prerequisites are
     * the same as for addMergeJob()
     * \see addMergeJob()
     */
    virtual bool addSpontaneousJob(KisSpontaneousJob *spontaneousJob);

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

    /**
     * Set the number of threads available for this updater context
     * WARNING: one cannot change the number of threads if there is
     *          at least one job running in the context! So before
     *          calling this method make sure you do two things:
     *          1) barrierLock() the update scheduler
     *          2) lock() the context
     */
    void setThreadsLimit(int value);

    /**
     * Return the number of available threads in the context. Make sure you
     * lock the context before calling this function!
     */
    int threadsLimit() const;

    void jobFinished(int index);
    void continueUpdate(const QRect& rc);
    void doSomeUsefulWork();

    friend class KisUpdateJobItem;

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

    mutable QReadWriteLock m_rwLock;
    QVector<KisUpdateJobItem*> m_jobs;
    QThreadPool m_threadPool;
    KisLockFreeLodCounter m_lodCounter;
<<<<<<< HEAD
    KisLocklessStack<int> m_spareThreadsIndexes;
=======
>>>>>>> master
    KisUpdateScheduler *m_scheduler;
};

class KRITAIMAGE_EXPORT KisTestableUpdaterContext : public KisUpdaterContext
{
public:
    /**
     * Creates an explicit number of threads
     */
    KisTestableUpdaterContext(qint32 threadCount);
    ~KisTestableUpdaterContext() override;

    /**
     * The only difference - it doesn't start execution
     * of the job
     */
    bool addMergeJob(KisBaseRectsWalkerSP walker) override;
    bool addStrokeJob(KisStrokeJob *strokeJob) override;
    bool addSpontaneousJob(KisSpontaneousJob *spontaneousJob) override;

    const QVector<KisUpdateJobItem*> getJobs();
    void clear();

    friend class KisUpdateJobItem;
};




#endif /* __KIS_UPDATER_CONTEXT_H */

