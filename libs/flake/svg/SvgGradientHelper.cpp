/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2010 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgGradientHelper.h"

#include <QConicalGradient>
#include <QLinearGradient>
#include <QRadialGradient>

#include <cmath>
#include <KoFlake.h>

SvgGradientHelper::SvgGradientHelper()
    : m_gradient(new QGradient())
    , m_meshgradient(new SvgMeshGradient)
    , m_gradientUnits(KoFlake::ObjectBoundingBox)
{
}

SvgGradientHelper::~SvgGradientHelper()
{
}

SvgGradientHelper::SvgGradientHelper(const SvgGradientHelper &other)
    : m_gradient(KoFlake::cloneGradient(other.m_gradient.data()))
    , m_meshgradient(new SvgMeshGradient(*other.m_meshgradient))
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
    m_gradient.reset(KoFlake::cloneGradient(rhs.m_gradient.data()));
    m_meshgradient.reset(new SvgMeshGradient(*rhs.m_meshgradient));

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
    return m_gradient.data();
}

void SvgGradientHelper::setGradient(QGradient * g)
{
    m_gradient.reset(g);
}

void SvgGradientHelper::setMeshGradient(SvgMeshGradient *g)
{
    m_meshgradient.reset(new SvgMeshGradient(*g));
}

QScopedPointer<SvgMeshGradient>& SvgGradientHelper::meshgradient()
{
    return m_meshgradient;
}

bool SvgGradientHelper::isMeshGradient() const
{
    return m_meshgradient->isValid();
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
    return m_gradient->spread();
}

void SvgGradientHelper::setSpreadMode(const QGradient::Spread &spreadMode)
{
    m_gradient->setSpread(spreadMode);
}
