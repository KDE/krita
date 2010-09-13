/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include "EllipseShape.h"

#include <KoPathPoint.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoTextOnShapeContainer.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

#include <math.h>

EllipseShape::EllipseShape()
    :m_startAngle(0),
    m_endAngle(0),
    m_kindAngle(M_PI),
    m_type(Arc)
{
    QList<QPointF> handles;
    handles.push_back(QPointF(100, 50));
    handles.push_back(QPointF(100, 50));
    handles.push_back(QPointF(0, 50));
    setHandles(handles);
    QSizeF size(100, 100);
    m_radii = QPointF(size.width() / 2.0, size.height() / 2.0);
    m_center = QPointF(m_radii.x(), m_radii.y());
    updatePath(size);
}

EllipseShape::~EllipseShape()
{
}

void EllipseShape::saveOdf(KoShapeSavingContext &context) const
{
    if (isParametricShape()) {
        context.xmlWriter().startElement("draw:ellipse");
        saveOdfAttributes(context, OdfAllAttributes);

        switch (m_type) {
        case Arc:
            context.xmlWriter().addAttribute("draw:kind", sweepAngle()==360 ? "full" : "arc");
            break;
        case Pie:
            context.xmlWriter().addAttribute("draw:kind", "section");
            break;
        case Chord:
            context.xmlWriter().addAttribute("draw:kind", "cut");
            break;
        default:
            context.xmlWriter().addAttribute("draw:kind", "full");
        }
        if (m_type != Arc || sweepAngle() != 360) {
            context.xmlWriter().addAttribute("draw:start-angle", m_startAngle);
            context.xmlWriter().addAttribute("draw:end-angle", m_endAngle);
        }
        saveOdfCommonChildElements(context);
        if (parent())
            parent()->saveOdfChildElements(context);
        context.xmlWriter().endElement();
    } else {
        KoPathShape::saveOdf(context);
    }
}

bool EllipseShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    QSizeF size;

    bool radiusGiven = true;

    if (element.hasAttributeNS( KoXmlNS::svg, "rx") && element.hasAttributeNS(KoXmlNS::svg, "ry")) {
        qreal rx = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "rx"));
        qreal ry = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "ry"));
        size = QSizeF( 2*rx, 2*ry );
    } else if(element.hasAttributeNS(KoXmlNS::svg, "r")) {
        qreal r = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "r"));
        size = QSizeF(2*r, 2*r);
    } else {
        size.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
        size.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));
        radiusGiven = false;
    }
    setSize(size);

    QPointF pos;

    if (element.hasAttributeNS(KoXmlNS::svg, "cx") && element.hasAttributeNS(KoXmlNS::svg, "cy")) {
        qreal cx = KoUnit::parseValue(element.attributeNS( KoXmlNS::svg, "cx"));
        qreal cy = KoUnit::parseValue(element.attributeNS( KoXmlNS::svg, "cy"));
        pos = QPointF(cx - 0.5 * size.width(), cy - 0.5 * size.height());
    } else {
        pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
    }
    setPosition(pos);

    QString kind = element.attributeNS(KoXmlNS::draw, "kind", "full");
    if (kind == "section")
        setType(Pie);
    else if (kind == "cut")
        setType(Chord);
    else
        setType(Arc);

    setStartAngle(element.attributeNS(KoXmlNS::draw, "start-angle", "0").toDouble());
    setEndAngle(element.attributeNS(KoXmlNS::draw, "end-angle", "360").toDouble());
    if (!radiusGiven) {
        // is the size was given by width and height we have to reset the data as the size of the 
        // part of the cut/pie is given.
        setSize(size);
        setPosition(pos);
    }

    loadOdfAttributes(element, context, OdfMandatories | OdfTransformation | OdfAdditionalAttributes | OdfCommonChildElements);

    KoTextOnShapeContainer::tryWrapShape(this, element, context);

    return true;
}

