/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_busy_progress_indicator.h"

#include <QTimer>
#include <QAtomicInt>

#include "KoProgressProxy.h"


struct KisBusyProgressIndicator::Private
{
    Private() : numEmptyTicks(0) {}

    QTimer timer;
    int numEmptyTicks;
    QAtomicInt numUpdates;
    QAtomicInt timerStarted;
    KoProgressProxy *progressProxy;

    void startProgressReport() {
        progressProxy->setRange(0, 0);
    }

    void stopProgressReport() {
        progressProxy->setRange(0, 100);
        progressProxy->setValue(100);
    }
};


KisBusyProgressIndicator::KisBusyProgressIndicator(KoProgressProxy *progressProxy)
    : m_d(new Private)
{
    connect(&m_d->timer, SIGNAL(timeout()), SLOT(timerFinished()));
    connect(this, SIGNAL(sigStartTimer()), SLOT(slotStartTimer()));
    m_d->timer.setInterval(200);
    m_d->progressProxy = progressProxy;
}

KisBusyProgressIndicator::~KisBusyProgressIndicator()
{
    m_d->stopProgressReport();
}

void KisBusyProgressIndicator::timerFinished()
{
    int value = m_d->numUpdates.fetchAndStoreOrdered(0);

    if (!value) {
        m_d->numEmptyTicks++;

        if (m_d->numEmptyTicks > 2) {
            m_d->timerStarted = 0;
            m_d->timer.stop();
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
    m_d->timer.start();
    m_d->startProgressReport();
}
