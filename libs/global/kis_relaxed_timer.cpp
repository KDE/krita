/*
 *  Copyright (c) 2017 Bernhard Liebl <poke1024@gmx.de>
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

#include "kis_relaxed_timer.h"
#include "kis_assert.h"

KisRelaxedTimer::KisRelaxedTimer(QObject *parent)
    : QObject(parent)
    , m_interval(0)
    , m_singleShot(false)
    , m_nextTimerTickSeqNo(1)
    , m_emitOnTimeTick(0)
    , m_isEmitting(false)
{
}

void KisRelaxedTimer::setInterval(int interval)
{
    KIS_SAFE_ASSERT_RECOVER(!isActive()) {
        this->stop();
    }

    m_interval = interval;
}

void KisRelaxedTimer::setSingleShot(bool singleShot)
{
    m_singleShot = singleShot;
}

int KisRelaxedTimer::remainingTime() const
{
    // in contrast to normal QTimers, the remaining time is calculated in
    // terms of 2 * m_interval as this is the worst case interval.

    if (!isActive()) {
        return -1;
    } else {
        return qMax(qint64(0), 2 * m_interval - qint64(m_elapsed.elapsed()));
    }
}

void KisRelaxedTimer::start()
{
    m_elapsed.start();

    // cancels any previously scheduled timer and schedules a new timer to be
    // triggered as soon as possible, but never sooner than \p m_interval ms.

    if (!m_timer.isActive()) {
        // no internal timer is running. start one, and configure it to send
        // us a timeout event on the next possible tick which will be exactly
        // \p m_interval ms in the future.

        m_emitOnTimeTick = m_nextTimerTickSeqNo;
        m_timer.start(m_interval, this);
    } else if (m_isEmitting) {
        // an internal timer is running and we are actually called from a
        // timeout event. so we know the next tick will happen in exactly
        // \p m_interval ms.

        m_emitOnTimeTick = m_nextTimerTickSeqNo;
    } else {
        // an internal timer is already running, but we do not know when
        // the next tick will happen. we need to skip next tick as it
        // will be sooner than m_delay. the one after that will be good as
        // it will be m_interval * (1 + err) in the future.

        m_emitOnTimeTick = m_nextTimerTickSeqNo + 1;
    }
}

void KisRelaxedTimer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    const int ticksStopThreshold = 5;

    const qint64 timerTickSeqNo = m_nextTimerTickSeqNo;

    // from this point on, if this is an emit tick, we are no longer active.
    m_nextTimerTickSeqNo++;

    if (timerTickSeqNo == m_emitOnTimeTick) {
        if (m_singleShot) {
            stop();
        }
        const IsEmitting emitting(*this);
        emit timeout();
    } else if (timerTickSeqNo - m_emitOnTimeTick > ticksStopThreshold) {
        m_timer.stop();
    }
}
