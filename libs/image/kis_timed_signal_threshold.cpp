/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
