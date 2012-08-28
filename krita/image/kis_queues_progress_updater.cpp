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

#include "kis_queues_progress_updater.h"

#include <QMutex>
#include <QAtomicInt>
#include <QTimer>
#include <KoProgressProxy.h>


struct KisQueuesProgressUpdater::Private
{
    Private()
        : queuedSizeMetric(0)
        , trackingStarted(false)
        , timerFiredOnce(false)
        , progressProxy(0)
    {
    }

    QMutex mutex;
    QTimer timer;

    QAtomicInt doneSizeMetric;
    int queuedSizeMetric;

    QString jobName;

    bool trackingStarted;
    bool timerFiredOnce;

    KoProgressProxy *progressProxy;

    static const int TIMER_INTERVAL = 250;
};


KisQueuesProgressUpdater::KisQueuesProgressUpdater(KoProgressProxy *progressProxy)
    : m_d(new Private)
{
    m_d->progressProxy = progressProxy;

    m_d->timer.setInterval(Private::TIMER_INTERVAL);
    m_d->timer.setSingleShot(false);
    connect(&m_d->timer, SIGNAL(timeout()), SLOT(updateProxy()));
}

KisQueuesProgressUpdater::~KisQueuesProgressUpdater()
{
    delete m_d;
}

void KisQueuesProgressUpdater::notifyJobDone(int sizeMetric)
{
    m_d->doneSizeMetric.fetchAndAddOrdered(sizeMetric);
}

void KisQueuesProgressUpdater::updateProgress(int /*queueSizeMetric*/, const QString &/*jobName*/)
{
//    QMutexLocker locker(&m_d->mutex);
//    m_d->queuedSizeMetric = queueSizeMetric;
//    m_d->jobName = jobName;

//    if(m_d->queuedSizeMetric && !m_d->timer.isActive()) {
//        m_d->trackingStarted = true;
//        m_d->timerFiredOnce = false;
//        m_d->timer.start();
//    }
//    else if(!m_d->queuedSizeMetric && !m_d->timerFiredOnce) {
//            m_d->trackingStarted = false;
//            m_d->timer.stop();
//            m_d->doneSizeMetric = 0;
//    }
}

void KisQueuesProgressUpdater::updateProxy()
{
//    QMutexLocker locker(&m_d->mutex);

//    if(!m_d->trackingStarted) return;
//    m_d->timerFiredOnce = true;

//    m_d->progressProxy->setRange(0, m_d->doneSizeMetric + m_d->queuedSizeMetric);
//    m_d->progressProxy->setValue(m_d->doneSizeMetric);
//    m_d->progressProxy->setFormat(m_d->jobName);

//    if(!m_d->queuedSizeMetric) {
//        m_d->timer.stop();
//        m_d->doneSizeMetric = 0;
//    }
}
