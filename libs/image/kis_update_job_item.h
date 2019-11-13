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

#ifndef __KIS_UPDATE_JOB_ITEM_H
#define __KIS_UPDATE_JOB_ITEM_H

#include <atomic>

#include <QRunnable>
#include <QReadWriteLock>

#include "kis_stroke_job.h"
#include "kis_spontaneous_job.h"
#include "kis_base_rects_walker.h"
#include "kis_async_merger.h"
#include "kis_updater_context.h"

//#define DEBUG_JOBS_SEQUENCE


class KisUpdateJobItem :  public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum class Type : int {
        EMPTY = 0,
        WAITING,
        MERGE,
        STROKE,
        SPONTANEOUS
    };

public:
    KisUpdateJobItem(KisUpdaterContext *updaterContext)
        : m_updaterContext(updaterContext),
          m_atomicType(Type::EMPTY),
          m_runnableJob(0)
    {
        setAutoDelete(false);
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_atomicType.is_lock_free());
    }
    ~KisUpdateJobItem() override
    {
        delete m_runnableJob;
    }

    void run() override {
        if (!isRunning()) return;

        /**
         * Here we break the idea of QThreadPool a bit. Ideally, we should split the
         * jobs into distinct QRunnable objects and pass all of them to QThreadPool.
         * That is a nice idea, but it doesn't work well when the jobs are small enough
         * and the number of available cores is high (>4 cores). It this case the
         * threads just tend to execute the job very quickly and go to sleep, which is
         * an expensive operation.
         *
         * To overcome this problem we try to bulk-process the jobs. In sigJobFinished()
         * signal (which is DirectConnection), the context may add the job to ourselves(!!!),
         * so we switch from "done" state into "running" again.
         */

        while (1) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(isRunning());

            if(m_exclusive) {
                m_updaterContext->m_exclusiveJobLock.lockForWrite();
            } else {
                m_updaterContext->m_exclusiveJobLock.lockForRead();
            }

            if(m_atomicType == Type::MERGE) {
                runMergeJob();
            } else {
                KIS_ASSERT(m_atomicType == Type::STROKE ||
                           m_atomicType == Type::SPONTANEOUS);

                if (m_runnableJob) {
#ifdef DEBUG_JOBS_SEQUENCE
                    if (m_atomicType == Type::STROKE) {
                        qDebug() << "running: stroke" << m_runnableJob->id();
                    } else if (m_atomicType == Type::SPONTANEOUS) {
                        qDebug() << "running: spont " << m_runnableJob->id();
                    } else {
                        qDebug() << "running: unkn. " << m_runnableJob->id();
                    }
#endif

                    m_runnableJob->run();
                }
            }

            setDone();

            m_updaterContext->doSomeUsefulWork();

            // may flip the current state from Waiting -> Running again
            m_updaterContext->jobFinished();

            m_updaterContext->m_exclusiveJobLock.unlock();

            // try to exit the loop. Please note, that no one can flip the state from
            // WAITING to EMPTY except ourselves!
            Type expectedValue = Type::WAITING;
            if (m_atomicType.compare_exchange_strong(expectedValue, Type::EMPTY)) {
                break;
            }
        }
    }

    inline void runMergeJob() {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_atomicType == Type::MERGE);
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_walker);
        // dbgKrita << "Executing merge job" << m_walker->changeRect()
        //          << "on thread" << QThread::currentThreadId();

#ifdef DEBUG_JOBS_SEQUENCE
        qDebug() << "running: merge " << m_walker->startNode() << m_walker->changeRect();

#endif

        m_merger.startMerge(*m_walker);

        QRect changeRect = m_walker->changeRect();
        m_updaterContext->continueUpdate(changeRect);
    }

    // return true if the thread should actually be started
    inline bool setWalker(KisBaseRectsWalkerSP walker) {
        KIS_ASSERT(m_atomicType <= Type::WAITING);

        m_accessRect = walker->accessRect();
        m_changeRect = walker->changeRect();
        m_walker = walker;

        m_exclusive = false;
        m_runnableJob = 0;

        const Type oldState = m_atomicType.exchange(Type::MERGE);
        return oldState == Type::EMPTY;
    }

    // return true if the thread should actually be started
    inline bool setStrokeJob(KisStrokeJob *strokeJob) {
        KIS_ASSERT(m_atomicType <= Type::WAITING);

        m_runnableJob = strokeJob;
        m_strokeJobSequentiality = strokeJob->sequentiality();

        m_exclusive = strokeJob->isExclusive();
        m_walker = 0;
        m_accessRect = m_changeRect = QRect();

        const Type oldState = m_atomicType.exchange(Type::STROKE);
        return oldState == Type::EMPTY;
    }

    // return true if the thread should actually be started
    inline bool setSpontaneousJob(KisSpontaneousJob *spontaneousJob) {
        KIS_ASSERT(m_atomicType <= Type::WAITING);

        m_runnableJob = spontaneousJob;

        m_exclusive = spontaneousJob->isExclusive();
        m_walker = 0;
        m_accessRect = m_changeRect = QRect();

        const Type oldState = m_atomicType.exchange(Type::SPONTANEOUS);
        return oldState == Type::EMPTY;
    }

    inline void setDone() {
        m_walker = 0;
        delete m_runnableJob;
        m_runnableJob = 0;
        m_atomicType = Type::WAITING;
    }

    inline bool isRunning() const {
        return m_atomicType >= Type::MERGE;
    }

    inline Type type() const {
        return m_atomicType;
    }

    inline const QRect& accessRect() const {
        return m_accessRect;
    }

    inline const QRect& changeRect() const {
        return m_changeRect;
    }

    inline KisStrokeJobData::Sequentiality strokeJobSequentiality() const {
        return m_strokeJobSequentiality;
    }

private:
    /**
     * Open walker and stroke job for the testing suite.
     * Please, do not use it in production code.
     */
    friend class KisTestableUpdaterContext;
    friend class KisSimpleUpdateQueueTest;
    friend class KisStrokesQueueTest;
    friend class KisUpdateSchedulerTest;
    friend class KisUpdaterContext;

    inline KisBaseRectsWalkerSP walker() const {
        return m_walker;
    }

    inline KisStrokeJob* strokeJob() const {
        KisStrokeJob *job = dynamic_cast<KisStrokeJob*>(m_runnableJob);
        Q_ASSERT(job);

        return job;
    }

    inline void testingSetDone() {
        setDone();
    }

private:
    KisUpdaterContext *m_updaterContext;

    bool m_exclusive;

    std::atomic<Type> m_atomicType;

    volatile KisStrokeJobData::Sequentiality m_strokeJobSequentiality;

    /**
     * Runnable jobs part
     * The job is owned by the context and deleted after completion
     */
    KisRunnableWithDebugName *m_runnableJob;

    /**
     * Merge jobs part
     */

    KisBaseRectsWalkerSP m_walker;
    KisAsyncMerger m_merger;

    /**
     * These rects cache actual values from the walker
     * to eliminate concurrent access to a walker structure
     */
    QRect m_accessRect;
    QRect m_changeRect;
};


#endif /* __KIS_UPDATE_JOB_ITEM_H */