void EllipseShape::setSize(const QSizeF &newSize)
{
    QTransform matrix(resizeMatrix(newSize));
    m_center = matrix.map(m_center);
    m_radii = matrix.map(m_radii);
    KoParameterShape::setSize(newSize);
}

QPointF EllipseShape::normalize()
{
    QPointF offset(KoParameterShape::normalize());
    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());
    m_center = matrix.map(m_center);
    return offset;
}

void EllipseShape::moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QPointF p(point);

    QPointF diff(m_center - point);
    diff.setX(-diff.x());
    qreal angle = 0;
    if (diff.x() == 0) {
        angle = (diff.y() < 0 ? 270 : 90 ) * M_PI / 180.0;
    } else {
        diff.setY(diff.y() * m_radii.x() / m_radii.y());
        angle = atan(diff.y() / diff.x ());
        if (angle < 0)
            angle = M_PI + angle;
        if (diff.y() < 0)
            angle += M_PI;
    }

    QList<QPointF> handles = this->handles();
    switch ( handleId ) {
    case 0:
        p = QPointF(m_center + QPointF(cos(angle) * m_radii.x(), -sin(angle) * m_radii.y()));
        m_startAngle = angle * 180.0 / M_PI;
        handles[handleId] = p;
        updateKindHandle();
        break;
    case 1:
        p = QPointF(m_center + QPointF(cos(angle) * m_radii.x(), -sin(angle) * m_radii.y()));
        m_endAngle = angle * 180.0 / M_PI;
        handles[handleId] = p;
        updateKindHandle();
        break;
    case 2: {
        QList<QPointF> kindHandlePositions;
        kindHandlePositions.push_back(QPointF(m_center + QPointF(cos(m_kindAngle) * m_radii.x(), -sin(m_kindAngle) * m_radii.y())));
        kindHandlePositions.push_back(m_center);
        kindHandlePositions.push_back((handles[0] + handles[1]) / 2.0);

        QPointF diff = m_center * 2.0;
        int handlePos = 0;
        for (int i = 0; i < kindHandlePositions.size(); ++i) {
            QPointF pointDiff(p - kindHandlePositions[i]);
            if (i == 0 || qAbs(pointDiff.x()) + qAbs(pointDiff.y()) < qAbs(diff.x()) + qAbs(diff.y())) {
                diff = pointDiff;
                handlePos = i;
            }
        }
        handles[handleId] = kindHandlePositions[handlePos];
        m_type = EllipseType(handlePos);
    }
    break;
    }
    setHandles(handles);
}

void EllipseShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);
    QPointF startpoint(handles()[0]);

    QPointF curvePoints[12];

    int pointCnt = arcToCurve(m_radii.x(), m_radii.y(), m_startAngle, sweepAngle() , startpoint, curvePoints);

    int curvePointCount = 1 + pointCnt / 3;
    int requiredPointCount = curvePointCount;
    if (m_type == Pie) {
        requiredPointCount++;
    } else if (m_type == Arc && m_startAngle == m_endAngle) {
        curvePointCount--;
        requiredPointCount--;
    }

    createPoints(requiredPointCount);

    KoSubpath &points = *m_subpaths[0];

    int curveIndex = 0;
    points[0]->setPoint(startpoint);
    points[0]->removeControlPoint1();
    points[0]->setProperty(KoPathPoint::StartSubpath);
    for (int i = 1; i < curvePointCount; ++i) {
        points[i-1]->setControlPoint2(curvePoints[curveIndex++]);
        points[i]->setControlPoint1(curvePoints[curveIndex++]);
        points[i]->setPoint(curvePoints[curveIndex++]);
        points[i]->removeControlPoint2();
    }

    if (m_type == Pie) {
        points[requiredPointCount-1]->setPoint(m_center);
        points[requiredPointCount-1]->removeControlPoint1();
        points[requiredPointCount-1]->removeControlPoint2();
    } else if (m_type == Arc && m_startAngle == m_endAngle) {
        points[curvePointCount-1]->setControlPoint2(curvePoints[curveIndex]);
        points[0]->setControlPoint1(curvePoints[++curveIndex]);
    }

    for (int i = 0; i < requiredPointCount; ++i) {
        points[i]->unsetProperty(KoPathPoint::StopSubpath);
        points[i]->unsetProperty(KoPathPoint::CloseSubpath);
    }
    m_subpaths[0]->last()->setProperty(KoPathPoint::StopSubpath);
    if (m_type == Arc && m_startAngle != m_endAngle) {
        m_subpaths[0]->first()->unsetProperty(KoPathPoint::CloseSubpath);
        m_subpaths[0]->last()->unsetProperty(KoPathPoint::CloseSubpath);
    } else {
        m_subpaths[0]->first()->setProperty(KoPathPoint::CloseSubpath);
        m_subpaths[0]->last()->setProperty(KoPathPoint::CloseSubpath);
    }

    normalize();
}

