/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgClipPathHelper.h"

SvgClipPathHelper::SvgClipPathHelper()
    : m_clipPathUnits(KoFlake::UserSpaceOnUse) // default as per svg spec
{
}

SvgClipPathHelper::~SvgClipPathHelper()
{
}

void SvgClipPathHelper::setClipPathUnits(KoFlake::CoordinateSystem clipPathUnits)
{
    m_clipPathUnits = clipPathUnits;
}

KoFlake::CoordinateSystem SvgClipPathHelper::clipPathUnits() const
{
    return m_clipPathUnits;
}

QList<KoShape *> SvgClipPathHelper::shapes() const
{
    return m_shapes;
}

void SvgClipPathHelper::setShapes(const QList<KoShape *> &shapes)
{
    m_shapes = shapes;
}

bool SvgClipPathHelper::isEmpty() const
{
    return m_shapes.isEmpty();
}
