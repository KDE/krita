/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#include "SvgFilterHelper.h"
#include "SvgUtil.h"

SvgFilterHelper::SvgFilterHelper()
        : m_filterUnits(ObjectBoundingBox) // default as per svg spec
        , m_primitiveUnits(UserSpaceOnUse) // default as per svg spec
        , m_position(-0.1, -0.1) // default as per svg spec
        , m_size(1.2, 1.2) // default as per svg spec
{
}

SvgFilterHelper::~SvgFilterHelper()
{
}

void SvgFilterHelper::setFilterUnits(Units filterUnits)
{
    m_filterUnits = filterUnits;
}

SvgFilterHelper::Units SvgFilterHelper::filterUnits() const
{
    return m_filterUnits;
}

void SvgFilterHelper::setPrimitiveUnits(Units primitiveUnits)
{
    m_primitiveUnits = primitiveUnits;
}

SvgFilterHelper::Units SvgFilterHelper::primitiveUnits() const
{
    return m_primitiveUnits;
}

void SvgFilterHelper::setPosition(const QPointF & position)
{
    m_position = position;
}

QPointF SvgFilterHelper::position(const QRectF & objectBound) const
{
    if (m_filterUnits == UserSpaceOnUse) {
        return m_position;
    } else {
        return SvgUtil::objectToUserSpace(m_position, objectBound);
    }
}

void SvgFilterHelper::setSize(const QSizeF & size)
{
    m_size = size;
}

QSizeF SvgFilterHelper::size(const QRectF & objectBound) const
{
    if (m_filterUnits == UserSpaceOnUse) {
        return m_size;
    } else {
        return SvgUtil::objectToUserSpace(m_size, objectBound);
    }
}

void SvgFilterHelper::setContent(const KoXmlElement &content)
{
    m_filterContent = content;
}

KoXmlElement SvgFilterHelper::content() const
{
    return m_filterContent;
}
