/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSnapPointStrategy.h"

#include <QPainterPath>
#include <KoViewConverter.h>
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

    setSnappedPosition(snappedPoint, ToPoint);
    return minDistance < std::numeric_limits<qreal>::max();
}

QPainterPath KisSnapPointStrategy::decoration(const KoViewConverter &converter) const
{
    QRectF unzoomedRect = converter.viewToDocument(QRectF(0, 0, 11, 11));
    unzoomedRect.moveCenter(snappedPosition());
    QPainterPath decoration;
    decoration.addEllipse(unzoomedRect);
    return decoration;
}

void KisSnapPointStrategy::addPoint(const QPointF &pt)
{
    m_d->points << pt;
}

