/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_queues_progress_updater.h"

#include <QMutex>
#include <QTimer>
#include <KoProgressProxy.h>


struct Q_DECL_HIDDEN KisQueuesProgressUpdater::Private
{
    Private(KisQueuesProgressUpdater *q)
        : timer(q)
        , startDelayTimer(q)
        , queueSizeMetric(0)
        , initialQueueSizeMetric(0)
        , progressProxy(0)
        , tickingRequested(false)
    {
    }

    QMutex mutex;
    QTimer timer;
    QTimer startDelayTimer;

    int queueSizeMetric;
    int initialQueueSizeMetric;
    QString jobName;

    KoProgressProxy *progressProxy;

    bool tickingRequested;

    static const int TIMER_INTERVAL = 500;
    static const int PROGRESS_DELAY = 1000;
};


KisQueuesProgressUpdater::KisQueuesProgressUpdater(KoProgressProxy *progressProxy, QObject *parent)
    : QObject(parent),
      m_d(new Private(this))
{
    m_d->progressProxy = progressProxy;

    m_d->timer.setInterval(Private::TIMER_INTERVAL);
    m_d->timer.setSingleShot(false);

    connect(this, SIGNAL(sigStartTicking()), SLOT(startTicking()), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopTicking()), SLOT(stopTicking()), Qt::QueuedConnection);
    connect(&m_d->timer, SIGNAL(timeout()), SLOT(timerTicked()));

    m_d->startDelayTimer.setInterval(Private::PROGRESS_DELAY);
    m_d->startDelayTimer.setSingleShot(true);

    connect(&m_d->startDelayTimer, SIGNAL(timeout()), &m_d->timer, SLOT(start()));
    connect(&m_d->startDelayTimer, SIGNAL(timeout()), SLOT(timerTicked()));
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
    m_d->startDelayTimer.start();
}

void KisQueuesProgressUpdater::stopTicking()
{
    m_d->startDelayTimer.stop();
    m_d->timer.stop();
    timerTicked();
}

void KisQueuesProgressUpdater::timerTicked()
{
    QMutexLocker locker(&m_d->mutex);

    if (!m_d->initialQueueSizeMetric) {
        m_d->progressProxy->setRange(0, 100);
        m_d->progressProxy->setValue(100);
        m_d->progressProxy->setFormat("%p%");
    } else {
        m_d->progressProxy->setRange(0, m_d->initialQueueSizeMetric);
        m_d->progressProxy->setValue(m_d->initialQueueSizeMetric - m_d->queueSizeMetric);
        m_d->progressProxy->setFormat(m_d->jobName);
    }
}
