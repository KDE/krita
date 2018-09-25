/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
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

#include "RectangleShape.h"

#include <KoParameterShape_p.h>
#include <KoPathPoint.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <SvgSavingContext.h>
#include <SvgLoadingContext.h>
#include <SvgUtil.h>
#include <SvgStyleWriter.h>

RectangleShape::RectangleShape()
    : m_cornerRadiusX(0)
    , m_cornerRadiusY(0)
{
    QList<QPointF> handles;
    handles.push_back(QPointF(100, 0));
    handles.push_back(QPointF(100, 0));
    setHandles(handles);
    QSizeF size(100, 100);
    updatePath(size);
}

RectangleShape::RectangleShape(const RectangleShape &rhs)
    : KoParameterShape(new KoParameterShapePrivate(*rhs.d_func(), this)),
      m_cornerRadiusX(rhs.m_cornerRadiusX),
      m_cornerRadiusY(rhs.m_cornerRadiusY)
{
}

RectangleShape::~RectangleShape()
{
}

KoShape *RectangleShape::cloneShape() const
{
    return new RectangleShape(*this);
}

bool RectangleShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfMandatories | OdfGeometry | OdfAdditionalAttributes | OdfCommonChildElements);

    if (element.hasAttributeNS(KoXmlNS::svg, "rx") && element.hasAttributeNS(KoXmlNS::svg, "ry")) {
        qreal rx = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "rx", "0"));
        qreal ry = KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "ry", "0"));
        m_cornerRadiusX = rx / (0.5 * size().width()) * 100;
        m_cornerRadiusY = ry / (0.5 * size().height()) * 100;
    } else {
        QString cornerRadius = element.attributeNS(KoXmlNS::draw, "corner-radius", "");
        if (!cornerRadius.isEmpty()) {
            qreal radius = KoUnit::parseValue(cornerRadius);
            m_cornerRadiusX = qMin<qreal>(radius / (0.5 * size().width()) * 100, qreal(100));
            m_cornerRadiusY = qMin<qreal>(radius / (0.5 * size().height()) * 100, qreal(100));
        }
    }

    updatePath(size());
    updateHandles();

    loadOdfAttributes(element, context, OdfTransformation);
    loadText(element, context);

    return true;
}

void RectangleShape::saveOdf(KoShapeSavingContext &context) const
{
    if (isParametricShape()) {
        context.xmlWriter().startElement("draw:rect");
        saveOdfAttributes(context, OdfAllAttributes);
        if (m_cornerRadiusX > 0 && m_cornerRadiusY > 0) {
            context.xmlWriter().addAttribute("svg:rx", m_cornerRadiusX * (0.5 * size().width()) / 100.0);
            context.xmlWriter().addAttribute("svg:ry", m_cornerRadiusY * (0.5 * size().height()) / 100.0);
        }
        saveOdfCommonChildElements(context);
        saveText(context);
        context.xmlWriter().endElement();
    } else {
        KoPathShape::saveOdf(context);
    }
}

void RectangleShape::moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    QPointF p(point);

    qreal width2 = size().width() / 2.0;
    qreal height2 = size().height() / 2.0;
    switch (handleId) {
    case 0:
        if (p.x() < width2) {
            p.setX(width2);
        } else if (p.x() > size().width()) {
            p.setX(size().width());
        }
        p.setY(0);
        m_cornerRadiusX = (size().width() - p.x()) / width2 * 100.0;
        if (!(modifiers & Qt::ControlModifier)) {
            m_cornerRadiusY = (size().width() - p.x()) / height2 * 100.0;
        }
        break;
    case 1:
        if (p.y() < 0) {
            p.setY(0);
        } else if (p.y() > height2) {
            p.setY(height2);
        }
        p.setX(size().width());
        m_cornerRadiusY = p.y() / height2 * 100.0;
        if (!(modifiers & Qt::ControlModifier)) {
            m_cornerRadiusX = p.y() / width2 * 100.0;
        }
        break;
    }
    // this is needed otherwise undo/redo might not end in the same result
    if (100 - m_cornerRadiusX < 1e-10) {
        m_cornerRadiusX = 100;
    }
    if (100 - m_cornerRadiusY < 1e-10) {
        m_cornerRadiusY = 100;
    }

    updateHandles();
}

