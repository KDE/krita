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

#include "kis_snap_line_strategy.h"

#include <QPainterPath>


struct KisSnapLineStrategy::Private
{
    QList<qreal> horizontalLines;
    QList<qreal> verticalLines;
};

KisSnapLineStrategy::KisSnapLineStrategy()
    : KoSnapStrategy(KoSnapGuide::CustomSnapping),
      m_d(new Private)
{
}

KisSnapLineStrategy::~KisSnapLineStrategy()
{
}

bool KisSnapLineStrategy::snap(const QPointF &mousePosition, KoSnapProxy *proxy, qreal maxSnapDistance)
{
    QPointF snappedPoint = mousePosition;
    qreal minDistance = std::numeric_limits<qreal>::max();

    Q_FOREACH (qreal line, m_d->horizontalLines) {
        const qreal dist = qAbs(mousePosition.y() - line);

        if (dist < maxSnapDistance && dist < minDistance) {
            minDistance = dist;
            snappedPoint = QPointF(mousePosition.x(), line);
        }
    }

    Q_FOREACH (qreal line, m_d->verticalLines) {
        const qreal dist = qAbs(mousePosition.x() - line);

        if (dist < maxSnapDistance && dist < minDistance) {
            minDistance = dist;
            snappedPoint = QPointF(line, mousePosition.y());
        }
    }

    setSnappedPosition(snappedPoint);
    return minDistance < std::numeric_limits<qreal>::max();
}

QPainterPath KisSnapLineStrategy::decoration(const KoViewConverter &converter) const
{
    return QPainterPath();
}

void KisSnapLineStrategy::addLine(Qt::Orientation orientation, qreal pos)
{
    if (orientation == Qt::Horizontal) {
        m_d->horizontalLines << pos;
    } else {
        m_d->verticalLines << pos;
    }
}

void KisSnapLineStrategy::setHorizontalLines(const QList<qreal> &lines)
{
    m_d->horizontalLines = lines;
}

void KisSnapLineStrategy::setVerticalLines(const QList<qreal> &lines)
{
    m_d->verticalLines = lines;
}

