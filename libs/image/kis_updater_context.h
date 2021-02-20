/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_UPDATER_CONTEXT_H
#define __KIS_UPDATER_CONTEXT_H

#include <QMutex>
#include <QReadWriteLock>
#include <QThreadPool>

#include "kis_base_rects_walker.h"
#include "kis_async_merger.h"
#include "kis_lock_free_lod_counter.h"

#include "KisUpdaterContextSnapshotEx.h"
#include "kis_update_scheduler.h"

class KisUpdateJobItem;
class KisSpontaneousJob;
class KisStrokeJob;
class KisUpdateScheduler;

class KRITAIMAGE_EXPORT KisUpdaterContext
{
public:
    static const int useIdealThreadCountTag;

public:
    KisUpdaterContext(qint32 threadCount = useIdealThreadCountTag, KisUpdateScheduler *parent = 0);
    ~KisUpdaterContext();


    /**
     * Returns the number of currently running jobs of each type.
     * To use this information you should lock the context beforehand.
     *
     * \see lock()
     */
    void getJobsSnapshot(qint32 &numMergeJobs, qint32 &numStrokeJobs);

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
    void addMergeJob(KisBaseRectsWalkerSP walker);

    /**
     * Adds a stroke job to the context. The prerequisites are
     * the same as for addMergeJob()
     * \see addMergeJob()
     */
    void addStrokeJob(KisStrokeJob *strokeJob);


    /**
     * Adds a spontaneous job to the context. The prerequisites are
     * the same as for addMergeJob()
     * \see addMergeJob()
     */
    void addSpontaneousJob(KisSpontaneousJob *spontaneousJob);

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

    void continueUpdate(const QRect& rc);
    void doSomeUsefulWork();
    void jobFinished();

    void setTestingMode(bool value);

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
    KisLockFreeLodCounter m_lodCounter;
    KisUpdateScheduler *m_scheduler;
    bool m_testingMode = false;

private:

    friend class KisUpdaterContextTest;
    friend class KisUpdateSchedulerTest;
    friend class KisStrokesQueueTest;
    friend class KisSimpleUpdateQueueTest;
    friend class KisUpdateJobItem;

    const QVector<KisUpdateJobItem*> getJobs();
    void clear();

};

class KRITAIMAGE_EXPORT KisTestableUpdaterContext : public KisUpdaterContext
{
public:
    /**
     * Creates an explicit number of threads
     */
    KisTestableUpdaterContext(qint32 threadCount);
};


#endif /* __KIS_UPDATER_CONTEXT_H */



