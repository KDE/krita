/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOCONNECTIONSHAPEPRIVATE_P
#define KOCONNECTIONSHAPEPRIVATE_P

#include <QSharedData>
#include "KoConnectionShape.h"

class KoConnectionShape::Private : public QSharedData
{
public:
    explicit Private();
    explicit Private(const Private &rhs);

    /// Checks if rays from given points into given directions intersect
    bool intersects(const QPointF &p1, const QPointF &d1, const QPointF &p2, const QPointF &d2, QPointF &isect);

    /// Returns perpendicular direction from given point p1 and direction d1 toward point p2
    QPointF perpendicularDirection(const QPointF &p1, const QPointF &d1, const QPointF &p2);

    qreal scalarProd(const QPointF &v1, const QPointF &v2) const;
    qreal crossProd(const QPointF &v1, const QPointF &v2) const;

    /// Returns if given handle is connected to a shape
    bool handleConnected(int handleId) const;

    QList<QPointF> path;

    KoShape *shape1;
    KoShape *shape2;
    int connectionPointId1;
    int connectionPointId2;
    KoConnectionShape::Type connectionType;
    bool forceUpdate;
    bool hasCustomPath;
};

#endif
