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

#include <KoParameterShape_p.h>
#include "KarbonSimplifyPath.h"
#include <KoCurveFit.h>
#include <KoColorBackground.h>
#include <KoXmlNS.h>
#include <SvgSavingContext.h>
#include <SvgLoadingContext.h>
#include <SvgUtil.h>
#include <SvgStyleWriter.h>
#include <KoXmlWriter.h>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>
#include <QColor>

#include <cmath>
#include <QtMath>
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
    setStroke(KoShapeStrokeModelSP());
}

KarbonCalligraphicShape::KarbonCalligraphicShape(const KarbonCalligraphicShape &rhs)
    : KoParameterShape(new KoParameterShapePrivate(*rhs.d_func(), this)),
      m_points(rhs.m_points),
      m_lastWasFlip(rhs.m_lastWasFlip),
      m_caps(rhs.m_caps)
{
}

KarbonCalligraphicShape::~KarbonCalligraphicShape()
{
}
KoShape *KarbonCalligraphicShape::cloneShape() const
{
    return new KarbonCalligraphicShape(*this);
}

void KarbonCalligraphicShape::appendPoint(const QPointF &p1, qreal angle, qreal width)
{
    // convert the point from canvas to shape coordinates
    QPointF p = p1 - position();
    KarbonCalligraphicPoint *calligraphicPoint =
            new KarbonCalligraphicPoint(p, angle, width);

    QList<QPointF> handles = this->handles();
    handles.append(p);
    setHandles(handles);
    m_points.append(calligraphicPoint);
    appendPointToPath(m_points.count()-1);

    if (m_points.count() == 4) {
        m_points[0]->setAngle(angle);
        m_points[1]->setAngle(angle);
        m_points[2]->setAngle(angle);
    }
}

void KarbonCalligraphicShape::appendPointToPath(int indexPoint)
{
    if (indexPoint<1) return;
    KarbonCalligraphicPoint *pm1 =m_points.at(indexPoint-1);
    KarbonCalligraphicPoint *p =m_points.at(indexPoint);


    qreal dx = std::cos(pm1->angle()) * pm1->width();
    qreal dy = std::sin(pm1->angle()) * pm1->width();
    qreal dx2 = std::cos(p->angle()) * p->width();
    qreal dy2 = std::sin(p->angle()) * p->width();

    // find the outline points
    QPointF p1 = pm1->point() - QPointF(dx / 2, dy / 2);
    QPointF p2 = p->point() - QPointF(dx2 / 2, dy2 / 2);
    QPointF p3 = p->point() + QPointF(dx2 / 2, dy2 / 2);
    QPointF p4 = pm1->point() + QPointF(dx / 2, dy / 2);
    QLineF line = QLineF(p1, p2);
    qreal angle = QLineF(pm1->point(), p->point()).angle();
    if (indexPoint>2) {
        angle = QLineF(m_points.at(indexPoint-2)->point(),p->point()).angle();
    }
    line.setLength(line.length()*0.33);
    line.setAngle(angle);
    QPointF c1 = line.p2();
    line = QLineF(p2, p1);
    line.setLength(line.length()*0.33);
    QPointF c2 = line.p2();
    line = QLineF(p3, p4);
    line.setLength(line.length()*0.33);
    QPointF c3 = line.p2();
    line = QLineF(p4, p3);
    line.setLength(line.length()*0.33);
    line.setAngle(angle);
    QPointF c4 = line.p2();



    if (ccw(p1, p2, p3)>0) {
        moveTo(p4);
        curveTo((c4), (c3), p3);
        lineTo(p2);
        curveTo((c2), (c1), p1);
        close();
    } else {
        moveTo(p1);
        curveTo((c1), (c2), p2);
        lineTo(p3);
        curveTo((c3), (c4), p4);
        close();
    }
    //smoothLastPoints();


    normalize();
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
    int index = subpathPointCount(qMax(subpathCount()-1, 0)) / 2;
    qDebug()<<"subpath"<<subpathPointCount(qMax(subpathCount()-1, 0))<<"index"<<index;
    smoothPoint(index - 1, qMax(subpathCount()-1, 0));
    smoothPoint(index + 2, qMax(subpathCount()-1, 0));
}

