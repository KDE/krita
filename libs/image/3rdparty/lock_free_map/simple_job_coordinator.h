/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef SIMPLEJOBCOORDINATOR_H
#define SIMPLEJOBCOORDINATOR_H

#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>

#include "kis_assert.h"
#include "atomic.h"
#include "unistd.h"
#include "qsbr.h"
#include "kis_debug.h"

#define SANITY_CHECK

class SimpleJobCoordinator
{
public:
    struct Job {
        virtual ~Job()
        {
        }

        virtual void run() = 0;
    };

private:
    Atomic<quint64> m_job;
    QSBR *m_qsbr = 0;
    QMutex mutex;
    QWaitCondition condVar;
    QAtomicInt m_haveParticipants;

public:
    SimpleJobCoordinator() : m_job(quint64(NULL))
    {
    }
    ~SimpleJobCoordinator() {
        KIS_ASSERT(!m_haveParticipants.loadAcquire());
    }

    Job* loadConsume() const
    {
        return (Job*) m_job.load(Consume);
    }

    void storeRelease(Job* job, QSBR *qsbr)
    {
        {
            QMutexLocker guard(&mutex);
            m_job.store(quint64(job), Release);
            m_qsbr = qsbr;
        }

        condVar.wakeAll();
    }

    void participate()
    {
        m_haveParticipants.ref();

        quint64 prevJob = quint64(NULL);

        for (;;) {
            quint64 job = m_job.load(Consume);
            if (job == prevJob) {
                QMutexLocker guard(&mutex);

                KIS_ASSERT_RECOVER(!job || (m_qsbr && m_qsbr->rawPointerUsers() > 0)) {
                    qDebug() << ppVar(m_qsbr) << (m_qsbr ? m_qsbr->rawPointerUsers() : -13);
                }

                for (;;) {
                    KIS_ASSERT_RECOVER(!job || (m_qsbr && m_qsbr->rawPointerUsers() > 0)) {
                        qDebug() << ppVar(m_qsbr) << (m_qsbr ? m_qsbr->rawPointerUsers() : -13);
                    }

                    job = m_job.loadNonatomic(); // No concurrent writes inside lock
                    if (job != prevJob) {
                        break;
                    }

                    condVar.wait(&mutex);
                    ::usleep(qrand() % 4096);
                }
            }

            if (job == 1) {
                m_haveParticipants.deref();
                return;
            }

            KIS_ASSERT_RECOVER(m_qsbr && m_qsbr->rawPointerUsers() > 0) {
                qDebug() << ppVar(m_qsbr) << (m_qsbr ? m_qsbr->rawPointerUsers() : -13);
            }

            ::usleep(qrand() % 4096);

            reinterpret_cast<Job*>(job)->run();
            prevJob = job;
        }

        m_haveParticipants.deref();
    }

//    void runOne(Job* job)
//    {
//#ifdef SANITY_CHECK
//        KIS_ASSERT_RECOVER_NOOP(job != (Job*) m_job.load(Relaxed));
//#endif // SANITY_CHECK
//        storeRelease(job);
//        job->run();
//    }

    void end()
    {
        {
            QMutexLocker guard(&mutex);
            m_job.store(1, Release);
        }

        condVar.wakeAll();
    }
};

#endif // SIMPLEJOBCOORDINATOR_H
