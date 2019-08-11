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

#include <QConicalGradient>
#include <QLinearGradient>
#include <QRadialGradient>

#include <cmath>
#include <KoFlake.h>

SvgGradientHelper::SvgGradientHelper()
    : m_gradient(new QGradient()), m_gradientUnits(KoFlake::ObjectBoundingBox)
{
}

SvgGradientHelper::~SvgGradientHelper()
{
    delete m_gradient;
}

SvgGradientHelper::SvgGradientHelper(const SvgGradientHelper &other)
    : m_gradient(KoFlake::cloneGradient(other.m_gradient))
    , m_gradientUnits(other.m_gradientUnits)
    , m_gradientTransform(other.m_gradientTransform)
{
}

SvgGradientHelper & SvgGradientHelper::operator = (const SvgGradientHelper & rhs)
{
    if (this == &rhs)
        return *this;

    m_gradientUnits = rhs.m_gradientUnits;
    m_gradientTransform = rhs.m_gradientTransform;
    m_gradient = KoFlake::cloneGradient(rhs.m_gradient);

    return *this;
}

void SvgGradientHelper::setGradientUnits(KoFlake::CoordinateSystem units)
{
    m_gradientUnits = units;
}

KoFlake::CoordinateSystem SvgGradientHelper::gradientUnits() const
{
    return m_gradientUnits;
}

QGradient * SvgGradientHelper::gradient() const
{
    return m_gradient;
}

void SvgGradientHelper::setGradient(QGradient * g)
{
    delete m_gradient;
    m_gradient = g;
}

QTransform SvgGradientHelper::transform() const
{
    return m_gradientTransform;
}

void SvgGradientHelper::setTransform(const QTransform &transform)
{
    m_gradientTransform = transform;
}

QGradient::Spread SvgGradientHelper::spreadMode() const
{
    return m_spreadMode;
}

void SvgGradientHelper::setSpreadMode(const QGradient::Spread &spreadMode)
{
    m_spreadMode = spreadMode;
}
