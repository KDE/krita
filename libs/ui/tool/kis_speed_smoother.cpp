/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_speed_smoother.h"

#include <boost/circular_buffer.hpp>
#include <QElapsedTimer>
#include <QPointF>

#include "kis_debug.h"
#include "kis_global.h"
#include "kis_config.h"

#define MAX_SMOOTH_HISTORY 512

#define NUM_SMOOTHING_SAMPLES 3
#define MIN_TRACKING_DISTANCE 5

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>
#include "kis_algebra_2d.h"

using namespace boost::accumulators;

struct KisSpeedSmoother::Private
{
    Private(int historySize)
        : distances(historySize),
          lastSpeed(0),
          timeDiffAccumulator(tag::rolling_window::window_size = 200)
    {
        timer.start();
    }

    struct DistancePoint {
        DistancePoint()
            : distance(0), time(0)
        {
        }

        DistancePoint(qreal _distance, qreal _time)
            : distance(_distance), time(_time)
        {
        }

        qreal distance;
        qreal time;
    };

    typedef boost::circular_buffer<DistancePoint> DistanceBuffer;
    DistanceBuffer distances;

    QPointF lastPoint;
    QElapsedTimer timer;
    qreal lastTime;
    qreal lastSpeed;

    accumulator_set<qreal, stats<tag::rolling_mean>> timeDiffAccumulator;

};


KisSpeedSmoother::KisSpeedSmoother()
    : m_d(new Private(MAX_SMOOTH_HISTORY))
{
}

KisSpeedSmoother::~KisSpeedSmoother()
{
}

qreal KisSpeedSmoother::lastSpeed() const
{
    return m_d->lastSpeed;
}

qreal KisSpeedSmoother::getNextSpeed(const QPointF &pt, ulong timestamp)
{
    return getNextSpeedImpl(pt, timestamp);
}

qreal KisSpeedSmoother::getNextSpeed(const QPointF &pt)
{
    const qreal time = qreal(m_d->timer.nsecsElapsed()) / 1000000;
    return getNextSpeedImpl(pt, time);
}

void KisSpeedSmoother::clear()
{
    m_d->timer.restart();
    m_d->distances.clear();
    m_d->distances.push_back(Private::DistancePoint(0.0, 0.0));
    m_d->lastPoint = QPointF();
    m_d->lastSpeed = 0.0;
}

qreal KisSpeedSmoother::getNextSpeedImpl(const QPointF &pt, qreal time)
{
    const qreal dist = kisDistance(pt, m_d->lastPoint);

    if (m_d->lastPoint.isNull()) {
        m_d->lastPoint = pt;
        m_d->lastTime = time;
        m_d->lastSpeed = 0.0;
        return 0.0;
    }

    const qreal avgTimeDiff = rolling_mean(m_d->timeDiffAccumulator);

    const qreal timeDiff = time - m_d->lastTime;

    const qreal timeDiffPortion =
        !qFuzzyIsNull(avgTimeDiff) ? timeDiff / avgTimeDiff : 1.0;

    // don't count samples that are too different from average
    if (timeDiffPortion > 0.25 && timeDiffPortion < 2.0) {
        m_d->timeDiffAccumulator(timeDiff);
    }

    m_d->lastPoint = pt;
    m_d->lastTime = time;

    m_d->distances.push_back(Private::DistancePoint(dist, time));

    Private::DistanceBuffer::const_reverse_iterator it = m_d->distances.rbegin();
    Private::DistanceBuffer::const_reverse_iterator end = m_d->distances.rend();

    qreal totalDistance = 0;
    qreal totalTime = 0.0;
    int itemsSearched = 0;

    for (; it != end; ++it) {
        itemsSearched++;
        totalDistance += it->distance;
        totalTime += avgTimeDiff;

        if (itemsSearched > NUM_SMOOTHING_SAMPLES &&
            totalDistance > MIN_TRACKING_DISTANCE) {

            break;
        }
    }

    if (totalTime > 0 && totalDistance > MIN_TRACKING_DISTANCE) {
        m_d->lastSpeed = totalDistance / totalTime;
    }

    return m_d->lastSpeed;
}