void KarbonCalligraphicShape::smoothPoint(const int index, const int subPathIndex)
{
    if (subpathPointCount(subPathIndex) <= index + 2) {
        return;
    } else if (index < 1) {
        return;
    }

    const KoPathPointIndex PREV(subPathIndex, index - 1);
    const KoPathPointIndex INDEX(subPathIndex, index);
    const KoPathPointIndex NEXT(subPathIndex, index + 1);

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
    /**
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
    **/


    return this->boundingRect();
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
    QTransform matrix(resizeMatrix(newSize));
    for (int i = 0; i < m_points.size(); ++i) {
        m_points[i]->setPoint(matrix.map(m_points[i]->point()));
        QPointF widthDummy = matrix.map(QPointF(m_points[i]->width(), m_points[i]->width()));
        m_points[i]->setWidth(widthDummy.x());
    }
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
    m_lastOffset = offset;
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
    // remove all points
    clear();
    //KarbonCalligraphicPoint *pLast = m_points.at(0);
    for (int i=0; i<m_points.count();i++) {
        appendPointToPath(i);
    }
    simplifyPath();

    QList<QPointF> handles;
    Q_FOREACH (KarbonCalligraphicPoint *p, m_points) {
        handles.append(p->point());
    }
    setHandles(handles);
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
    //karbonSimplifyPath(this, 0.3);
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
    qreal capSize = m_caps;
    QPointF p = p2 + direction * capSize * width;

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

bool KarbonCalligraphicShape::saveSvg(SvgSavingContext &context)
{
    context.shapeWriter().startElement("path");
    context.shapeWriter().addAttribute("krita:type", "calligraphic-stroke");
    context.shapeWriter().addAttribute("id", context.getID(this));
    context.shapeWriter().addAttribute("transform", SvgUtil::transformToString(transformation()));
    context.shapeWriter().addAttribute("d", this->toString(context.userSpaceTransform()));
    SvgStyleWriter::saveSvgStyle(this, context);
    QDomDocument doc= QDomDocument();
    QDomElement baseNode = doc.createElement("krita:calligraphic-stroke-data");
    baseNode.setAttribute("xmlns", KoXmlNS::krita);
    Q_FOREACH (KarbonCalligraphicPoint *p, m_points) {
        QDomElement infoElt = doc.createElement("point");
        infoElt.setAttribute("x", p->point().x());
        infoElt.setAttribute("y", p->point().x());
        infoElt.setAttribute("width", p->width());
        infoElt.setAttribute("angle", p->angle());
        baseNode.appendChild(infoElt);
    }
    doc.appendChild(baseNode);
    context.shapeWriter().addCompleteElement(doc.toString().toUtf8());
    context.shapeWriter().endElement();
    return true;
}

bool KarbonCalligraphicShape::loadSvg(const KoXmlElement &element, SvgLoadingContext &context)
{
    Q_UNUSED(context);

    const QString extendedNamespace = element.attribute("krita:type");

    if (element.tagName() == "path" && !extendedNamespace.isEmpty()) {

        QDomDocument doc = QDomDocument();
        KoXml::asQDomElement(doc, element);
        QDomElement root = doc.firstChildElement("path").firstChildElement("krita:calligraphic-stroke-data");

        QDomElement infoElt = root.firstChildElement("point");
        while (!infoElt.isNull()) {
            QPointF pos = QPointF();
            pos.setX(infoElt.attribute("x").toFloat());
            pos.setY(infoElt.attribute("y").toFloat());
            qreal width = infoElt.attribute("width").toFloat();
            qreal angle = infoElt.attribute("angle").toFloat();
            m_points.append(new KarbonCalligraphicPoint(pos, angle, width));
            infoElt = infoElt.nextSiblingElement("point");
        }
        return true;
    }
    return false;
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
        qreal prevWidth = (*(i - 1))->angle();
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
            //(*i)->paintInfo();            delete *i;
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
