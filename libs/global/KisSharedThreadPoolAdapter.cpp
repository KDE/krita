/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisSharedThreadPoolAdapter.h"

#include "kis_assert.h"
#include <QThreadPool>
#include <QElapsedTimer>


KisSharedThreadPoolAdapter::KisSharedThreadPoolAdapter(QThreadPool *parentPool)
    : m_parentPool(parentPool),
      m_numRunningJobs(0)
{
}

KisSharedThreadPoolAdapter::~KisSharedThreadPoolAdapter()
{
    waitForDone();
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_numRunningJobs);
}

void KisSharedThreadPoolAdapter::start(KisSharedRunnable *runnable, int priority)
{
    QMutexLocker l(&m_mutex);

    runnable->setSharedThreadPoolAdapter(this);
    m_parentPool->start(runnable, priority);

    m_numRunningJobs++;
}

bool KisSharedThreadPoolAdapter::tryStart(KisSharedRunnable *runnable)
{
    QMutexLocker l(&m_mutex);

    runnable->setSharedThreadPoolAdapter(this);
    const bool result = m_parentPool->tryStart(runnable);

    if (result) {
        m_numRunningJobs++;
    }

    return result;
}

bool KisSharedThreadPoolAdapter::waitForDone(int msecs)
{
    QElapsedTimer t;
    t.start();

    while (1) {
        QMutexLocker l(&m_mutex);

        if (!m_numRunningJobs) break;

        const qint64 elapsed = t.elapsed();
        if (msecs >= 0 && msecs < elapsed) return false;

        const unsigned long timeout = msecs < 0 ? ULONG_MAX : msecs - elapsed;

        m_waitCondition.wait(&m_mutex, timeout);
    }

    return true;
}

void KisSharedThreadPoolAdapter::notifyJobCompleted()
{
    QMutexLocker l(&m_mutex);

    KIS_SAFE_ASSERT_RECOVER (m_numRunningJobs > 0) {
        m_waitCondition.wakeAll();
        return;
    }

    m_numRunningJobs--;
    if (!m_numRunningJobs) {
        m_waitCondition.wakeAll();
    }
}


