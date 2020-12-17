/*
 *  SPDX-FileCopyrightText: 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgMeshGradient.h"

SvgMeshGradient::SvgMeshGradient()
    : m_type(BILINEAR)
    , m_gradientUnits(KoFlake::UserSpaceOnUse)
    , m_mesharray(new SvgMeshArray())
{
}

SvgMeshGradient::SvgMeshGradient(const SvgMeshGradient& other)
    : m_type(other.m_type)
    , m_gradientUnits(other.m_gradientUnits)
    , m_mesharray(new SvgMeshArray(*other.m_mesharray))
{
}

void SvgMeshGradient::setType(SvgMeshGradient::Shading type)
{
    m_type = type;
}

SvgMeshGradient::Shading SvgMeshGradient::type() const
{
    return m_type;
}

void SvgMeshGradient::setTransform(const QTransform& matrix)
{
    m_mesharray->setTransform(matrix);
}

bool SvgMeshGradient::isValid() const
{
    return m_mesharray->numRows() > 0 && m_mesharray->numColumns() > 0;
}

QRectF SvgMeshGradient::boundingRect() const
{
    return m_mesharray->boundingRect();
}

const QScopedPointer<SvgMeshArray>& SvgMeshGradient::getMeshArray() const
{
    return m_mesharray;
}
