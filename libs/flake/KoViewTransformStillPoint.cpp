/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDebug>
#include "KoViewTransformStillPoint.h"

KoViewTransformStillPoint::KoViewTransformStillPoint(const QPointF &docPoint, const QPointF &viewPoint)
    : std::pair<QPointF, QPointF>(docPoint, viewPoint)
{
}

KoViewTransformStillPoint::KoViewTransformStillPoint(const std::pair<QPointF, QPointF> &rhs)
    : std::pair<QPointF, QPointF>(rhs)
{
}

QPointF KoViewTransformStillPoint::docPoint() const {
    return first;
}

QPointF KoViewTransformStillPoint::viewPoint() const {
    return second;
}

QDebug operator<<(QDebug dbg, const KoViewTransformStillPoint &point)
{
    dbg.nospace() << "KoViewTransformStillPoint(docPoint: " << point.docPoint() << ", viewPoint: " << point.viewPoint() << ")";
    return dbg.space();
}