void RectangleShape::updateHandles()
{
    QList<QPointF> handles;
    handles.append(QPointF(size().width() - m_cornerRadiusX / 100.0 * 0.5 * size().width(), 0.0));
    handles.append(QPointF(size().width(), m_cornerRadiusY / 100.0 * 0.5 * size().height()));
    setHandles(handles);
}

void RectangleShape::updatePath(const QSizeF &size)
{
    Q_D(KoParameterShape);

    qreal rx = 0;
    qreal ry = 0;
    if (m_cornerRadiusX > 0 && m_cornerRadiusY > 0) {
        rx = size.width() / 200.0 * m_cornerRadiusX;
        ry = size.height() / 200.0 * m_cornerRadiusY;
    }

    qreal x2 = size.width() - rx;
    qreal y2 = size.height() - ry;

    QPointF curvePoints[12];

    int requiredCurvePointCount = 4;
    if (rx && m_cornerRadiusX < 100) {
        requiredCurvePointCount += 2;
    }
    if (ry && m_cornerRadiusY < 100) {
        requiredCurvePointCount += 2;
    }

    createPoints(requiredCurvePointCount);

    KoSubpath &points = *d->subpaths[0];

    int cp = 0;

    // first path starts and closes path
    points[cp]->setProperty(KoPathPoint::StartSubpath);
    points[cp]->setProperty(KoPathPoint::CloseSubpath);
    points[cp]->setPoint(QPointF(rx, 0));
    points[cp]->removeControlPoint1();
    points[cp]->removeControlPoint2();

    if (m_cornerRadiusX < 100 || m_cornerRadiusY == 0) {
        // end point of the top edge
        points[++cp]->setPoint(QPointF(x2, 0));
        points[cp]->removeControlPoint1();
        points[cp]->removeControlPoint2();
    }

    if (rx) {
        // the top right radius
        arcToCurve(rx, ry, 90, -90, points[cp]->point(), curvePoints);
        points[cp]->setControlPoint2(curvePoints[0]);
        points[++cp]->setControlPoint1(curvePoints[1]);
        points[cp]->setPoint(curvePoints[2]);
        points[cp]->removeControlPoint2();
    }

    if (m_cornerRadiusY < 100 || m_cornerRadiusX == 0) {
        // the right edge
        points[++cp]->setPoint(QPointF(size.width(), y2));
        points[cp]->removeControlPoint1();
        points[cp]->removeControlPoint2();
    }

    if (rx) {
        // the bottom right radius
        arcToCurve(rx, ry, 0, -90, points[cp]->point(), curvePoints);
        points[cp]->setControlPoint2(curvePoints[0]);
        points[++cp]->setControlPoint1(curvePoints[1]);
        points[cp]->setPoint(curvePoints[2]);
        points[cp]->removeControlPoint2();
    }

    if (m_cornerRadiusX < 100 || m_cornerRadiusY == 0) {
        // the bottom edge
        points[++cp]->setPoint(QPointF(rx, size.height()));
        points[cp]->removeControlPoint1();
        points[cp]->removeControlPoint2();
    }

    if (rx) {
        // the bottom left radius
        arcToCurve(rx, ry, 270, -90, points[cp]->point(), curvePoints);
        points[cp]->setControlPoint2(curvePoints[0]);
        points[++cp]->setControlPoint1(curvePoints[1]);
        points[cp]->setPoint(curvePoints[2]);
        points[cp]->removeControlPoint2();
    }

    if ((m_cornerRadiusY < 100 || m_cornerRadiusX == 0) && ry) {
        // the right edge
        points[++cp]->setPoint(QPointF(0, ry));
        points[cp]->removeControlPoint1();
        points[cp]->removeControlPoint2();
    }

    if (rx) {
        // the top left radius
        arcToCurve(rx, ry, 180, -90, points[cp]->point(), curvePoints);
        points[cp]->setControlPoint2(curvePoints[0]);
        points[0]->setControlPoint1(curvePoints[1]);
        points[0]->setPoint(curvePoints[2]);
    }

    // unset all stop/close path properties
    for (int i = 1; i < cp; ++i) {
        points[i]->unsetProperty(KoPathPoint::StopSubpath);
        points[i]->unsetProperty(KoPathPoint::CloseSubpath);
    }

    // last point stops and closes path
    points.last()->setProperty(KoPathPoint::StopSubpath);
    points.last()->setProperty(KoPathPoint::CloseSubpath);

    notifyPointsChanged();
}

