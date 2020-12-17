/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_busy_progress_indicator.h"

#include <QTimer>
#include <QAtomicInt>

#include "KoProgressProxy.h"


struct KisBusyProgressIndicator::Private
{
    Private(KisBusyProgressIndicator *_q)
        : timer(new QTimer(_q)),
          numEmptyTicks(0),
          isStarted(false) {}

    QTimer *timer; // owned by QObject hierarchy
    int numEmptyTicks;
    QAtomicInt numUpdates;
    QAtomicInt timerStarted;
    KoProgressProxy *progressProxy;

    bool isStarted;

    void startProgressReport()
    {
        if (!progressProxy) {
            return;
        }
        isStarted = true;
        progressProxy->setRange(0, 0);
    }

    void stopProgressReport()
    {
        if (!isStarted || !progressProxy) {
            return;
        }
        progressProxy->setRange(0, 100);
        progressProxy->setValue(100);
        isStarted = false;
    }
};


KisBusyProgressIndicator::KisBusyProgressIndicator(KoProgressProxy *progressProxy)
    : m_d(new Private(this))
{
    connect(m_d->timer, SIGNAL(timeout()), SLOT(timerFinished()));
    connect(this, SIGNAL(sigStartTimer()), SLOT(slotStartTimer()));
    m_d->timer->setInterval(200);
    m_d->progressProxy = progressProxy;
}

KisBusyProgressIndicator::~KisBusyProgressIndicator()
{
    m_d->stopProgressReport();
}

void KisBusyProgressIndicator::prepareDestroying()
{
    m_d->progressProxy = 0;
}

void KisBusyProgressIndicator::timerFinished()
{
    int value = m_d->numUpdates.fetchAndStoreOrdered(0);

    if (!value) {
        m_d->numEmptyTicks++;

        if (m_d->numEmptyTicks > 2) {
            m_d->timerStarted = 0;
            m_d->timer->stop();
            m_d->stopProgressReport();
        }
    } else {
        m_d->numEmptyTicks = 0;
    }
}

void KisBusyProgressIndicator::update()
{
    m_d->numUpdates.ref();

    if (!m_d->timerStarted) {
        emit sigStartTimer();
    }
}

void KisBusyProgressIndicator::slotStartTimer()
{
    m_d->timerStarted.ref();
    m_d->timer->start();
    m_d->startProgressReport();
}
