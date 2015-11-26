/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

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

#include "KarbonCalligraphicShape.h"

#include <KoPathPoint.h>

#include "KarbonSimplifyPath.h"
#include <KoCurveFit.h>
#include <KoColorBackground.h>

#include <QDebug>
#include <QColor>

#include <cmath>
#include <cstdlib>

#undef M_PI
const qreal M_PI = 3.1415927;

KarbonCalligraphicShape::KarbonCalligraphicShape(qreal caps)
    : m_lastWasFlip(false)
    , m_caps(caps)
{
    setShapeId(KoPathShapeId);
    setFillRule(Qt::WindingFill);
    setBackground(QSharedPointer<KoShapeBackground>(new KoColorBackground(QColor(Qt::black))));
    setStroke(0);
}

KarbonCalligraphicShape::~KarbonCalligraphicShape()
{
}

void KarbonCalligraphicShape::appendPoint(const QPointF &point, qreal angle, qreal width)
{
    // convert the point from canvas to shape coordinates
    QPointF p = point - position();
    KarbonCalligraphicPoint *calligraphicPoint =
        new KarbonCalligraphicPoint(p, angle, width);

    QList<QPointF> handles = this->handles();
    handles.append(p);
    setHandles(handles);
    m_points.append(calligraphicPoint);
    appendPointToPath(*calligraphicPoint);

    // make the angle of the first point more in line with the actual
    // direction
    if (m_points.count() == 4) {
        m_points[0]->setAngle(angle);
        m_points[1]->setAngle(angle);
        m_points[2]->setAngle(angle);
    }
}

void KarbonCalligraphicShape::appendPointToPath(const KarbonCalligraphicPoint &p)
{
    qreal dx = std::cos(p.angle()) * p.width();
    qreal dy = std::sin(p.angle()) * p.width();

    // find the outline points
    QPointF p1 = p.point() - QPointF(dx / 2, dy / 2);
    QPointF p2 = p.point() + QPointF(dx / 2, dy / 2);

    if (pointCount() == 0) {
        moveTo(p1);
        lineTo(p2);
        normalize();
        return;
    }
    // pointCount > 0

    bool flip = (pointCount() >= 2) ? flipDetected(p1, p2) : false;

    // if there was a flip add additional points
    if (flip) {
        appendPointsToPathAux(p2, p1);
        if (pointCount() > 4) {
            smoothLastPoints();
        }
    }

    appendPointsToPathAux(p1, p2);

    if (pointCount() > 4) {
        smoothLastPoints();

        if (flip) {
            int index = pointCount() / 2;
            // find the last two points
            KoPathPoint *last1 = pointByIndex(KoPathPointIndex(0, index - 1));
            KoPathPoint *last2 = pointByIndex(KoPathPointIndex(0, index));

            last1->removeControlPoint1();
            last1->removeControlPoint2();
            last2->removeControlPoint1();
            last2->removeControlPoint2();
            m_lastWasFlip = true;
        }

        if (m_lastWasFlip) {
            int index = pointCount() / 2;
            // find the previous two points
            KoPathPoint *prev1 = pointByIndex(KoPathPointIndex(0, index - 2));
            KoPathPoint *prev2 = pointByIndex(KoPathPointIndex(0, index + 1));

            prev1->removeControlPoint1();
            prev1->removeControlPoint2();
            prev2->removeControlPoint1();
            prev2->removeControlPoint2();

            if (!flip) {
                m_lastWasFlip = false;
            }
        }
    }
    normalize();

    // add initial cap if it's the fourth added point
    // this code is here because this function is called from different places
    // pointCount() == 8 may causes crashes because it doesn't take possible
    // flips into account
    if (m_points.count() >= 4 && &p == m_points[3]) {
        addCap(3, 0, 0, true);
        // duplicate the last point to make the points remain "balanced"
        // needed to keep all indexes code (else I would need to change
        // everything in the code...)
        KoPathPoint *last = pointByIndex(KoPathPointIndex(0, pointCount() - 1));
        KoPathPoint *newPoint = new KoPathPoint(this, last->point());
        insertPoint(newPoint, KoPathPointIndex(0, pointCount()));
        close();
    }
}

