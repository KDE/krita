/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stabilized_events_sampler.h"

#include <QList>
#include <QElapsedTimer>
#include <QtMath>

#include "kis_paint_information.h"


struct KisStabilizedEventsSampler::Private
{
    Private(int _sampleTime) : sampleTime(_sampleTime), elapsedTimeOverride(0) {}

    QElapsedTimer lastPaintTime;
    QList<KisPaintInformation> realEvents;
    int sampleTime;
    int elapsedTimeOverride;

    KisPaintInformation lastPaintInformation;
};

KisStabilizedEventsSampler::KisStabilizedEventsSampler(int sampleTime)
    : m_d(new Private(sampleTime))
{
}

KisStabilizedEventsSampler::~KisStabilizedEventsSampler()
{
}

void KisStabilizedEventsSampler::clear()
{
    if (!m_d->realEvents.isEmpty()) {
        m_d->lastPaintInformation = m_d->realEvents.last();
    }

    m_d->realEvents.clear();
    m_d->lastPaintTime.start();
}

void KisStabilizedEventsSampler::addEvent(const KisPaintInformation &pi)
{
    if (!m_d->lastPaintTime.isValid()) {
        m_d->lastPaintTime.start();
    }

    m_d->realEvents.append(pi);
}

void KisStabilizedEventsSampler::addFinishingEvent(int numSamples)
{
    if (m_d->realEvents.size() > 0) {
        dbgKrita << "DEBUG: KisStabilizedEventsSampler::addFinishingEvent called "
                    "before `realEvents` is cleared";
        clear();
    }

    m_d->elapsedTimeOverride = numSamples;
    m_d->realEvents.append(m_d->lastPaintInformation);
}

const KisPaintInformation& KisStabilizedEventsSampler::iterator::dereference() const
{
    const int k = qFloor(m_alpha * m_index);
    return k < m_sampler->m_d->realEvents.size() ?
        m_sampler->m_d->realEvents[k] : m_sampler->m_d->lastPaintInformation;
}

std::pair<KisStabilizedEventsSampler::iterator, KisStabilizedEventsSampler::iterator>
KisStabilizedEventsSampler::range() const
{
    const int elapsed = (m_d->lastPaintTime.restart() + m_d->elapsedTimeOverride) / m_d->sampleTime;
    const qreal alpha = qreal(m_d->realEvents.size()) / elapsed;

    m_d->elapsedTimeOverride = 0;

    return std::make_pair(iterator(this, 0, alpha),
                          iterator(this, elapsed, alpha));
}


