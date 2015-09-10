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
#include <QTimer>
#include <KoProgressProxy.h>


struct KisQueuesProgressUpdater::Private
{
    Private()
        : queueSizeMetric(0)
        , initialQueueSizeMetric(0)
        , progressProxy(0)
        , tickingRequested(false)
    {
    }

    QMutex mutex;
    QTimer timer;

    int queueSizeMetric;
    int initialQueueSizeMetric;
    QString jobName;

    KoProgressProxy *progressProxy;

    bool tickingRequested;

    static const int TIMER_INTERVAL = 500;
};


KisQueuesProgressUpdater::KisQueuesProgressUpdater(KoProgressProxy *progressProxy)
    : m_d(new Private)
{
    m_d->progressProxy = progressProxy;

    m_d->timer.setInterval(Private::TIMER_INTERVAL);
    m_d->timer.setSingleShot(false);

    connect(this, SIGNAL(sigStartTicking()), SLOT(startTicking()), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopTicking()), SLOT(stopTicking()), Qt::QueuedConnection);
    connect(&m_d->timer, SIGNAL(timeout()), SLOT(timerTicked()));
}

KisQueuesProgressUpdater::~KisQueuesProgressUpdater()
{
    delete m_d;
}

void KisQueuesProgressUpdater::updateProgress(int queueSizeMetric, const QString &jobName)
{
    QMutexLocker locker(&m_d->mutex);

    m_d->queueSizeMetric = queueSizeMetric;

    if (queueSizeMetric &&
        (jobName != m_d->jobName ||
         m_d->queueSizeMetric > m_d->initialQueueSizeMetric)) {

        m_d->jobName = jobName;
        m_d->initialQueueSizeMetric = m_d->queueSizeMetric;
    }

    if (m_d->queueSizeMetric && !m_d->tickingRequested) {

        m_d->tickingRequested = true;
        emit sigStartTicking();

    } else if (!m_d->queueSizeMetric && m_d->tickingRequested) {

        m_d->initialQueueSizeMetric = 0;
        m_d->jobName.clear();
        m_d->tickingRequested = false;
        emit sigStopTicking();
    }
}

void KisQueuesProgressUpdater::hide()
{
    updateProgress(0, "");
}

void KisQueuesProgressUpdater::startTicking()
{
    m_d->timer.start();
    timerTicked();
}

void KisQueuesProgressUpdater::stopTicking()
{
    m_d->timer.stop();
    timerTicked();
}

void KisQueuesProgressUpdater::timerTicked()
{
    QMutexLocker locker(&m_d->mutex);

    m_d->progressProxy->setRange(0, m_d->initialQueueSizeMetric);
    m_d->progressProxy->setValue(m_d->initialQueueSizeMetric - m_d->queueSizeMetric);
    m_d->progressProxy->setFormat(m_d->jobName);
}