void KarbonCalligraphicShape::appendPointsToPathAux(const QPointF &p1, const QPointF &p2)
{
    KoPathPoint *pathPoint1 = new KoPathPoint(this, p1);
    KoPathPoint *pathPoint2 = new KoPathPoint(this, p2);

    // calculate the index of the insertion position
    int index = pointCount() / 2;

    insertPoint(pathPoint2, KoPathPointIndex(0, index));
    insertPoint(pathPoint1, KoPathPointIndex(0, index));
}

void KarbonCalligraphicShape::smoothLastPoints()
{
    int index = pointCount() / 2;
    smoothPoint(index - 2);
    smoothPoint(index + 1);
}

void KarbonCalligraphicShape::smoothPoint(const int index)
{
    if (pointCount() < index + 2) {
        return;
    } else if (index < 1) {
        return;
    }

    const KoPathPointIndex PREV(0, index - 1);
    const KoPathPointIndex INDEX(0, index);
    const KoPathPointIndex NEXT(0, index + 1);

    QPointF prev = pointByIndex(PREV)->point();
    QPointF point = pointByIndex(INDEX)->point();
    QPointF next = pointByIndex(NEXT)->point();

    QPointF vector = next - prev;
    qreal dist = (QLineF(prev, next)).length();
    // normalize the vector (make it's size equal to 1)
    if (!qFuzzyCompare(dist + 1, 1)) {
        vector /= dist;
    }
    qreal mult = 0.35; // found by trial and error, might not be perfect...
    // distance of the control points from the point
    qreal dist1 = (QLineF(point, prev)).length() * mult;
    qreal dist2 = (QLineF(point, next)).length() * mult;
    QPointF vector1 = vector * dist1;
    QPointF vector2 = vector * dist2;
    QPointF controlPoint1 = point - vector1;
    QPointF controlPoint2 = point + vector2;

    pointByIndex(INDEX)->setControlPoint1(controlPoint1);
    pointByIndex(INDEX)->setControlPoint2(controlPoint2);
}

const QRectF KarbonCalligraphicShape::lastPieceBoundingRect()
{
    if (pointCount() < 6) {
        return QRectF();
    }

    int index = pointCount() / 2;

    QPointF p1 = pointByIndex(KoPathPointIndex(0, index - 3))->point();
    QPointF p2 = pointByIndex(KoPathPointIndex(0, index - 2))->point();
    QPointF p3 = pointByIndex(KoPathPointIndex(0, index - 1))->point();
    QPointF p4 = pointByIndex(KoPathPointIndex(0, index))->point();
    QPointF p5 = pointByIndex(KoPathPointIndex(0, index + 1))->point();
    QPointF p6 = pointByIndex(KoPathPointIndex(0, index + 2))->point();

    // TODO: also take the control points into account
    QPainterPath p;
    p.moveTo(p1);
    p.lineTo(p2);
    p.lineTo(p3);
    p.lineTo(p4);
    p.lineTo(p5);
    p.lineTo(p6);

    return p.boundingRect().translated(position());
}

bool KarbonCalligraphicShape::flipDetected(const QPointF &p1, const QPointF &p2)
{
    // detect the flip caused by the angle changing 180 degrees
    // thus detect the boundary crossing
    int index = pointCount() / 2;
    QPointF last1 = pointByIndex(KoPathPointIndex(0, index - 1))->point();
    QPointF last2 = pointByIndex(KoPathPointIndex(0, index))->point();

    int sum1 = std::abs(ccw(p1, p2, last1) + ccw(p1, last2, last1));
    int sum2 = std::abs(ccw(p2, p1, last2) + ccw(p2, last1, last2));
    // if there was a flip
    return sum1 < 2 && sum2 < 2;
}

int KarbonCalligraphicShape::ccw(const QPointF &p1, const QPointF &p2,const QPointF &p3)
{
    // calculate two times the area of the triangle fomed by the points given
    qreal area2 = (p2.x() - p1.x()) * (p3.y() - p1.y()) -
                  (p2.y() - p1.y()) * (p3.x() - p1.x());
    if (area2 > 0) {
        return +1; // the points are given in counterclockwise order
    } else if (area2 < 0) {
        return -1; // the points are given in clockwise order
    } else {
        return 0; // the points form a degenerate triangle
    }
}

