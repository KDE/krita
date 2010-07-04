/* This file is part of the KDE project
   Copyright (C) 2007 Rob Buis <buis@kde.org>

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

#include "SpiralShape.h"

#include <KoPathPoint.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

#include <math.h>

SpiralShape::SpiralShape()
    : m_fade(.9)
    , m_kindAngle(M_PI)
    , m_radii(100.0, 100.0)
    , m_type(Curve)
    , m_clockwise(true)
{
    //m_handles.push_back(QPointF(50, 0));
    //m_handles.push_back(QPointF(50, 50));
    //m_handles.push_back(QPointF(0, 50));
    createPath(QSizeF(m_radii.x(), m_radii.y()));
}

SpiralShape::~SpiralShape()
{
}

void SpiralShape::saveOdf(KoShapeSavingContext &context) const
{
    // TODO?
    KoPathShape::saveOdf(context);
}

bool SpiralShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &/*context*/)
{
    Q_UNUSED(element);

    // TODO?
    return true;
}

void SpiralShape::setSize(const QSizeF &newSize)
{
    QTransform matrix(resizeMatrix(newSize));
    m_center = matrix.map(m_center);
    m_radii = matrix.map(m_radii);
    KoParameterShape::setSize(newSize);
}

QPointF SpiralShape::normalize()
{
    QPointF offset(KoParameterShape::normalize());
    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());
    m_center = matrix.map(m_center);
    return offset;
}

void SpiralShape::moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(handleId);
    Q_UNUSED(point);
    Q_UNUSED(modifiers);
#if 0
    QPointF p(point);

    QPointF diff(m_center - point);
    diff.setX(-diff.x());
    qreal angle = 0;
    if (diff.x() == 0)
    {
        angle = (diff.y() < 0 ? 270 : 90) * M_PI / 180.0;
    }
    else
    {
        diff.setY(diff.y() * m_radii.x() / m_radii.y());
        angle = atan(diff.y() / diff.x ());
        if (angle < 0)
            angle = M_PI + angle;
        if (diff.y() < 0)
            angle += M_PI;
    }

    switch (handleId)
    {
        case 0:
            p = QPointF(m_center + QPointF(cos(angle) * m_radii.x(), -sin(angle) * m_radii.y()));
            m_handles[handleId] = p;
            updateKindHandle();
            break;
        case 1:
            p = QPointF(m_center + QPointF(cos(angle) * m_radii.x(), -sin(angle) * m_radii.y()));
            m_handles[handleId] = p;
            updateKindHandle();
            break;
        case 2:
        {
            QList<QPointF> kindHandlePositions;
            kindHandlePositions.push_back(QPointF(m_center + QPointF(cos(m_kindAngle) * m_radii.x(), -sin(m_kindAngle) * m_radii.y())));
            kindHandlePositions.push_back(m_center);
            kindHandlePositions.push_back((m_handles[0] + m_handles[1]) / 2.0);

            QPointF diff = m_center * 2.0;
            int handlePos = 0;
            for (int i = 0; i < kindHandlePositions.size(); ++i)
            {
                QPointF pointDiff(p - kindHandlePositions[i]);
                if (i == 0 || qAbs(pointDiff.x()) + qAbs(pointDiff.y()) < qAbs(diff.x()) + qAbs(diff.y()))
                {
                    diff = pointDiff;
                    handlePos = i;
                }
            }
            m_handles[handleId] = kindHandlePositions[handlePos];
            m_type = SpiralType(handlePos);
        } break;
    }
#endif
}

