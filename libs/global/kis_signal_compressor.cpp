/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

/**
 * KisSignalCompressor will never trigger timeout more often than every \p delay ms,
 * i.e. \p delay ms is a given lower limit defining the highest frequency.
 *
 * The current implementation uses a long-running monitor timer to eliminate the
 * overhead incurred by restarting and stopping timers with each signal. The
 * consequence of this is that the given \p delay ms is not always exactly followed.
 *
 * KisSignalCompressor makes the following callback guarantees (0 < err <= 1, with
 * err == 0 if this is the first signal after a while):
 *
 * POSTPONE:
 *   - timeout after <= (1 + err) * \p delay ms.
 * FIRST_ACTIVE_POSTPONE_NEXT:
 *   - first timeout immediately
 *   - postponed timeout after (1 + err) * \p delay ms
 * FIRST_ACTIVE:
 *   - first timeout immediately
 *   - second timeout after (1 + err) * \p delay ms
 *   - after that: \p delay ms
 * FIRST_INACTIVE:
 *   - timeout after (1 + err) * \p delay ms
 */

#include "kis_signal_compressor.h"
#include "kis_relaxed_timer.h"

KisSignalCompressor::KisSignalCompressor()
    : QObject(0)
    , m_timer(new KisRelaxedTimer(this))
    , m_mode(UNDEFINED)
    , m_gotSignals(false)
{
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimerExpired()));
}

KisSignalCompressor::KisSignalCompressor(int delay, Mode mode, QObject *parent)
    : QObject(parent),
      m_timer(new KisRelaxedTimer(this)),
      m_mode(mode),
      m_gotSignals(false)
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(delay);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimerExpired()));
}

void KisSignalCompressor::setDelay(int delay)
{
    const bool wasActive = m_timer->isActive();

    if (wasActive) {
        m_timer->stop();
    }

    m_timer->setInterval(delay);

    if (wasActive) {
        m_timer->start();
    }
}

void KisSignalCompressor::start()
{
    Q_ASSERT(m_mode != UNDEFINED);

    switch (m_mode) {
    case POSTPONE:
        m_timer->start();
        break;
    case FIRST_ACTIVE_POSTPONE_NEXT:
    case FIRST_ACTIVE:
        if (!m_timer->isActive()) {
            m_gotSignals = false;
            m_timer->start();
            emit timeout();
        } else {
            m_gotSignals = true;
            if (m_mode == FIRST_ACTIVE_POSTPONE_NEXT) {
                m_timer->start();
            } else if (m_mode == FIRST_ACTIVE && m_timer->remainingTime() == 0) {
                // overdue, swamped by other events
                m_timer->stop();
                slotTimerExpired();
            }
        }
        break;
    case FIRST_INACTIVE:
        if (!m_timer->isActive()) {
            m_timer->start();
        }
    case UNDEFINED:
        ; // Should never happen, but do nothing
    };

    if (m_mode == POSTPONE || !m_timer->isActive()) {
        m_timer->start();
    }
}

void KisSignalCompressor::slotTimerExpired()
{
    Q_ASSERT(m_mode != UNDEFINED);
    if ((m_mode != FIRST_ACTIVE && m_mode != FIRST_ACTIVE_POSTPONE_NEXT) || m_gotSignals) {
        m_gotSignals = false;
        emit timeout();
    }
}

void KisSignalCompressor::stop()
{
    m_timer->stop();
}

bool KisSignalCompressor::isActive() const
{
    return m_timer->isActive() && (m_mode != FIRST_ACTIVE || m_gotSignals);
}

void KisSignalCompressor::setMode(KisSignalCompressor::Mode mode)
{
    m_mode = mode;
}
