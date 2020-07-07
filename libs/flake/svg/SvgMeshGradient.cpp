/*
 *  Copyright (c) 2020 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "SvgMeshGradient.h"

SvgMeshGradient::SvgMeshGradient()
    : m_mesharray(new SvgMeshArray())
{
}

SvgMeshGradient::SvgMeshGradient(const SvgMeshGradient& other)
    : m_type(other.m_type)
    , m_mesharray(new SvgMeshArray(*other.m_mesharray))
{
}

void SvgMeshGradient::setType(SvgMeshGradient::Type type)
{
    m_type = type;
}

SvgMeshGradient::Type SvgMeshGradient::type() const
{
    return m_type;
}

void SvgMeshGradient::setTransform(const QTransform& matrix)
{
    m_mesharray->setTransform(matrix);
}

bool SvgMeshGradient::isValid() const
{
    return m_mesharray->numRows() != 0;
}

QRectF SvgMeshGradient::boundingRect() const
{
    return m_mesharray->boundingRect();
}

QScopedPointer<SvgMeshArray>& SvgMeshGradient::getMeshArray()
{
    return m_mesharray;
}
