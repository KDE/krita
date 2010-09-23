/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPathPoint.h"
#include "KoPathShape.h"
#include "KoPointGroup.h"

#include <KDebug>
#include <QtGui/QPainter>
#include <QPointF>

#include <math.h>

#ifndef QT_NO_DEBUG
#include <qnumeric.h> // for qIsNaN
static bool qIsNaNPoint(const QPointF &p) {
    return qIsNaN(p.x()) || qIsNaN(p.y());
}
#endif

class KoPathPoint::Private
{
public:
    Private()
            : shape(0), properties(Normal), pointGroup(0)
            , activeControlPoint1(false), activeControlPoint2(false) {}
    KoPathShape * shape;
    QPointF point;
    QPointF controlPoint1;
    QPointF controlPoint2;
    PointProperties properties;
    KoPointGroup * pointGroup;
    bool activeControlPoint1;
    bool activeControlPoint2;
};

KoPathPoint::KoPathPoint(const KoPathPoint &pathPoint)
        : d(new Private())
{
    d->shape = pathPoint.d->shape;
    d->point = pathPoint.d->point;
    d->controlPoint1 = pathPoint.d->controlPoint1;
    d->controlPoint2 = pathPoint.d->controlPoint2;
    d->properties = pathPoint.d->properties;
    d->activeControlPoint1 = pathPoint.d->activeControlPoint1;
    d->activeControlPoint2 = pathPoint.d->activeControlPoint2;
}

KoPathPoint::KoPathPoint()
        : d(new Private())
{
}

KoPathPoint::KoPathPoint(KoPathShape * path, const QPointF &point, PointProperties properties)
        : d(new Private())
{
    d->shape = path;
    d->point = point;
    d->controlPoint1 = point;
    d->controlPoint2 = point;
    d->properties = properties;
}

KoPathPoint::~KoPathPoint()
{
    delete d;
}

KoPathPoint &KoPathPoint::operator=(const KoPathPoint &rhs)
{
    if (this == &rhs)
        return (*this);

    d->shape = rhs.d->shape;
    d->point = rhs.d->point;
    d->controlPoint1 = rhs.d->controlPoint1;
    d->controlPoint2 = rhs.d->controlPoint2;
    d->properties = rhs.d->properties;
    //d->pointGroup = rhs.d->pointGroup;
    d->activeControlPoint1 = rhs.d->activeControlPoint1;
    d->activeControlPoint2 = rhs.d->activeControlPoint2;

    return (*this);
}

bool KoPathPoint::operator == (const KoPathPoint &rhs) const
{
    if (d->point != rhs.d->point)
        return false;
    if (d->controlPoint1 != rhs.d->controlPoint1)
        return false;
    if (d->controlPoint2 != rhs.d->controlPoint2)
        return false;
    if (d->properties != rhs.d->properties)
        return false;
    if (d->activeControlPoint1 != rhs.d->activeControlPoint1)
        return false;
    if (d->activeControlPoint2 != rhs.d->activeControlPoint2)
        return false;
    return true;
}

