/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_timed_signal_threshold.h"
#include <QElapsedTimer>
#include "kis_debug.h"


struct KisTimedSignalThreshold::Private
{
    Private(int _delay, int _cancelDelay)
        : delay(_delay),
          cancelDelay(0),
          enabled(true)
    {
        setCancelDelay(_cancelDelay);
    }

    void setCancelDelay(int value) {
        cancelDelay = value >= 0 ? value : 2 * delay;
    }

    QElapsedTimer timer;
    int delay;
    int cancelDelay;
    bool enabled;
};


KisTimedSignalThreshold::KisTimedSignalThreshold(int delay, int cancelDelay, QObject *parent)
    : QObject(parent),
      m_d(new Private(delay, cancelDelay))
{
}

KisTimedSignalThreshold::~KisTimedSignalThreshold()
{
}

void KisTimedSignalThreshold::forceDone()
{
    stop();
    emit timeout();
}

void KisTimedSignalThreshold::start()
{
    if (!m_d->enabled) return;

    if (!m_d->timer.isValid()) {
        m_d->timer.start();
    } else if (m_d->timer.elapsed() > m_d->cancelDelay) {
        stop();
    } else if (m_d->timer.elapsed() > m_d->delay) {
        forceDone();
    }
}

void KisTimedSignalThreshold::stop()
{
    m_d->timer.invalidate();
}

void KisTimedSignalThreshold::setEnabled(bool value)
{
    m_d->enabled = value;
    if (!m_d->enabled) {
        stop();
    }
}

void KisTimedSignalThreshold::setDelayThreshold(int delay, int cancelDelay)
{
    m_d->delay = delay;
    m_d->setCancelDelay(cancelDelay);
}
