/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "JobsListPolicy.h"
#include <Job.h>

namespace ThreadWeaver {
JobsListPolicy::JobsListPolicy() : mutex(QMutex::Recursive) {
}

bool JobsListPolicy::canRun (Job *job) {
    mutex.lock();
    bool rc = m_jobs.isEmpty() || m_jobs[0] == job;
    mutex.unlock();
    return rc;
}

void JobsListPolicy::free (Job *job) {
    Q_UNUSED(job);
    release(job);
}

void JobsListPolicy::release (Job *job) {
    mutex.lock();
    m_jobs.removeAll(job);
    job->deleteLater();
    mutex.unlock();
}

void JobsListPolicy::destructed (Job *job) {
    release(job);
}

const QList<Job*> JobsListPolicy::jobs() {
    QList<Job*> answer;
    mutex.lock();
    foreach(Job *job, m_jobs)
        answer.append(job);
    mutex.unlock();
    return answer;
}

void JobsListPolicy::addJob(Job *job) {
    mutex.lock();
    m_jobs.append(job);
    mutex.unlock();
}

int JobsListPolicy::count() {
    mutex.lock();
    int i = m_jobs.count();
    mutex.unlock();
    return i;
}
}
