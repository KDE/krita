/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006-2009 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "StarShape.h"

#include <KoParameterShape_p.h>
#include <KoPathPoint.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <QStringList>

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

    m_center = QPointF(50, 50);
    updatePath(QSize(100, 100));
}

StarShape::StarShape(const StarShape &rhs)
    : KoParameterShape(rhs),
      m_cornerCount(rhs.m_cornerCount),
      m_radius(rhs.m_radius),
      m_angles(rhs.m_angles),
      m_zoomX(rhs.m_zoomX),
      m_zoomY(rhs.m_zoomY),
      m_roundness(rhs.m_roundness),
      m_center(rhs.m_center),
      m_convex(rhs.m_convex)
{
}

StarShape::~StarShape()
{
}

KoShape *StarShape::cloneShape() const
{
    return new StarShape(*this);
}


void StarShape::setCornerCount(uint cornerCount)
{
    if (cornerCount >= 3) {
        double oldDefaultAngle = defaultAngleRadian();
        m_cornerCount = cornerCount;
        double newDefaultAngle = defaultAngleRadian();
        m_angles[base] += newDefaultAngle - oldDefaultAngle;
        m_angles[tip] += newDefaultAngle - oldDefaultAngle;

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

void StarShape::moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if (modifiers & Qt::ShiftModifier) {
        QPointF handle = handles()[handleId];
        QPointF tangentVector = point - handle;
        qreal distance = sqrt(tangentVector.x() * tangentVector.x() + tangentVector.y() * tangentVector.y());
        QPointF radialVector = handle - m_center;
        // cross product to determine in which direction the user is dragging
        qreal moveDirection = radialVector.x() * tangentVector.y() - radialVector.y() * tangentVector.x();
        // make the roundness stick to zero if distance is under a certain value
        float snapDistance = 3.0;
        if (distance >= 0.0) {
            distance = distance < snapDistance ? 0.0 : distance - snapDistance;
        } else {
            distance = distance > -snapDistance ? 0.0 : distance + snapDistance;
        }
        // control changes roundness on both handles, else only the actual handle roundness is changed
        if (modifiers & Qt::ControlModifier) {
            m_roundness[handleId] = moveDirection < 0.0f ? distance : -distance;
        } else {
            m_roundness[base] = m_roundness[tip] = moveDirection < 0.0f ? distance : -distance;
        }
    } else {
        QPointF distVector = point - m_center;
        // unapply scaling
        distVector.rx() /= m_zoomX;
        distVector.ry() /= m_zoomY;
        m_radius[handleId] = sqrt(distVector.x() * distVector.x() + distVector.y() * distVector.y());

        qreal angle = atan2(distVector.y(), distVector.x());
        if (angle < 0.0) {
            angle += 2.0 * M_PI;
        }
        qreal diffAngle = angle - m_angles[handleId];
        qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);
        if (handleId == tip) {
            m_angles[tip] += diffAngle - radianStep;
            m_angles[base] += diffAngle - radianStep;
        } else {
            // control make the base point move freely
            if (modifiers & Qt::ControlModifier) {
                m_angles[base] += diffAngle - 2 * radianStep;
            } else {
                m_angles[base] = m_angles[tip];
            }
        }
    }
}

void StarShape::updatePath(const QSizeF &size)
{
    Q_UNUSED(size);
    qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);

    createPoints(m_convex ? m_cornerCount : 2 * m_cornerCount);

    KoSubpath &points = *subpaths()[0];

    uint index = 0;
    for (uint i = 0; i < 2 * m_cornerCount; ++i) {
        uint cornerType = i % 2;
        if (cornerType == base && m_convex) {
            continue;
        }
        qreal radian = static_cast<qreal>((i + 1) * radianStep) + m_angles[cornerType];
        QPointF cornerPoint = QPointF(m_zoomX * m_radius[cornerType] * cos(radian), m_zoomY * m_radius[cornerType] * sin(radian));

        points[index]->setPoint(m_center + cornerPoint);
        points[index]->unsetProperty(KoPathPoint::StopSubpath);
        points[index]->unsetProperty(KoPathPoint::CloseSubpath);
        if (m_roundness[cornerType] > 1e-10 || m_roundness[cornerType] < -1e-10) {
            // normalized cross product to compute tangential vector for handle point
            QPointF tangentVector(cornerPoint.y() / m_radius[cornerType], -cornerPoint.x() / m_radius[cornerType]);
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
    if (!m_convex) {
        handles.push_back(points.at(base)->point());
    }
    setHandles(handles);

    m_center = computeCenter();
}

void StarShape::createPoints(int requiredPointCount)
{
    if (subpaths().count() != 1) {
        clear();
        subpaths().append(new KoSubpath());
    }
    int currentPointCount = subpaths()[0]->count();
    if (currentPointCount > requiredPointCount) {
        for (int i = 0; i < currentPointCount - requiredPointCount; ++i) {
            delete subpaths()[0]->front();
            subpaths()[0]->pop_front();
        }
    } else if (requiredPointCount > currentPointCount) {
        for (int i = 0; i < requiredPointCount - currentPointCount; ++i) {
            subpaths()[0]->append(new KoPathPoint(this, QPointF()));
        }
    }

    notifyPointsChanged();
}

void StarShape::setSize(const QSizeF &newSize)
{
    QTransform matrix(resizeMatrix(newSize));
    m_zoomX *= matrix.m11();
    m_zoomY *= matrix.m22();

    // this transforms the handles
    KoParameterShape::setSize(newSize);

    m_center = computeCenter();
}

QPointF StarShape::computeCenter() const
{
    KoSubpath &points = *subpaths()[0];

    QPointF center(0, 0);
    for (uint i = 0; i < m_cornerCount; ++i) {
        if (m_convex) {
            center += points[i]->point();
        } else {
            center += points[2 * i]->point();
        }
    }
    if (m_cornerCount > 0) {
        return center / static_cast<qreal>(m_cornerCount);
    }
    return center;

}

QString StarShape::pathShapeId() const
{
    return StarShapeId;
}

double StarShape::defaultAngleRadian() const
{
    qreal radianStep = M_PI / static_cast<qreal>(m_cornerCount);

    return M_PI_2 - 2 * radianStep;
}
