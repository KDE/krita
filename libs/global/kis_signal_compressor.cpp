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
 *   - timeout after = [0.5 ... 1.0] * \p delay ms.
 * FIRST_ACTIVE_POSTPONE_NEXT:
 *   - first timeout immediately
 *   - postponed timeout after [0.5 ... 1.0] * \p delay ms
 * FIRST_ACTIVE:
 *   - first timeout immediately
 *   - after that [0.5 ... 1.5] * \p delay ms
  * FIRST_INACTIVE:
 *   - timeout after [0.5 ... 1.5] * \p delay ms
 */

#include "kis_signal_compressor.h"

#include <QTimer>
#include "kis_assert.h"
#include "kis_debug.h"


KisSignalCompressor::KisSignalCompressor()
    : QObject(0)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimerExpired()));
}

KisSignalCompressor::KisSignalCompressor(int delay, Mode mode, QObject *parent)
    : QObject(parent),
      m_timer(new QTimer(this)),
      m_mode(mode)
{
    m_timer->setSingleShot(false);
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
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_mode != UNDEFINED);

    const bool isFirstStart = !m_timer->isActive();

    KIS_SAFE_ASSERT_RECOVER_NOOP(!isFirstStart || !m_signalsPending);

    switch (m_mode) {
    case POSTPONE:
        if (isFirstStart) {
            m_timer->start();
        }
        m_lastEmittedTimer.restart();
        m_signalsPending = true;
        break;
    case FIRST_ACTIVE_POSTPONE_NEXT:
    case FIRST_ACTIVE:
        if (isFirstStart) {
            m_timer->start();
            m_lastEmittedTimer.restart();
            m_signalsPending = false;
            if (!tryEmitSignalSafely()) {
                m_signalsPending = true;
            }
        } else {
            if (m_mode == FIRST_ACTIVE) {
                m_signalsPending = true;
                tryEmitOnTick(false);
            } else {
                m_lastEmittedTimer.restart();
                m_signalsPending = true;
            }
        }
        break;
    case FIRST_INACTIVE:
        if (isFirstStart) {
            m_timer->start();
            m_lastEmittedTimer.restart();
            m_signalsPending = true;
        } else {
            m_signalsPending = true;
            tryEmitOnTick(false);
        }
    case UNDEFINED:
        ; // Should never happen, but do nothing
    };

    KIS_SAFE_ASSERT_RECOVER(m_timer->isActive()) {
        m_timer->start();
    }
}

bool KisSignalCompressor::tryEmitOnTick(bool isFromTimer)
{
    bool wasEmitted = false;

    // we have different requirements for hi-frequency events (the mean
    // of the events rate must be min(compressorRate, eventsRate)
    const int realInterval = m_timer->interval();
    const int minInterval = realInterval < 100 ? 0.5 * realInterval : realInterval;

    // Enable for debugging:
    // ENTER_FUNCTION() << ppVar(isFromTimer) << ppVar(m_signalsPending) << m_lastEmittedTimer.elapsed();

    if (m_signalsPending && m_lastEmittedTimer.elapsed() >= minInterval) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!isFromTimer || !m_isEmitting);

        m_lastEmittedTimer.start();
        m_signalsPending = false;
        if (!tryEmitSignalSafely()) {
            m_signalsPending = true;
        }
        wasEmitted = true;
    } else if (!isFromTimer) {
        m_signalsPending = true;
    }

    return wasEmitted;
}

bool KisSignalCompressor::tryEmitSignalSafely()
{
    bool wasEmitted = false;

    m_isEmitting++;

    if (m_isEmitting == 1) {
        emit timeout();
        wasEmitted = true;
    }

    m_isEmitting--;

    return wasEmitted;
}

void KisSignalCompressor::slotTimerExpired()
{
    KIS_ASSERT_RECOVER_NOOP(m_mode != UNDEFINED);
    if (!tryEmitOnTick(true)) {
        const int calmDownInterval = 5 * m_timer->interval();

        if (!m_lastEmittedTimer.isValid() ||
            m_lastEmittedTimer.elapsed() > calmDownInterval) {

            m_timer->stop();
        }
    }
}

void KisSignalCompressor::stop()
{
    m_timer->stop();
    m_signalsPending = false;
    m_lastEmittedTimer.invalidate();
}

bool KisSignalCompressor::isActive() const
{
    return m_signalsPending && m_timer->isActive();
}

void KisSignalCompressor::setMode(KisSignalCompressor::Mode mode)
{
    m_mode = mode;
}
