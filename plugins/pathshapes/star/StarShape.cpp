/* This file is part of the KDE project
   Copyright (C) 2006-2009 Jan Hambrecht <jaham@gmx.net>
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

#include "StarShape.h"

#include <KoPathPoint.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include <math.h>

StarShape::StarShape()
: m_cornerCount(5)
, m_zoomX(1.0)
, m_zoomY(1.0)
, m_convex(false)
{
    m_radius[base] = 25.0;
    m_radius[tip] = 50.0;
    m_angles[base] = m_angles[tip] = defaultAngleRadian();
    m_roundness[base] = m_roundness[tip] = 0.0f;

    m_center = QPointF(50,50);
    updatePath(QSize(100,100));
}

StarShape::~StarShape()
{
}

void StarShape::setCornerCount(uint cornerCount)
{
    if (cornerCount >= 3) {
        double oldDefaultAngle = defaultAngleRadian();
        m_cornerCount = cornerCount;
        double newDefaultAngle = defaultAngleRadian();
        m_angles[base] += newDefaultAngle-oldDefaultAngle;
        m_angles[tip] += newDefaultAngle-oldDefaultAngle;

        updatePath(QSize());
    }
}

uint StarShape::cornerCount() const
{
    return m_cornerCount;
}

void StarShape::setBaseRadius(qreal baseRadius)
{
    m_radius[base] = fabs(baseRadius);
    updatePath(QSize());
}

qreal StarShape::baseRadius() const
{
    return m_radius[base];
}

void StarShape::setTipRadius(qreal tipRadius)
{
    m_radius[tip] = fabs(tipRadius);
    updatePath(QSize());
}

qreal StarShape::tipRadius() const
{
    return m_radius[tip];
}

void StarShape::setBaseRoundness(qreal baseRoundness)
{
    m_roundness[base] = baseRoundness;
    updatePath(QSize());
}

void StarShape::setTipRoundness(qreal tipRoundness)
{
    m_roundness[tip] = tipRoundness;
    updatePath(QSize());
}

void StarShape::setConvex(bool convex)
{
    m_convex = convex;
    updatePath(QSize());
}

bool StarShape::convex() const
{
    return m_convex;
}

QPointF StarShape::starCenter() const
{
    return m_center;
}

void StarShape::moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ShiftModifier) {
        QPointF handle = handles()[handleId];
        QPointF tangentVector = point - handle;
        qreal distance = sqrt(tangentVector.x()*tangentVector.x() + tangentVector.y()*tangentVector.y());
        QPointF radialVector = handle - m_center;
        // cross product to determine in which direction the user is dragging
        qreal moveDirection = radialVector.x()*tangentVector.y() - radialVector.y()*tangentVector.x();
        // make the roundness stick to zero if distance is under a certain value
        float snapDistance = 3.0;
        if (distance >= 0.0)
            distance = distance < snapDistance ? 0.0 : distance-snapDistance;
        else
            distance = distance > -snapDistance ? 0.0 : distance+snapDistance;
        // control changes roundness on both handles, else only the actual handle roundness is changed
        if (modifiers & Qt::ControlModifier)
            m_roundness[handleId] = moveDirection < 0.0f ? distance : -distance;
        else
            m_roundness[base] = m_roundness[tip] = moveDirection < 0.0f ? distance : -distance;
    }
    else {
        QPointF distVector = point - m_center;
        // unapply scaling
        distVector.rx() /= m_zoomX;
        distVector.ry() /= m_zoomY;
        m_radius[handleId] = sqrt(distVector.x()*distVector.x() + distVector.y()*distVector.y());

        qreal angle = atan2(distVector.y(), distVector.x());
        if (angle < 0.0)
            angle += 2.0*M_PI;
        qreal diffAngle = angle-m_angles[handleId];
        qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);
        if (handleId == tip) {
            m_angles[tip] += diffAngle-radianStep;
            m_angles[base] += diffAngle-radianStep;
        } else {
            // control make the base point move freely
            if (modifiers & Qt::ControlModifier)
                m_angles[base] += diffAngle-2*radianStep;
            else
                m_angles[base] = m_angles[tip];
        }
    }
}

void StarShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);
    qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);

    createPoints(m_convex ? m_cornerCount : 2*m_cornerCount);

    KoSubpath &points = *m_subpaths[0];

    uint index = 0;
    for (uint i = 0; i < 2*m_cornerCount; ++i) {
        uint cornerType = i % 2;
        if (cornerType == base && m_convex)
            continue;
        qreal radian = static_cast<qreal>((i+1)*radianStep) + m_angles[cornerType];
        QPointF cornerPoint = QPointF(m_zoomX * m_radius[cornerType] * cos(radian), m_zoomY * m_radius[cornerType] * sin(radian));

        points[index]->setPoint(m_center + cornerPoint);
        points[index]->unsetProperty(KoPathPoint::StopSubpath);
        points[index]->unsetProperty(KoPathPoint::CloseSubpath);
        if (m_roundness[cornerType] > 1e-10 || m_roundness[cornerType] < -1e-10) {
            // normalized cross product to compute tangential vector for handle point
            QPointF tangentVector(cornerPoint.y()/m_radius[cornerType], -cornerPoint.x()/m_radius[cornerType]);
            points[index]->setControlPoint2(points[index]->point() - m_roundness[cornerType] * tangentVector);
            points[index]->setControlPoint1(points[index]->point() + m_roundness[cornerType] * tangentVector);
        } else {
            points[index]->removeControlPoint1();
            points[index]->removeControlPoint2();
        }
        index++;
    }

    // first path starts and closes path
    points[0]->setProperty(KoPathPoint::StartSubpath);
    points[0]->setProperty(KoPathPoint::CloseSubpath);
    // last point stops and closes path
    points.last()->setProperty(KoPathPoint::StopSubpath);
    points.last()->setProperty(KoPathPoint::CloseSubpath);

    normalize();

    QList<QPointF> handles;
    handles.push_back(points.at(tip)->point());
    if (! m_convex)
        handles.push_back(points.at(base)->point());
    setHandles(handles);

    m_center = computeCenter();
}

void StarShape::createPoints(int requiredPointCount)
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

void StarShape::setSize(const QSizeF &newSize)
{
    QMatrix matrix(resizeMatrix(newSize));
    m_zoomX *= matrix.m11();
    m_zoomY *= matrix.m22();

    // this transforms the handles
    KoParameterShape::setSize(newSize);

    m_center = computeCenter();
}

QPointF StarShape::computeCenter() const
{
    KoSubpath &points = *m_subpaths[0];

    QPointF center(0, 0);
    for (uint i = 0; i < m_cornerCount; ++i) {
        if (m_convex)
            center += points[i]->point();
        else
            center += points[2*i]->point();
    }
    return center / static_cast<qreal>(m_cornerCount);
}

bool StarShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    bool loadAsCustomShape = false;

    if (element.localName() == "custom-shape") {
        QString drawEngine = element.attributeNS(KoXmlNS::draw, "engine", "");
        if (drawEngine != "koffice:star")
            return false;
        loadAsCustomShape = true;
    } else if (element.localName() != "regular-polygon") {
        return false;
    }

    QPointF loadedPosition = position();

    m_radius[tip] = 50;
    m_center = QPointF(50,50);

    if (!loadAsCustomShape) {
        QString corners = element.attributeNS(KoXmlNS::draw, "corners", "");
        if (! corners.isEmpty()) {
            m_cornerCount = corners.toUInt();
            // initialize default angles of tip and base
            m_angles[base] = m_angles[tip] = defaultAngleRadian();
        }

        m_convex = (element.attributeNS(KoXmlNS::draw, "concave", "false") == "false");

        if (m_convex) {
            m_radius[base] = m_radius[tip];
        } else {
            // sharpness is radius of ellipse on which inner polygon points are located
            // 0% means all polygon points are on a single ellipse
            // 100% means inner points are located at polygon center point
            QString sharpness = element.attributeNS(KoXmlNS::draw, "sharpness", "");
            if (! sharpness.isEmpty() && sharpness.right(1) == "%")
            {
                float percent = sharpness.left(sharpness.length()-1).toFloat();
                m_radius[base] = m_radius[tip] * (100-percent)/100;
            }
        }
    }
    else {
        QString drawData = element.attributeNS(KoXmlNS::draw, "data");
        if (drawData.isEmpty())
            return false;

        QStringList properties = drawData.split(';');
        if (properties.count() == 0)
            return false;

        foreach (const QString &property, properties) {
            QStringList pair = property.split(':');
            if (pair.count() != 2)
                continue;
            if (pair[0] == "corners") {
                m_cornerCount = pair[1].toInt();
            } else if (pair[0] == "concave") {
                m_convex = (pair[1] == "false");
            } else if (pair[0] == "baseRoundness") {
                m_roundness[base] = pair[1].toDouble();
            } else if (pair[0] == "tipRoundness") {
                m_roundness[tip] = pair[1].toDouble();
            } else if (pair[0] == "baseAngle") {
                m_angles[base] = pair[1].toDouble();
            } else if (pair[0] == "tipAngle") {
                m_angles[tip] = pair[1].toDouble();
            } else if (pair[0] == "sharpness") {
                float percent = pair[1].left(pair[1].length()-1).toFloat();
                m_radius[base] = m_radius[tip] * (100-percent)/100;
            }
        }

        if (m_convex) {
            m_radius[base] = m_radius[tip];
        }
    }

    updatePath(QSizeF());

    // reset transformation
    setTransformation(QMatrix());

    loadOdfAttributes(element, context, OdfAllAttributes);

    return true;
}

void StarShape::saveOdf(KoShapeSavingContext & context) const
{
    if (isParametricShape()) {
        double defaultAngle = defaultAngleRadian();
        bool hasRoundness = m_roundness[tip] != 0.0f || m_roundness[base] != 0.0f;
        bool hasAngleOffset = m_angles[base] != defaultAngle || m_angles[tip] != defaultAngle;
        if (hasRoundness || hasAngleOffset) {
            // draw:regular-polygon has no means of saving roundness
            // so we save as a custom shape with a specific draw:engine
            context.xmlWriter().startElement("draw:custom-shape");
            saveOdfAttributes(context, OdfAllAttributes);

            // now write the special shape data
            context.xmlWriter().addAttribute("draw:engine", "koffice:star");
            // create the data attribute
            QString drawData = QString("corners:%1;").arg(m_cornerCount);
            drawData += m_convex ? "concave:false;" : "concave:true;";
            if (! m_convex) {
                // sharpness is radius of ellipse on which inner polygon points are located
                // 0% means all polygon points are on a single ellipse
                // 100% means inner points are located at polygon center point
                qreal percent = (m_radius[tip]-m_radius[base]) / m_radius[tip] * 100.0;
                drawData += QString("sharpness:%1%;").arg(percent);
            }
            if (m_roundness[base] != 0.0f) {
                drawData += QString("baseRoundness:%1;").arg(m_roundness[base]);
            }
            if (m_roundness[tip] != 0.0f) {
                drawData += QString("tipRoundness:%1;").arg(m_roundness[tip]);
            }
            drawData += QString("baseAngle:%1;").arg(m_angles[base]);
            drawData += QString("tipAngle:%1;").arg(m_angles[tip]);

            context.xmlWriter().addAttribute("draw:data", drawData);

            // write a enhanced geometry element for compatibility with other applications
            context.xmlWriter().startElement("draw:enhanced-geometry");
            context.xmlWriter().addAttribute("draw:enhanced-path", toString(transformation()));
            context.xmlWriter().endElement(); // draw:enhanced-geometry

            saveOdfCommonChildElements(context);
            context.xmlWriter().endElement(); // draw:custom-shape
        }
        else {
            context.xmlWriter().startElement("draw:regular-polygon");
            saveOdfAttributes(context, OdfAllAttributes);
            context.xmlWriter().addAttribute("draw:corners", m_cornerCount);
            context.xmlWriter().addAttribute("draw:concave", m_convex ? "false" : "true");
            if (! m_convex) {
                // sharpness is radius of ellipse on which inner polygon points are located
                // 0% means all polygon points are on a single ellipse
                // 100% means inner points are located at polygon center point
                qreal percent = (m_radius[tip]-m_radius[base]) / m_radius[tip] * 100.0;
                context.xmlWriter().addAttribute("draw:sharpness", QString("%1%").arg(percent));
            }
            saveOdfCommonChildElements(context);
            context.xmlWriter().endElement();
        }
    } else {
        KoPathShape::saveOdf(context);
    }
}

QString StarShape::pathShapeId() const
{
    return StarShapeId;
}

double StarShape::defaultAngleRadian() const
{
    qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);

    return M_PI_2-2*radianStep;
}
