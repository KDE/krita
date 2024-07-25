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

#include "kis_algebra_2d.h"
#include "KisFilteredRollingMean.h"

struct KisSpeedSmoother::Private
{
    Private(int historySize)
        : distances(historySize),
          timeDiffsMean(200, 0.8)
    {
        timer.start();
    }

    struct DistancePoint {
        DistancePoint()
            : distance(0.0)
            , time(0.0)
        {
        }

        DistancePoint(qreal _distance, qreal _time)
            : distance(_distance)
            , time(_time)
        {
        }

        qreal distance {0.0};
        qreal time {0.0};
    };

    typedef boost::circular_buffer<DistancePoint> DistanceBuffer;
    DistanceBuffer distances;

    KisFilteredRollingMean timeDiffsMean;

    QPointF lastPoint;
    QElapsedTimer timer;
    qreal lastTime {0.0};
    qreal lastSpeed {0.0};

    bool useTimestamps {false};
    int numSmoothingSamples {3};
};


KisSpeedSmoother::KisSpeedSmoother()
    : m_d(new Private(MAX_SMOOTH_HISTORY))
{
    updateSettings();
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
    const qreal time = m_d->useTimestamps ?
        qreal(timestamp) :
        qreal(m_d->timer.nsecsElapsed()) / 1000000;

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

void KisSpeedSmoother::updateSettings()
{

    KisConfig cfg(true);
    m_d->useTimestamps = cfg.readEntry("useTimestampsForBrushSpeed", false);
    m_d->numSmoothingSamples = cfg.readEntry("speedValueSmoothing", 3);
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

    const qreal timeDiff = time - m_d->lastTime;

    m_d->timeDiffsMean.addValue(timeDiff);
    const qreal avgTimeDiff = m_d->timeDiffsMean.filteredMean();

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

        /**
         * Mind you, we don't care about the specific timestamps
         * of the tablet events! They are not reliable. Instead,
         * we are trying to estimate the sample rate of the tablet
         * itself using the filtered mean accumulator. It works in
         * an assumption that all the tablets generate events at a
         * fixed sample rate.
         */
        totalTime += avgTimeDiff;

        if (itemsSearched > m_d->numSmoothingSamples &&
            totalDistance > MIN_TRACKING_DISTANCE) {

            break;
        }
    }

    if (totalTime > 0 && totalDistance > MIN_TRACKING_DISTANCE) {
        m_d->lastSpeed = totalDistance / totalTime;
    }

    return m_d->lastSpeed;
}
