/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisSnapPointStrategy.h"

#include <QPainterPath>
#include "kis_global.h"

struct KisSnapPointStrategy::Private
{
    QList<QPointF> points;
};

KisSnapPointStrategy::KisSnapPointStrategy(KoSnapGuide::Strategy type)
    : KoSnapStrategy(type),
      m_d(new Private)
{
}

KisSnapPointStrategy::~KisSnapPointStrategy()
{
}

bool KisSnapPointStrategy::snap(const QPointF &mousePosition, KoSnapProxy *proxy, qreal maxSnapDistance)
{
    Q_UNUSED(proxy);

    QPointF snappedPoint = mousePosition;
    qreal minDistance = std::numeric_limits<qreal>::max();

    Q_FOREACH (const QPointF &pt, m_d->points) {
        const qreal dist = kisDistance(mousePosition, pt);

        if (dist < maxSnapDistance && dist < minDistance) {
            minDistance = dist;
            snappedPoint = pt;
        }
    }

    setSnappedPosition(snappedPoint);
    return minDistance < std::numeric_limits<qreal>::max();
}

QPainterPath KisSnapPointStrategy::decoration(const KoViewConverter &converter) const
{
    Q_UNUSED(converter);
    return QPainterPath();
}

void KisSnapPointStrategy::addPoint(const QPointF &pt)
{
    m_d->points << pt;
}

