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

#include "kis_speed_smoother.h"

#include <boost/circular_buffer.hpp>
#include <QElapsedTimer>
#include <QPointF>

#include "kis_debug.h"
#include "kis_global.h"

#define MAX_SMOOTH_HISTORY 10
#define MAX_TIME_DIFF 500
#define MAX_TRACKING_DISTANCE 300
#define MIN_TRACKING_DISTANCE 5


struct KisSpeedSmoother::Private
{
    Private(int historySize)
        : distances(historySize),
          lastSpeed(0)
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
    qreal lastSpeed;
};


KisSpeedSmoother::KisSpeedSmoother()
    : m_d(new Private(MAX_SMOOTH_HISTORY))
{
}

KisSpeedSmoother::~KisSpeedSmoother()
{
}

qreal KisSpeedSmoother::getNextSpeed(const QPointF &pt)
{
    if (m_d->lastPoint.isNull()) {
        m_d->lastPoint = pt;
        return 0.0;
    }

    qreal time = qreal(m_d->timer.nsecsElapsed()) / 1000000;
    qreal dist = kisDistance(pt, m_d->lastPoint);
    m_d->lastPoint = pt;

    m_d->distances.push_back(Private::DistancePoint(dist, time));

    Private::DistanceBuffer::const_reverse_iterator it = m_d->distances.rbegin();
    Private::DistanceBuffer::const_reverse_iterator end = m_d->distances.rend();

    const qreal currentTime = it->time;

    qreal totalDistance = 0;
    qreal startTime = currentTime;

    for (; it != end; ++it) {
        if (currentTime - it->time > MAX_TIME_DIFF) {
            break;
        }

        totalDistance += it->distance;
        startTime = it->time;

        if (totalDistance > MAX_TRACKING_DISTANCE) {
            break;
        }
    }

    qreal totalTime = currentTime - startTime;

    if (totalTime > 0 && totalDistance > MIN_TRACKING_DISTANCE) {
        qreal speed = totalDistance / totalTime;

        const qreal alpha = 0.2;
        m_d->lastSpeed = alpha * speed + (1 - alpha) * m_d->lastSpeed;
    }

    return m_d->lastSpeed;
}
