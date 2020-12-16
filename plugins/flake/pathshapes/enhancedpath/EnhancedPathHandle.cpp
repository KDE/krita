/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "EnhancedPathHandle.h"
#include "EnhancedPathShape.h"
#include "EnhancedPathParameter.h"
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>

#include <math.h>

EnhancedPathHandle::EnhancedPathHandle(EnhancedPathShape *parent)
    : m_parent(parent)
    , m_positionX(0)
    , m_positionY(0)
    , m_minimumX(0)
    , m_minimumY(0)
    , m_maximumX(0)
    , m_maximumY(0)
    , m_polarX(0)
    , m_polarY(0)
    , m_minRadius(0)
    , m_maxRadius(0)
{
    Q_ASSERT(m_parent);
}

EnhancedPathHandle::~EnhancedPathHandle()
{
}

bool EnhancedPathHandle::hasPosition() const
{
    return m_positionX && m_positionY;
}

void EnhancedPathHandle::setPosition(EnhancedPathParameter *positionX, EnhancedPathParameter *positionY)
{
    m_positionX = positionX;
    m_positionY = positionY;
}

QPointF EnhancedPathHandle::position()
{
    if (!hasPosition()) {
        return QPointF();
    }

    QPointF position(m_positionX->evaluate(), m_positionY->evaluate());
    if (isPolar()) {
        // convert polar coordinates into cartesian coordinates
        QPointF center(m_polarX->evaluate(), m_polarY->evaluate());
        qreal angleInRadian = position.x() * M_PI / 180.0;
        position = center + position.y() * QPointF(cos(angleInRadian), sin(angleInRadian));
    }

    return position;
}

void EnhancedPathHandle::changePosition(const QPointF &position)
{
    if (!hasPosition()) {
        return;
    }

    QPointF constrainedPosition(position);

    if (isPolar()) {
        // convert cartesian coordinates into polar coordinates
        QPointF polarCenter(m_polarX->evaluate(), m_polarY->evaluate());
        QPointF diff = constrainedPosition - polarCenter;
        // compute the polar radius
        qreal radius = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        // compute the polar angle
        qreal angle = atan2(diff.y(), diff.x());
        if (angle < 0.0) {
            angle += 2 * M_PI;
        }

        // constrain the radius
        if (m_minRadius) {
            radius = qMax(m_minRadius->evaluate(), radius);
        }
        if (m_maxRadius) {
            radius = qMin(m_maxRadius->evaluate(), radius);
        }

        constrainedPosition.setX(angle * 180.0 / M_PI);
        constrainedPosition.setY(radius);
    } else {
        // constrain x coordinate
        if (m_minimumX) {
            constrainedPosition.setX(qMax(m_minimumX->evaluate(), constrainedPosition.x()));
        }
        if (m_maximumX) {
            constrainedPosition.setX(qMin(m_maximumX->evaluate(), constrainedPosition.x()));
        }

        // constrain y coordinate
        if (m_minimumY) {
            constrainedPosition.setY(qMax(m_minimumY->evaluate(), constrainedPosition.y()));
        }
        if (m_maximumY) {
            constrainedPosition.setY(qMin(m_maximumY->evaluate(), constrainedPosition.y()));
        }
    }

    m_positionX->modify(constrainedPosition.x());
    m_positionY->modify(constrainedPosition.y());
}

void EnhancedPathHandle::setRangeX(EnhancedPathParameter *minX, EnhancedPathParameter *maxX)
{
    m_minimumX = minX;
    m_maximumX = maxX;
}

void EnhancedPathHandle::setRangeY(EnhancedPathParameter *minY, EnhancedPathParameter *maxY)
{
    m_minimumY = minY;
    m_maximumY = maxY;
}

void EnhancedPathHandle::setPolarCenter(EnhancedPathParameter *polarX, EnhancedPathParameter *polarY)
{
    m_polarX = polarX;
    m_polarY = polarY;
}

void EnhancedPathHandle::setRadiusRange(EnhancedPathParameter *minRadius, EnhancedPathParameter *maxRadius)
{
    m_minRadius = minRadius;
    m_maxRadius = maxRadius;
}

bool EnhancedPathHandle::isPolar() const
{
    return m_polarX && m_polarY;
}