void RectangleShape::createPoints(int requiredPointCount)
{
    Q_D(KoParameterShape);

    if (d->subpaths.count() != 1) {
        clear();
        d->subpaths.append(new KoSubpath());
    }
    int currentPointCount = d->subpaths[0]->count();
    if (currentPointCount > requiredPointCount) {
        for (int i = 0; i < currentPointCount - requiredPointCount; ++i) {
            delete d->subpaths[0]->front();
            d->subpaths[0]->pop_front();
        }
    } else if (requiredPointCount > currentPointCount) {
        for (int i = 0; i < requiredPointCount - currentPointCount; ++i) {
            d->subpaths[0]->append(new KoPathPoint(this, QPointF()));
        }
    }

    notifyPointsChanged();
}

qreal RectangleShape::cornerRadiusX() const
{
    return m_cornerRadiusX;
}

void RectangleShape::setCornerRadiusX(qreal radius)
{
    radius = qBound(0.0, radius, 100.0);
    m_cornerRadiusX = radius;
    updatePath(size());
    updateHandles();
}

qreal RectangleShape::cornerRadiusY() const
{
    return m_cornerRadiusY;
}

void RectangleShape::setCornerRadiusY(qreal radius)
{
    radius = qBound(0.0, radius, 100.0);
    m_cornerRadiusY = radius;
    updatePath(size());
    updateHandles();
}

QString RectangleShape::pathShapeId() const
{
    return RectangleShapeId;
}

bool RectangleShape::saveSvg(SvgSavingContext &context)
{
    // let basic path saiving code handle our saving
    if (!isParametricShape()) return false;

    context.shapeWriter().startElement("rect");
    context.shapeWriter().addAttribute("id", context.getID(this));
    SvgUtil::writeTransformAttributeLazy("transform", transformation(), context.shapeWriter());

    SvgStyleWriter::saveSvgStyle(this, context);

    const QSizeF size = this->size();
    context.shapeWriter().addAttribute("width", size.width());
    context.shapeWriter().addAttribute("height", size.height());

    double rx = cornerRadiusX();
    if (rx > 0.0) {
        context.shapeWriter().addAttribute("rx", 0.01 * rx * 0.5 * size.width());
    }
    double ry = cornerRadiusY();
    if (ry > 0.0) {
        context.shapeWriter().addAttribute("ry", 0.01 * ry * 0.5 * size.height());
    }

    context.shapeWriter().endElement();

    return true;
}

bool RectangleShape::loadSvg(const KoXmlElement &element, SvgLoadingContext &context)
{
    const qreal x = SvgUtil::parseUnitX(context.currentGC(), element.attribute("x"));
    const qreal y = SvgUtil::parseUnitY(context.currentGC(), element.attribute("y"));
    const qreal w = SvgUtil::parseUnitX(context.currentGC(), element.attribute("width"));
    const qreal h = SvgUtil::parseUnitY(context.currentGC(), element.attribute("height"));
    const QString rxStr = element.attribute("rx");
    const QString ryStr = element.attribute("ry");
    qreal rx = rxStr.isEmpty() ? 0.0 : SvgUtil::parseUnitX(context.currentGC(), rxStr);
    qreal ry = ryStr.isEmpty() ? 0.0 : SvgUtil::parseUnitY(context.currentGC(), ryStr);
    // if one radius is given but not the other, use the same value for both
    if (!rxStr.isEmpty() && ryStr.isEmpty()) {
        ry = rx;
    }
    if (rxStr.isEmpty() && !ryStr.isEmpty()) {
        rx = ry;
    }

    setSize(QSizeF(w, h));
    setPosition(QPointF(x, y));
    if (rx >= 0.0) {
        setCornerRadiusX(qMin(qreal(100.0), qreal(rx / (0.5 * w) * 100.0)));
    }
    if (ry >= 0.0) {
        setCornerRadiusY(qMin(qreal(100.0), qreal(ry / (0.5 * h) * 100.0)));
    }
    if (w == 0.0 || h == 0.0) {
        setVisible(false);
    }

    return true;
}