void SpiralShape::updatePath(const QSizeF &size)
{
    createPath(size);
    normalize();
#if 0
    Q_UNUSED(size);
    QPointF startpoint(m_handles[0]);

    QPointF curvePoints[12];

    int pointCnt = arcToCurve(m_radii.x(), m_radii.y(), m_startAngle, sweepAngle() , startpoint, curvePoints);

    int cp = 0;
    m_points[cp]->setPoint(startpoint);
    m_points[cp]->unsetProperty(KoPathPoint::HasControlPoint1);
    for (int i = 0; i < pointCnt; i += 3)
    {
        m_points[cp]->setControlPoint2(curvePoints[i]);
        m_points[++cp]->setControlPoint1(curvePoints[i+1]); 
        m_points[cp]->setPoint(curvePoints[i+2]);
        m_points[cp]->unsetProperty(KoPathPoint::HasControlPoint2);
    }
    if (m_type == Curve)
    {
        m_points[++cp]->setPoint(m_center);
        m_points[cp]->unsetProperty(KoPathPoint::HasControlPoint1);
        m_points[cp]->unsetProperty(KoPathPoint::HasControlPoint2);
    }
    else if (m_type == Line && m_startAngle == m_endAngle)
    {
        m_points[0]->setControlPoint1(m_points[cp]->controlPoint1());
        m_points[0]->setPoint(m_points[cp]->point());
        --cp;
    }

    m_subpaths[0]->clear();
    for (int i = 0; i <= cp; ++i)
    {
        if (i < cp || (m_type == Line && m_startAngle != m_endAngle))
        {
            m_points[i]->unsetProperty(KoPathPoint::CloseSubpath);
        }
        else
        {
            m_points[i]->setProperty(KoPathPoint::CloseSubpath);
        }
        m_subpaths[0]->push_back(m_points[i]);
    }

#endif
}

void SpiralShape::createPath(const QSizeF &size)
{
    Q_UNUSED(size);
    clear();
    QPointF center = QPointF(m_radii.x() / 2.0, m_radii.y() / 2.0);
    //moveTo(QPointF(size.width(), m_radii.y()));
    qreal adv_ang = (m_clockwise ? -1.0 : 1.0) * M_PI_2;
    // radius of first segment is non-faded radius:
    qreal m_radius = m_radii.x() / 2.0;
    qreal r = m_radius;

    QPointF oldP(center.x(), (m_clockwise ? -1.0 : 1.0) * m_radius + center.y());
    QPointF newP;
    QPointF newCenter(center);
    moveTo(oldP);
    uint m_segments = 10;
    //m_handles[0] = oldP;

    for (uint i = 0; i < m_segments; ++i) {
        newP.setX(r * cos(adv_ang * (i + 2)) + newCenter.x());
        newP.setY(r * sin(adv_ang * (i + 2)) + newCenter.y());

        if (m_type == Curve) {
            qreal rx = abs(oldP.x() - newP.x());
            qreal ry = abs(oldP.y() - newP.y());
            if (m_clockwise) {
                arcTo(rx, ry, ((i + 1) % 4) * 90, 90);
            } else {
                arcTo(rx, ry, 360 - ((i + 1) % 4) * 90, -90);
            }
        } else {
            lineTo(newP);
        }

        newCenter += (newP - newCenter) * (1.0 - m_fade);
        oldP = newP;
        r *= m_fade;
    }
    //m_handles[1] = QPointF(center.x(), (m_clockwise ? -1.0 : 1.0) * m_radius + center.y());
    m_points = *m_subpaths[0];
}

void SpiralShape::updateKindHandle()
{
/*
   m_kindAngle = (m_startAngle + m_endAngle) * M_PI / 360.0;
   if (m_startAngle > m_endAngle)
   {
       m_kindAngle += M_PI;
   }
   switch (m_type)
   {
       case Curve:
           m_handles[2] = m_center + QPointF(cos(m_kindAngle) * m_radii.x(), -sin(m_kindAngle) * m_radii.y());
           break;
       case Line:
           m_handles[2] = m_center;
           break;
   }
   */
}

void SpiralShape::updateAngleHandles()
{
//    qreal startRadian = m_startAngle * M_PI / 180.0;
//    qreal endRadian = m_endAngle * M_PI / 180.0;
//    m_handles[0] = m_center + QPointF(cos(startRadian) * m_radii.x(), -sin(startRadian) * m_radii.y());
//    m_handles[1] = m_center + QPointF(cos(endRadian) * m_radii.x(), -sin(endRadian) * m_radii.y());
}

void SpiralShape::setType(SpiralType type)
{
    m_type = type;
    updateKindHandle();
    updatePath(size());
}

SpiralShape::SpiralType SpiralShape::type() const
{
    return m_type;
}

void SpiralShape::setFade(qreal fade)
{
    m_fade = fade;
    updateKindHandle();
    //updateAngleHandles();
    updatePath(size());
}

qreal SpiralShape::fade() const
{
    return m_fade;
}

bool SpiralShape::clockWise() const
{
    return m_clockwise;
}

void SpiralShape::setClockWise(bool clockWise)
{
    m_clockwise = clockWise;
    updateKindHandle();
    //updateAngleHandles();
    updatePath(size());
}

QString SpiralShape::pathShapeId() const
{
    return SpiralShapeId;
}
