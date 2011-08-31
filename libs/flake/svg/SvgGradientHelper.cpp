/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#include "SvgGradientHelper.h"

#include <cmath>
#include <KoFlake.h>

SvgGradientHelper::SvgGradientHelper()
        : m_gradient(0), m_gradientUnits(ObjectBoundingBox)
{
}

SvgGradientHelper::~SvgGradientHelper()
{
    delete m_gradient;
}

SvgGradientHelper::SvgGradientHelper(const SvgGradientHelper &other)
        : m_gradient(0), m_gradientUnits(ObjectBoundingBox)
{
    m_gradientUnits = other.m_gradientUnits;
    m_gradientTransform = other.m_gradientTransform;
    copyGradient(other.m_gradient);
}

SvgGradientHelper & SvgGradientHelper::operator = (const SvgGradientHelper & rhs)
{
    if (this == &rhs)
        return *this;

    m_gradientUnits = rhs.m_gradientUnits;
    m_gradientTransform = rhs.m_gradientTransform;
    copyGradient(rhs.m_gradient);

    return *this;
}

void SvgGradientHelper::setGradientUnits(Units units)
{
    m_gradientUnits = units;
}

SvgGradientHelper::Units SvgGradientHelper::gradientUnits() const
{
    return m_gradientUnits;
}

QGradient * SvgGradientHelper::gradient()
{
    return m_gradient;
}

void SvgGradientHelper::setGradient(QGradient * g)
{
    delete m_gradient;
    m_gradient = g;
}

void SvgGradientHelper::copyGradient(QGradient * other)
{
    delete m_gradient;
    m_gradient = duplicateGradient(other, QTransform());
}

QBrush SvgGradientHelper::adjustedFill(const QRectF &bound)
{
    QBrush brush;

    QGradient * g = adjustedGradient(bound);
    if (g) {
        brush = QBrush(*g);
        delete g;
    }

    return brush;
}

QTransform SvgGradientHelper::transform() const
{
    return m_gradientTransform;
}

void SvgGradientHelper::setTransform(const QTransform &transform)
{
    m_gradientTransform = transform;
}

QGradient * SvgGradientHelper::adjustedGradient(const QRectF &bound) const
{
    QTransform matrix;
    matrix.scale(0.01 * bound.width(), 0.01 * bound.height());

    return duplicateGradient(m_gradient, matrix);
}

QGradient * SvgGradientHelper::duplicateGradient(const QGradient * originalGradient, const QTransform &transform)
{
    if (! originalGradient)
        return 0;

    QGradient * duplicatedGradient = 0;

    switch (originalGradient->type()) {
    case QGradient::ConicalGradient: {
        const QConicalGradient * o = static_cast<const QConicalGradient*>(originalGradient);
        QConicalGradient * g = new QConicalGradient();
        g->setAngle(o->angle());
        g->setCenter(transform.map(o->center()));
        duplicatedGradient = g;
    }
    break;
    case QGradient::LinearGradient: {
        const QLinearGradient * o = static_cast<const QLinearGradient*>(originalGradient);
        QLinearGradient * g = new QLinearGradient();
        g->setStart(transform.map(o->start()));
        g->setFinalStop(transform.map(o->finalStop()));
        duplicatedGradient = g;
    }
    break;
    case QGradient::RadialGradient: {
        const QRadialGradient * o = static_cast<const QRadialGradient*>(originalGradient);
        QRadialGradient * g = new QRadialGradient();
        g->setCenter(transform.map(o->center()));
        g->setFocalPoint(transform.map(o->focalPoint()));
        g->setRadius(transform.map(QPointF(o->radius(), 0.0)).x());
        duplicatedGradient = g;
    }
    break;
    default:
        return 0;
    }

    duplicatedGradient->setCoordinateMode(originalGradient->coordinateMode());
    duplicatedGradient->setStops(originalGradient->stops());
    duplicatedGradient->setSpread(originalGradient->spread());

    return duplicatedGradient;
}

QGradient *SvgGradientHelper::convertGradient(const QGradient *originalGradient, const QSizeF &size)
{
    if (! originalGradient)
        return 0;

    if (originalGradient->coordinateMode() != QGradient::LogicalMode) {
        return duplicateGradient(originalGradient, QTransform());
    }

    QGradient *duplicatedGradient = 0;

    switch (originalGradient->type()) {
    case QGradient::ConicalGradient:
        {
            const QConicalGradient *o = static_cast<const QConicalGradient*>(originalGradient);
            QConicalGradient *g = new QConicalGradient();
            g->setAngle(o->angle());
            g->setCenter(KoFlake::toRelative(o->center(),size));
            duplicatedGradient = g;
        }
        break;
    case QGradient::LinearGradient:
        {
            const QLinearGradient *o = static_cast<const QLinearGradient*>(originalGradient);
            QLinearGradient *g = new QLinearGradient();
            g->setStart(KoFlake::toRelative(o->start(),size));
            g->setFinalStop(KoFlake::toRelative(o->finalStop(),size));
            duplicatedGradient = g;
        }
        break;
    case QGradient::RadialGradient:
        {
            const QRadialGradient *o = static_cast<const QRadialGradient*>(originalGradient);
            QRadialGradient *g = new QRadialGradient();
            g->setCenter(KoFlake::toRelative(o->center(),size));
            g->setFocalPoint(KoFlake::toRelative(o->focalPoint(),size));
            g->setRadius(KoFlake::toRelative(QPointF(o->radius(), 0.0),
                         QSizeF(sqrt(size.width() * size.width() + size.height() * size.height()), 0.0)).x());
            duplicatedGradient = g;
        }
        break;
    default:
        return 0;
    }

    duplicatedGradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    duplicatedGradient->setStops(originalGradient->stops());
    duplicatedGradient->setSpread(originalGradient->spread());

    return duplicatedGradient;
}