void EllipseShape::createPoints(int requiredPointCount)
{
    if (m_subpaths.count() != 1) {
        clear();
        m_subpaths.append(new KoSubpath());
    }
    int currentPointCount = m_subpaths[0]->count();
    if (currentPointCount > requiredPointCount) {
        for (int i = 0; i < currentPointCount-requiredPointCount; ++i) {
            delete m_subpaths[0]->front();
            m_subpaths[0]->pop_front();
        }
    } else if (requiredPointCount > currentPointCount) {
        for (int i = 0; i < requiredPointCount-currentPointCount; ++i) {
            m_subpaths[0]->append(new KoPathPoint(this, QPointF()));
        }
    }
}


void EllipseShape::updateKindHandle()
{
    m_kindAngle = (m_startAngle + m_endAngle) * M_PI / 360.0;
    if (m_startAngle > m_endAngle)
        m_kindAngle += M_PI;
    QList<QPointF> handles = this->handles();
    switch (m_type) {
    case Arc:
        handles[2] = m_center + QPointF(cos(m_kindAngle) * m_radii.x(), -sin(m_kindAngle) * m_radii.y());
        break;
    case Pie:
        handles[2] = m_center;
        break;
    case Chord:
        handles[2] = (handles[0] + handles[1]) / 2.0;
        break;
    }
    setHandles(handles);
}

void EllipseShape::updateAngleHandles()
{
    qreal startRadian = m_startAngle * M_PI / 180.0;
    qreal endRadian = m_endAngle * M_PI / 180.0;
    QList<QPointF> handles = this->handles();
    handles[0] = m_center + QPointF(cos(startRadian) * m_radii.x(), -sin(startRadian) * m_radii.y());
    handles[1] = m_center + QPointF(cos(endRadian) * m_radii.x(), -sin(endRadian) * m_radii.y());
    setHandles(handles);
}

qreal EllipseShape::sweepAngle() const
{
    qreal sAngle =  m_endAngle - m_startAngle;
    // treat also as full circle
    if (sAngle == 0 || sAngle == -360)
        sAngle = 360;
    if (m_startAngle > m_endAngle)
        sAngle = 360 - m_startAngle + m_endAngle;
    return sAngle;
}

void EllipseShape::setType(EllipseType type)
{
    m_type = type;
    updateKindHandle();
    updatePath(size());
}

EllipseShape::EllipseType EllipseShape::type() const
{
    return m_type;
}

void EllipseShape::setStartAngle(qreal angle)
{
    m_startAngle = angle;
    updateKindHandle();
    updateAngleHandles();
    updatePath(size());
}

qreal EllipseShape::startAngle() const
{
    return m_startAngle;
}

void EllipseShape::setEndAngle(qreal angle)
{
    m_endAngle = angle;
    updateKindHandle();
    updateAngleHandles();
    updatePath(size());
}

qreal EllipseShape::endAngle() const
{
    return m_endAngle;
}

QString EllipseShape::pathShapeId() const
{
    return EllipseShapeId;
}
