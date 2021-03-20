/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Fela Winkelmolen <fela.kde@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KarbonCalligraphicShape.h"

#include <KoPathPoint.h>

#include <KoParameterShape_p.h>
#include "KarbonSimplifyPath.h"
#include <KoCurveFit.h>
#include <KoColorBackground.h>

#include <QDebug>
#include <QColor>
#include <QPainterPath>

#include <cmath>
#include <cstdlib>

#undef M_PI
const qreal M_PI = 3.1415927;

struct KarbonCalligraphicShape::Private : public QSharedData
{
    Private(qreal _caps)
        : lastWasFlip(false),
          caps(_caps)

    {
    }

    Private(const Private &rhs) = default;

    bool lastWasFlip;
    qreal caps = 0.0;
    // the actual data then determines it's shape (guide path + data for points)
    QList<KarbonCalligraphicPoint> points;
};

KarbonCalligraphicShape::KarbonCalligraphicShape(qreal caps)
    : s(new Private(caps))
{
    setShapeId(KoPathShapeId);
    setFillRule(Qt::WindingFill);
    setBackground(QSharedPointer<KoShapeBackground>(new KoColorBackground(QColor(Qt::black))));
    setStroke(KoShapeStrokeModelSP());
}

KarbonCalligraphicShape::KarbonCalligraphicShape(const KarbonCalligraphicShape &rhs)
    : KoParameterShape(rhs),
      s(rhs.s)
{
}

KarbonCalligraphicShape::~KarbonCalligraphicShape()
{
}

KoShape *KarbonCalligraphicShape::cloneShape() const
{
    return new KarbonCalligraphicShape(*this);
}

void KarbonCalligraphicShape::appendPoint(const QPointF &point, qreal angle, qreal width)
{
    // convert the point from canvas to shape coordinates
    QPointF p = point - position();
    KarbonCalligraphicPoint calligraphicPoint(p, angle, width);

    QList<QPointF> handles = this->handles();
    handles.append(p);
    setHandles(handles);
    s->points.append(calligraphicPoint);
    appendPointToPath(calligraphicPoint);

    // make the angle of the first point more in line with the actual
    // direction
    if (s->points.count() == 4) {
        s->points[0].setAngle(angle);
        s->points[1].setAngle(angle);
        s->points[2].setAngle(angle);
    }

    normalize();
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
            s->lastWasFlip = true;
        }

        if (s->lastWasFlip) {
            int index = pointCount() / 2;
            // find the previous two points
            KoPathPoint *prev1 = pointByIndex(KoPathPointIndex(0, index - 2));
            KoPathPoint *prev2 = pointByIndex(KoPathPointIndex(0, index + 1));

            prev1->removeControlPoint1();
            prev1->removeControlPoint2();
            prev2->removeControlPoint1();
            prev2->removeControlPoint2();

            if (!flip) {
                s->lastWasFlip = false;
            }
        }
    }

    // add initial cap if it's the fourth added point
    // this code is here because this function is called from different places
    // pointCount() == 8 may causes crashes because it doesn't take possible
    // flips into account

    if (s->points.count() >= 4 && p == s->points[3]) {
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
    // calculate two times the area of the triangle formed by the points given
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

    for (int i = 0; i < s->points.size(); ++i) {
        s->points[i].setPoint(matrix.map(s->points[i].point()));
    }

    return offset;
}

void KarbonCalligraphicShape::moveHandleAction(int handleId,
        const QPointF &point,
        Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    s->points[handleId].setPoint(point);
}

void KarbonCalligraphicShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);

    QPointF pos = position();

    // remove all points
    clear();
    setPosition(QPoint(0, 0));

    Q_FOREACH (const KarbonCalligraphicPoint &p, s->points) {
        appendPointToPath(p);
    }

    QList<QPointF> handles;
    Q_FOREACH (const KarbonCalligraphicPoint &p, s->points) {
        handles.append(p.point());
    }
    setHandles(handles);

    setPosition(pos);
    normalize();
}

void KarbonCalligraphicShape::simplifyPath()
{
    if (s->points.count() < 2) {
        return;
    }

    close();

    // add final cap
    addCap(s->points.count() - 2, s->points.count() - 1, pointCount() / 2);

    // TODO: the error should be proportional to the width
    //       and it shouldn't be a magic number
    karbonSimplifyPath(this, 0.3);
}

void KarbonCalligraphicShape::addCap(int index1, int index2, int pointIndex, bool inverted)
{
    QPointF p1 = s->points[index1].point();
    QPointF p2 = s->points[index2].point();

    // TODO: review why spikes can appear with a lower limit
    QPointF delta = p2 - p1;
    if (delta.manhattanLength() < 1.0) {
        return;
    }

    QPointF direction = QLineF(QPointF(0, 0), delta).unitVector().p2();
    qreal width = s->points[index2].width();
    QPointF p = p2 + direction * s->caps * width;

    KoPathPoint *newPoint = new KoPathPoint(this, p);

    qreal angle = s->points[index2].angle();
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
    if (s->points.count() < 3) {
        return;
    }

    QList<QPointF> points;
    Q_FOREACH (const KarbonCalligraphicPoint &p, s->points) {
        points.append(p.point());
    }

    // cumulative data used to determine if the point can be removed
    qreal widthChange = 0;
    qreal directionChange = 0;
    QList<KarbonCalligraphicPoint>::iterator i = s->points.begin() + 2;

    while (i != std::prev(s->points.end())) {
        QPointF point = i->point();

        qreal width = i->width();
        qreal prevWidth = std::prev(i)->width();
        qreal widthDiff = width - prevWidth;
        widthDiff /= qMax(width, prevWidth);

        qreal directionDiff = 0;
        if (std::next(i) != s->points.end()) {
            QPointF prev = std::prev(i)->point();
            QPointF next = std::next(i)->point();

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
            i = s->points.erase(i);
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