void KoPathPoint::setPoint(const QPointF &point)
{
    d->point = point;
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::setControlPoint1(const QPointF &point)
{
    Q_ASSERT(!qIsNaNPoint(point));
    d->controlPoint1 = point;
    d->activeControlPoint1 = true;
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::setControlPoint2(const QPointF &point)
{
    Q_ASSERT(!qIsNaNPoint(point));
    d->controlPoint2 = point;
    d->activeControlPoint2 = true;
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::removeControlPoint1()
{
    d->activeControlPoint1 = false;
    d->properties &= ~IsSmooth;
    d->properties &= ~IsSymmetric;
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::removeControlPoint2()
{
    d->activeControlPoint2 = false;
    d->properties &= ~IsSmooth;
    d->properties &= ~IsSymmetric;
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::setProperties(PointProperties properties)
{
    d->properties = properties;
    // CloseSubpath only allowed with StartSubpath or StopSubpath
    if ((d->properties & StartSubpath) == 0 && (d->properties & StopSubpath) == 0)
        d->properties &= ~CloseSubpath;

    if (! activeControlPoint1() || ! activeControlPoint2()) {
        // strip smooth and symmetric flags if point has not two control points
        d->properties &= ~IsSmooth;
        d->properties &= ~IsSymmetric;
    }

    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::setProperty(PointProperty property)
{
    switch (property) {
    case StartSubpath:
    case StopSubpath:
    case CloseSubpath:
        // nothing special to do here
        break;
    case IsSmooth:
        d->properties &= ~IsSymmetric;
        break;
    case IsSymmetric:
        d->properties &= ~IsSmooth;
        break;
    default: return;
    }

    d->properties |= property;

    if (! activeControlPoint1() || ! activeControlPoint2()) {
        // strip smooth and symmetric flags if point has not two control points
        d->properties &= ~IsSymmetric;
        d->properties &= ~IsSmooth;
    }
}

void KoPathPoint::unsetProperty(PointProperty property)
{
    switch (property) {
    case StartSubpath:
        if (d->properties & StartSubpath && (d->properties & StopSubpath) == 0)
            d->properties &= ~CloseSubpath;
        break;
    case StopSubpath:
        if (d->properties & StopSubpath && (d->properties & StartSubpath) == 0)
            d->properties &= ~CloseSubpath;
        break;
    case CloseSubpath:
        if (d->properties & StartSubpath || d->properties & StopSubpath) {
            d->properties &= ~IsSmooth;
            d->properties &= ~IsSymmetric;
        }
        break;
    case IsSmooth:
    case IsSymmetric:
        // no others depend on these
        break;
    default: return;
    }
    d->properties &= ~property;
}

bool KoPathPoint::activeControlPoint1() const
{
    // only start point on closed subpaths can have a controlPoint1
    if ((d->properties & StartSubpath) && (d->properties & CloseSubpath) == 0)
        return false;

    return d->activeControlPoint1;
}

bool KoPathPoint::activeControlPoint2() const
{
    // only end point on closed subpaths can have a controlPoint2
    if ((d->properties & StopSubpath) && (d->properties & CloseSubpath) == 0)
        return false;

    return d->activeControlPoint2;
}

void KoPathPoint::map(const QTransform &matrix, bool mapGroup)
{
    if (d->pointGroup && mapGroup) {
        d->pointGroup->map(matrix);
    } else {
        d->point = matrix.map(d->point);
        d->controlPoint1 = matrix.map(d->controlPoint1);
        d->controlPoint2 = matrix.map(d->controlPoint2);
    }
    if (d->shape)
        d->shape->notifyChanged();
}

void KoPathPoint::paint(QPainter &painter, int handleRadius, PointTypes types, bool active)
{
    QRectF handle(-handleRadius, -handleRadius, 2*handleRadius, 2*handleRadius);

    bool drawControlPoint1 = types & ControlPoint1 && (!active || activeControlPoint1());
    bool drawControlPoint2 = types & ControlPoint2 && (!active || activeControlPoint2());

    // draw lines at the bottom
    if (drawControlPoint2)
        painter.drawLine(point(), controlPoint2());

    if (drawControlPoint1)
        painter.drawLine(point(), controlPoint1());


    QTransform worldMatrix = painter.worldTransform();

    painter.setWorldTransform(QTransform());

    // the point is lowest
    if (types & Node) {
        if (properties() & IsSmooth)
            painter.drawRect(handle.translated(worldMatrix.map(point())));
        else if (properties() & IsSymmetric) {
            QTransform matrix;
            matrix.rotate(45.0);
            QPolygonF poly(handle);
            poly = matrix.map(poly);
            poly.translate(worldMatrix.map(point()));
            painter.drawPolygon(poly);
        } else
            painter.drawEllipse(handle.translated(worldMatrix.map(point())));
    }

    // then comes control point 2
    if (drawControlPoint2)
        painter.drawEllipse(handle.translated(worldMatrix.map(controlPoint2())));

    // then comes control point 1
    if (drawControlPoint1)
        painter.drawEllipse(handle.translated(worldMatrix.map(controlPoint1())));

    painter.setWorldTransform(worldMatrix);
}

void KoPathPoint::setParent(KoPathShape* parent)
{
    // don't set to zero
    //Q_ASSERT(parent);
    d->shape = parent;
}

QRectF KoPathPoint::boundingRect(bool active) const
{
    QRectF rect(d->point, QSize(1, 1));
    if (!active && activeControlPoint1()) {
        QRectF r1(d->point, QSize(1, 1));
        r1.setBottomRight(d->controlPoint1);
        rect = rect.unite(r1);
    }
    if (!active && activeControlPoint2()) {
        QRectF r2(d->point, QSize(1, 1));
        r2.setBottomRight(d->controlPoint2);
        rect = rect.unite(r2);
    }
    if (d->shape)
        return d->shape->shapeToDocument(rect);
    else
        return rect;
}

void KoPathPoint::reverse()
{
    qSwap(d->controlPoint1, d->controlPoint2);
    qSwap(d->activeControlPoint1, d->activeControlPoint2);
    PointProperties newProps = Normal;
    newProps |= d->properties & IsSmooth;
    newProps |= d->properties & IsSymmetric;
    newProps |= d->properties & StartSubpath;
    newProps |= d->properties & StopSubpath;
    newProps |= d->properties & CloseSubpath;
    d->properties = newProps;
}

bool KoPathPoint::isSmooth(KoPathPoint * prev, KoPathPoint * next) const
{
    QPointF t1, t2;

    if (activeControlPoint1()) {
        t1 = point() - controlPoint1();
    } else {
        // we need the previous path point but there is none provided
        if (! prev)
            return false;
        if (prev->activeControlPoint2())
            t1 = point() - prev->controlPoint2();
        else
            t1 = point() - prev->point();
    }

    if (activeControlPoint2()) {
        t2 = controlPoint2() - point();
    } else {
        // we need the next path point but there is none provided
        if (! next)
            return false;
        if (next->activeControlPoint1())
            t2 = next->controlPoint1() - point();
        else
            t2 = next->point() - point();
    }

    // normalize tangent vectors
    qreal l1 = sqrt(t1.x() * t1.x() + t1.y() * t1.y());
    qreal l2 = sqrt(t2.x() * t2.x() + t2.y() * t2.y());
    if (qFuzzyCompare(l1 + 1, qreal(1.0)) || qFuzzyCompare(l2 + 1, qreal(1.0)))
        return true;

    t1 /= l1;
    t2 /= l2;

    qreal scalar = t1.x() * t2.x() + t1.y() * t2.y();
    // tangents are parallel if t1*t2 = |t1|*|t2|
    return qFuzzyCompare(scalar, qreal(1.0));
}

void KoPathPoint::removeFromGroup()
{
    if (d->pointGroup)
        d->pointGroup->remove(this);
    d->pointGroup = 0;
}

void KoPathPoint::addToGroup(KoPointGroup *pointGroup)
{
    if (d->pointGroup && d->pointGroup != pointGroup) {
        //TODO error message as this should not happen
        removeFromGroup();
    }
    d->pointGroup = pointGroup;
}

KoPathPoint::PointProperties KoPathPoint::properties() const
{
    return d->properties;
}

QPointF KoPathPoint::point() const
{
    return d->point;
}

QPointF KoPathPoint::controlPoint1() const
{
    return d->controlPoint1;
}

QPointF KoPathPoint::controlPoint2() const
{
    return d->controlPoint2;
}

KoPathShape * KoPathPoint::parent() const
{
    return d->shape;
}

KoPointGroup * KoPathPoint::group()
{
    return d->pointGroup;
}
