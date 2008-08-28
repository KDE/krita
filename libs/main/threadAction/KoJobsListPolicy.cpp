/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoJobsListPolicy.h"

#include <threadweaver/Job.h>

using namespace ThreadWeaver;

KoJobsListPolicy::KoJobsListPolicy() : mutex(QMutex::Recursive)
{
}

KoJobsListPolicy::~KoJobsListPolicy()
{
    foreach(Job *job , m_jobs)
    job->removeQueuePolicy(this);
}

bool KoJobsListPolicy::canRun(Job *job)
{
    QMutexLocker ml(&mutex);
    if (canRunMutex.tryLock()) {
        bool rc = m_jobs.isEmpty() || m_jobs[0] == job;
        canRunMutex.unlock();
        return rc;
    } else {
        return false;
    }
}

void KoJobsListPolicy::free(Job *job)
{
    Q_UNUSED(job);
    release(job);
}

void KoJobsListPolicy::release(Job *job)
{
    QMutexLocker ml(&mutex);
    Q_ASSERT(m_jobs.contains(job));
    m_jobs.removeAll(job);
    job->removeQueuePolicy(this);
}

void KoJobsListPolicy::destructed(Job *job)
{
    release(job);
}

const QList<Job*> KoJobsListPolicy::jobs()
{
    QList<Job*> answer;
    QMutexLocker ml(&mutex);
    foreach(Job *job, m_jobs)
    answer.append(job);
    return answer;
}

void KoJobsListPolicy::addJob(Job *job)
{
    QMutexLocker ml(&mutex);
    m_jobs.append(job);
}

int KoJobsListPolicy::count()
{
    QMutexLocker ml(&mutex);
    int i = m_jobs.count();
    return i;
}

ThreadWeaver::Job* KoJobsListPolicy::firstJob()
{
    QMutexLocker ml(&mutex);
    if (m_jobs.isEmpty()) return 0;
    return m_jobs[0];
}
