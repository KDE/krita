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
#ifndef TW_JOBSLISTPOLICY_H
#define TW_JOBSLISTPOLICY_H
#include <QueuePolicy.h>

#include <QMutex>
#include <QList>

class Job;

namespace ThreadWeaver {
    /**
     * Sequential job-queueing policy.
     * This class holds a list of jobs that will be handled sequentially while allowing
     * a not-yet-executing job to be removed from the queue.
     */
    class JobsListPolicy : public QueuePolicy {
    public:
        /// constructor
        JobsListPolicy();
        /// add a job that the policy will manage
        void addJob(Job *job);

        /// reimplemented method
        bool canRun (Job *job);
        /// reimplemented method
        void free (Job *job);
        /// reimplemented method
        void release (Job *job);
        /// reimplemented method
        void destructed (Job *job);

        /**
         * return a (copy of) the list of jobs.
         * Note that if you actually plan to use the list, by iterating over it or
         * even more, you should lock this policy using lock() which guarentees the
         * list to stay unchanged at least until you call unlock()
         */
        const QList<Job*> jobs();
        /// return the amount of jobs that is curently helt
        int count();

        /**
         * lock this policy from modifications by other threads. Will also disallow new
         * jobs to be started.
         * You should call unlock() afterwards.
         */
        void lock() { mutex.lock(); }
        /**
         * unlock.
         * @see lock()
         */
        void unlock() { mutex.unlock(); }

    private:
        QList<Job*> m_jobs;
        QMutex mutex;
    };
}

#endif
