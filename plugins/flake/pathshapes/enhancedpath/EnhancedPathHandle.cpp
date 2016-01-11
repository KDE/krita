/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "EnhancedPathHandle.h"
#include "EnhancedPathShape.h"
#include "EnhancedPathParameter.h"
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfWorkaround.h>

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

void EnhancedPathHandle::saveOdf(KoShapeSavingContext &context) const
{
    if (!hasPosition()) {
        return;
    }
    context.xmlWriter().startElement("draw:handle");
    context.xmlWriter().addAttribute("draw:handle-position", m_positionX->toString() + ' ' + m_positionY->toString());
    if (isPolar()) {
        context.xmlWriter().addAttribute("draw:handle-polar", m_polarX->toString() + ' ' + m_polarY->toString());
        if (m_minRadius) {
            context.xmlWriter().addAttribute("draw:handle-radius-range-minimum", m_minRadius->toString());
        }
        if (m_maxRadius) {
            context.xmlWriter().addAttribute("draw:handle-radius-range-maximum", m_maxRadius->toString());
        }
    } else {
        if (m_minimumX) {
            context.xmlWriter().addAttribute("draw:handle-range-x-minimum", m_minimumX->toString());
        }
        if (m_maximumX) {
            context.xmlWriter().addAttribute("draw:handle-range-x-maximum", m_maximumX->toString());
        }
        if (m_minimumY) {
            context.xmlWriter().addAttribute("draw:handle-range-y-minimum", m_minimumY->toString());
        }
        if (m_maximumY) {
            context.xmlWriter().addAttribute("draw:handle-range-y-maximum", m_maximumY->toString());
        }
    }
    context.xmlWriter().endElement(); // draw:handle
}

bool EnhancedPathHandle::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (element.localName() != "handle" || element.namespaceURI() != KoXmlNS::draw) {
        return false;
    }

    QString position = element.attributeNS(KoXmlNS::draw, "handle-position");
#ifndef NWORKAROUND_ODF_BUGS
    KoOdfWorkaround::fixEnhancedPathPolarHandlePosition(position, element, context);
#endif
    QStringList tokens = position.simplified().split(' ');
    if (tokens.count() != 2) {
        return false;
    }

    setPosition(m_parent->parameter(tokens[0]), m_parent->parameter(tokens[1]));

    // check if we have a polar handle
    if (element.hasAttributeNS(KoXmlNS::draw, "handle-polar")) {
        QString polar = element.attributeNS(KoXmlNS::draw, "handle-polar");
        QStringList tokens = polar.simplified().split(' ');
        if (tokens.count() == 2) {
            setPolarCenter(m_parent->parameter(tokens[0]), m_parent->parameter(tokens[1]));

            QString minRadius = element.attributeNS(KoXmlNS::draw, "handle-radius-range-minimum");
            QString maxRadius = element.attributeNS(KoXmlNS::draw, "handle-radius-range-maximum");
            if (!minRadius.isEmpty() && !maxRadius.isEmpty()) {
                setRadiusRange(m_parent->parameter(minRadius), m_parent->parameter(maxRadius));
            }
        }
    } else {
        QString minX = element.attributeNS(KoXmlNS::draw, "handle-range-x-minimum");
        QString maxX = element.attributeNS(KoXmlNS::draw, "handle-range-x-maximum");
        if (!minX.isEmpty() && ! maxX.isEmpty()) {
            setRangeX(m_parent->parameter(minX), m_parent->parameter(maxX));
        }

        QString minY = element.attributeNS(KoXmlNS::draw, "handle-range-y-minimum");
        QString maxY = element.attributeNS(KoXmlNS::draw, "handle-range-y-maximum");
        if (!minY.isEmpty() && ! maxY.isEmpty()) {
            setRangeY(m_parent->parameter(minY), m_parent->parameter(maxY));
        }
    }

    return hasPosition();
}
