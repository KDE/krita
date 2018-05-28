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

#include "atomic.h"

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
    QMutex mutex;
    QWaitCondition condVar;

public:
    SimpleJobCoordinator() : m_job(quint64(NULL))
    {
    }

    Job* loadConsume() const
    {
        return (Job*) m_job.load(Consume);
    }

    void storeRelease(Job* job)
    {
        {
            QMutexLocker guard(&mutex);
            m_job.store(quint64(job), Release);
        }

        condVar.wakeAll();
    }

    void participate()
    {
        quint64 prevJob = quint64(NULL);

        for (;;) {
            quint64 job = m_job.load(Consume);
            if (job == prevJob) {
                QMutexLocker guard(&mutex);

                for (;;) {
                    job = m_job.loadNonatomic(); // No concurrent writes inside lock
                    if (job != prevJob) {
                        break;
                    }

                    condVar.wait(&mutex);
                }
            }

            if (job == 1) {
                return;
            }

            reinterpret_cast<Job*>(job)->run();
            prevJob = job;
        }
    }

    void runOne(Job* job)
    {
        Q_ASSERT(job != (Job*) m_job.load(Relaxed));
        storeRelease(job);
        job->run();
    }

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
