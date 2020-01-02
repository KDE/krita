/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2011-12-28
 * @brief  re-implementation of action thread using threadweaver
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 * @author Copyright (C) 2011-2012 by A Janardhan Reddy
 *         <a href="annapareddyjanardhanreddy at gmail dot com">annapareddyjanardhanreddy at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "ractionthreadbase.h"

// Qt includes

#include <QMutexLocker>
#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <QList>
#include <QThreadPool>

// Local includes

#include "libkdcraw_debug.h"

namespace KDcrawIface
{

class Q_DECL_HIDDEN RActionThreadBase::Private
{
public:

    Private()
    {
        running = false;
        pool    = QThreadPool::globalInstance();
    }

    volatile bool  running;

    QWaitCondition condVarJobs;
    QMutex         mutex;

    RJobCollection todo;
    RJobCollection pending;
    RJobCollection processed;

    QThreadPool*   pool;
};

RActionThreadBase::RActionThreadBase(QObject* const parent)
    : QThread(parent),
      d(new Private)
{
    defaultMaximumNumberOfThreads();
}

RActionThreadBase::~RActionThreadBase()
{
    // cancel the thread
    cancel();
    // wait for the thread to finish
    wait();

    // Cleanup all jobs from memory
    Q_FOREACH (RActionJob* const job, d->todo.keys())
        delete(job);

    Q_FOREACH (RActionJob* const job, d->pending.keys())
        delete(job);

    Q_FOREACH (RActionJob* const job, d->processed.keys())
        delete(job);

    delete d;
}

void RActionThreadBase::setMaximumNumberOfThreads(int n)
{
    d->pool->setMaxThreadCount(n);
    qCDebug(LIBKDCRAW_LOG) << "Using " << n << " CPU core to run threads";
}

int RActionThreadBase::maximumNumberOfThreads() const
{
    return d->pool->maxThreadCount();
}

void RActionThreadBase::defaultMaximumNumberOfThreads()
{
    const int maximumNumberOfThreads = qMax(QThreadPool::globalInstance()->maxThreadCount(), 1);
    setMaximumNumberOfThreads(maximumNumberOfThreads);
}

void RActionThreadBase::slotJobFinished()
{
    RActionJob* const job = dynamic_cast<RActionJob*>(sender());
    if (!job) return;

    qCDebug(LIBKDCRAW_LOG) << "One job is done";

    QMutexLocker lock(&d->mutex);

    d->processed.insert(job, 0);
    d->pending.remove(job);

    if (isEmpty())
    {
        d->running = false;
    }

    d->condVarJobs.wakeAll();
}

void RActionThreadBase::cancel()
{
    qCDebug(LIBKDCRAW_LOG) << "Cancel Main Thread";
    QMutexLocker lock(&d->mutex);

    d->todo.clear();

    Q_FOREACH (RActionJob* const job, d->pending.keys())
    {
        job->cancel();
        d->processed.insert(job, 0);
    }

    d->pending.clear();
    d->condVarJobs.wakeAll();
    d->running = false;
}

bool RActionThreadBase::isEmpty() const
{
    return d->pending.isEmpty();
}

void RActionThreadBase::appendJobs(const RJobCollection& jobs)
{
    QMutexLocker lock(&d->mutex);

    for (RJobCollection::const_iterator it = jobs.begin() ; it != jobs.end(); ++it)
    {
        d->todo.insert(it.key(), it.value());
    }

    d->condVarJobs.wakeAll();
}

void RActionThreadBase::run()
{
    d->running = true;

    while (d->running)
    {
        QMutexLocker lock(&d->mutex);

        if (!d->todo.isEmpty())
        {
            qCDebug(LIBKDCRAW_LOG) << "Action Thread run " << d->todo.count() << " new jobs";

            for (RJobCollection::iterator it = d->todo.begin() ; it != d->todo.end(); ++it)
            {
                RActionJob* const job = it.key();
                int priority          =  it.value();

                connect(job, SIGNAL(signalDone()),
                        this, SLOT(slotJobFinished()));

                d->pool->start(job, priority);
                d->pending.insert(job, priority);
            }

            d->todo.clear();
        }
        else
        {
            d->condVarJobs.wait(&d->mutex);
        }
    }
}

}  // namespace KDcrawIface
