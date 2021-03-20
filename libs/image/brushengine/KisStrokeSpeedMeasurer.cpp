/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisStrokeSpeedMeasurer.h"

#include <QQueue>
#include <QVector>

#include "kis_global.h"

struct KisStrokeSpeedMeasurer::Private
{
    struct StrokeSample {
        StrokeSample() {}
        StrokeSample(int _time, qreal _distance) : time(_time), distance(_distance) {}

        int time = 0; /* ms */
        qreal distance = 0;
    };

    int timeSmoothWindow = 0;

    QList<StrokeSample> samples;
    QPointF lastSamplePos;
    int startTime = 0;

    qreal maxSpeed = 0;

    void purgeOldSamples();
    void addSampleImpl(const QPointF &pt, int time);
};

KisStrokeSpeedMeasurer::KisStrokeSpeedMeasurer(int timeSmoothWindow)
    : m_d(new Private())
{
    m_d->timeSmoothWindow = timeSmoothWindow;
}

KisStrokeSpeedMeasurer::~KisStrokeSpeedMeasurer()
{
}

void KisStrokeSpeedMeasurer::Private::addSampleImpl(const QPointF &pt, int time)
{
    if (samples.isEmpty()) {
        lastSamplePos = pt;
        startTime = time;
        samples.append(Private::StrokeSample(time, 0));
    } else {
        Private::StrokeSample &lastSample = samples.last();

        const qreal newStrokeDistance = lastSample.distance + kisDistance(lastSamplePos, pt);
        lastSamplePos = pt;

        if (lastSample.time >= time) {
            lastSample.distance = newStrokeDistance;
        } else {
            samples.append(Private::StrokeSample(time, newStrokeDistance));
        }
    }
}

void KisStrokeSpeedMeasurer::addSample(const QPointF &pt, int time)
{
    m_d->addSampleImpl(pt, time);
    m_d->purgeOldSamples();
    sampleMaxSpeed();
}

void KisStrokeSpeedMeasurer::addSamples(const QVector<QPointF> &points, int time)
{
    const int lastSampleTime = !m_d->samples.isEmpty() ? m_d->samples.last().time : 0;

    const int timeSmoothBase = qMin(lastSampleTime, time);
    const qreal timeSmoothStep = qreal(time - timeSmoothBase) / points.size();

    for (int i = 0; i < points.size(); i++) {
        const int sampleTime = timeSmoothBase + timeSmoothStep * (i + 1);
        m_d->addSampleImpl(points[i], sampleTime);
    }

    m_d->purgeOldSamples();
    sampleMaxSpeed();
}

qreal KisStrokeSpeedMeasurer::averageSpeed() const
{
    if (m_d->samples.isEmpty()) return 0;

    const Private::StrokeSample &lastSample = m_d->samples.last();

    const int timeDiff = lastSample.time - m_d->startTime;
    if (!timeDiff) return 0;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(timeDiff > 0, 0);

    return lastSample.distance / timeDiff;
}

void KisStrokeSpeedMeasurer::Private::purgeOldSamples()
{
    if (samples.size() <= 1) return;

    const Private::StrokeSample lastSample = samples.last();

    auto lastValueToKeep = samples.end();

    for (auto it = samples.begin(); it != samples.end(); ++it) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(lastSample.time - it->time >= 0);

        if (lastSample.time - it->time < timeSmoothWindow) break;
        lastValueToKeep = it;
    }

    if (lastValueToKeep != samples.begin() &&
        lastValueToKeep != samples.end()) {

        samples.erase(samples.begin(), lastValueToKeep - 1);
    }
}

qreal KisStrokeSpeedMeasurer::currentSpeed() const
{
    if (m_d->samples.size() <= 1) return 0;

    const Private::StrokeSample firstSample = m_d->samples.first();
    const Private::StrokeSample lastSample = m_d->samples.last();

    const int timeDiff = lastSample.time - firstSample.time;
    if (!timeDiff) return 0;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(timeDiff > 0, 0);

    return (lastSample.distance - firstSample.distance) / timeDiff;
}

qreal KisStrokeSpeedMeasurer::maxSpeed() const
{
    return m_d->maxSpeed;
}

void KisStrokeSpeedMeasurer::reset()
{
    m_d->samples.clear();
    m_d->lastSamplePos = QPointF();
    m_d->startTime = 0;
    m_d->maxSpeed = 0;
}

void KisStrokeSpeedMeasurer::sampleMaxSpeed()
{
    if (m_d->samples.size() <= 1) return;

    const Private::StrokeSample firstSample = m_d->samples.first();
    const Private::StrokeSample lastSample = m_d->samples.last();

    const int timeDiff = lastSample.time - firstSample.time;
    if (timeDiff < m_d->timeSmoothWindow) return;

    const qreal speed = currentSpeed();
    if (speed > m_d->maxSpeed) {
        m_d->maxSpeed = speed;
    }
}
