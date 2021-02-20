/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_snap_line_strategy.h"

#include <QPainterPath>
#include "kis_global.h"

struct KisSnapLineStrategy::Private
{
    QList<qreal> horizontalLines;
    QList<qreal> verticalLines;
};

KisSnapLineStrategy::KisSnapLineStrategy(KoSnapGuide::Strategy type)
    : KoSnapStrategy(type),
      m_d(new Private)
{
}

KisSnapLineStrategy::~KisSnapLineStrategy()
{
}

bool KisSnapLineStrategy::snap(const QPointF &mousePosition, KoSnapProxy *proxy, qreal maxSnapDistance)
{
    Q_UNUSED(proxy);

    QPointF snappedPoint = mousePosition;
    qreal minXDistance = std::numeric_limits<qreal>::max();
    qreal minYDistance = std::numeric_limits<qreal>::max();

    Q_FOREACH (qreal line, m_d->horizontalLines) {
        const qreal dist = qAbs(mousePosition.y() - line);

        if (dist < maxSnapDistance && dist < minYDistance) {
            minYDistance = dist;
            snappedPoint.ry() = line;
        }
    }

    Q_FOREACH (qreal line, m_d->verticalLines) {
        const qreal dist = qAbs(mousePosition.x() - line);

        if (dist < maxSnapDistance && dist < minXDistance) {
            minXDistance = dist;
            snappedPoint.rx() = line;
        }
    }

    if (kisDistance(snappedPoint, mousePosition) > maxSnapDistance) {
        if (minXDistance < minYDistance) {
            snappedPoint.ry() = mousePosition.y();
        } else {
            snappedPoint.rx() = mousePosition.x();
        }
    }

    setSnappedPosition(snappedPoint);
    return
        minXDistance < std::numeric_limits<qreal>::max() ||
        minYDistance < std::numeric_limits<qreal>::max();
}

QPainterPath KisSnapLineStrategy::decoration(const KoViewConverter &converter) const
{
    Q_UNUSED(converter);
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

