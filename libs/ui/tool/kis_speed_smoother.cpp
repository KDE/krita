/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_speed_smoother.h"

#include <boost/circular_buffer.hpp>
#include <QPointF>

#include <KoPointerEvent.h>

#include "kis_debug.h"
#include "kis_global.h"

#define MAX_SMOOTH_HISTORY 512
#define MAX_SAMPLE_COUNT 10
#define MAX_TIME_DIFF 15


struct KisSpeedSmoother::Private
{
    Private(int historySize)
        : distances(historySize),
          lastSpeed(0)
    {
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
    qreal lastTime;
    qreal lastSpeed;
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
    qreal dist = kisDistance(pt, m_d->lastPoint);

    if (m_d->lastPoint.isNull()) {
        m_d->lastPoint = pt;
        m_d->lastTime = timestamp;
        m_d->lastSpeed = 0.0;
        return m_d->lastSpeed;
    }

    m_d->distances.push_back(Private::DistancePoint(dist, timestamp - m_d->lastTime));
    m_d->lastPoint = pt;
    m_d->lastTime = timestamp;


    Private::DistanceBuffer::const_reverse_iterator it = m_d->distances.rbegin();
    Private::DistanceBuffer::const_reverse_iterator end = m_d->distances.rend();

    qreal totalDistance = 0.0;
    qreal totalTime = 0.0;
    int itemsSearched = 0;

    for (; it != end; ++it) {
        itemsSearched++;
        if (itemsSearched > MAX_SAMPLE_COUNT || it->time > MAX_TIME_DIFF) {
            break;
        }
        totalDistance += it->distance;
        if (it->time > 0) {
            totalTime += it->time;
        } else { // workaround for Windows event queue
            if (totalTime == 0) {
                if (m_d->lastSpeed > 0.0001) {
                    // we assume the cursor is moving at the same speed as lastSpeed.
                    totalTime += it->distance/m_d->lastSpeed;
                } else {
                    // No information to proceed so we assume it took 7ms
                    totalTime += 7;
                }
            } else {
                totalTime += totalDistance/totalTime;
            }
        }
    }

    if (totalTime == 0) {
        m_d->lastSpeed = 0.0;
    } else {
        m_d->lastSpeed = totalDistance / totalTime;
    }

    return m_d->lastSpeed;
}

void KisSpeedSmoother::clear()
{
    m_d->distances.clear();
    m_d->distances.push_back(Private::DistancePoint(0.0, 0.0));
    m_d->lastPoint = QPointF();
    m_d->lastSpeed = 0.0;
}