void KarbonCalligraphicShape::setSize(const QSizeF &newSize)
{
    // QSizeF oldSize = size();
    // TODO: check
    KoParameterShape::setSize(newSize);
}

QPointF KarbonCalligraphicShape::normalize()
{
    QPointF offset(KoParameterShape::normalize());
    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());

    for (int i = 0; i < m_points.size(); ++i) {
        m_points[i]->setPoint(matrix.map(m_points[i]->point()));
    }

    return offset;
}

void KarbonCalligraphicShape::moveHandleAction(int handleId,
        const QPointF &point,
        Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    m_points[handleId]->setPoint(point);
}

void KarbonCalligraphicShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);

    QPointF pos = position();

    // remove all points
    clear();
    setPosition(QPoint(0, 0));

    Q_FOREACH (KarbonCalligraphicPoint *p, m_points) {
        appendPointToPath(*p);
    }

    simplifyPath();

    QList<QPointF> handles;
    Q_FOREACH (KarbonCalligraphicPoint *p, m_points) {
        handles.append(p->point());
    }
    setHandles(handles);

    setPosition(pos);
}

void KarbonCalligraphicShape::simplifyPath()
{
    if (m_points.count() < 2) {
        return;
    }

    close();

    // add final cap
    addCap(m_points.count() - 2, m_points.count() - 1, pointCount() / 2);

    // TODO: the error should be proportional to the width
    //       and it shouldn't be a magic number
    karbonSimplifyPath(this, 0.3);
}

void KarbonCalligraphicShape::addCap(int index1, int index2, int pointIndex, bool inverted)
{
    QPointF p1 = m_points[index1]->point();
    QPointF p2 = m_points[index2]->point();

    // TODO: review why spikes can appear with a lower limit
    QPointF delta = p2 - p1;
    if (delta.manhattanLength() < 1.0) {
        return;
    }

    QPointF direction = QLineF(QPointF(0, 0), delta).unitVector().p2();
    qreal width = m_points[index2]->width();
    QPointF p = p2 + direction * m_caps * width;

    KoPathPoint *newPoint = new KoPathPoint(this, p);

    qreal angle = m_points[index2]->angle();
    if (inverted) {
        angle += M_PI;
    }

    qreal dx = std::cos(angle) * width;
    qreal dy = std::sin(angle) * width;
    newPoint->setControlPoint1(QPointF(p.x() - dx / 2, p.y() - dy / 2));
    newPoint->setControlPoint2(QPointF(p.x() + dx / 2, p.y() + dy / 2));

    insertPoint(newPoint, KoPathPointIndex(0, pointIndex));
}

QString KarbonCalligraphicShape::pathShapeId() const
{
    return KarbonCalligraphicShapeId;
}

void KarbonCalligraphicShape::simplifyGuidePath()
{
    // do not attempt to simplify if there are too few points
    if (m_points.count() < 3) {
        return;
    }

    QList<QPointF> points;
    Q_FOREACH (KarbonCalligraphicPoint *p, m_points) {
        points.append(p->point());
    }

    // cumulative data used to determine if the point can be removed
    qreal widthChange = 0;
    qreal directionChange = 0;
    QList<KarbonCalligraphicPoint *>::iterator i = m_points.begin() + 2;

    while (i != m_points.end() - 1) {
        QPointF point = (*i)->point();

        qreal width = (*i)->width();
        qreal prevWidth = (*(i - 1))->width();
        qreal widthDiff = width - prevWidth;
        widthDiff /= qMax(width, prevWidth);

        qreal directionDiff = 0;
        if ((i + 1) != m_points.end()) {
            QPointF prev = (*(i - 1))->point();
            QPointF next = (*(i + 1))->point();

            directionDiff = QLineF(prev, point).angleTo(QLineF(point, next));
            if (directionDiff > 180) {
                directionDiff -= 360;
            }
        }

        if (directionChange * directionDiff >= 0 &&
                qAbs(directionChange + directionDiff) < 20 &&
                widthChange * widthDiff >= 0 &&
                qAbs(widthChange + widthDiff) < 0.1) {
            // deleted point
            delete *i;
            i = m_points.erase(i);
            directionChange += directionDiff;
            widthChange += widthDiff;
        } else {
            // keep point
            directionChange = 0;
            widthChange = 0;
            ++i;
        }
    }

    updatePath(QSizeF());
}
