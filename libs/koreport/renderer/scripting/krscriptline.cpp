/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krscriptline.h"
#include <krlinedata.h>

namespace Scripting
{

Line::Line(KRLineData* l)
{
    m_line = l;
}

Line::~Line()
{
}


QColor Line::lineColor()
{
    return m_line->m_lineColor->value().value<QColor>();
}

void Line::setLineColor(const QColor& c)
{
    m_line->m_lineColor->setValue(c);
}

int Line::lineWeight()
{
    return m_line->m_lineWeight->value().toInt();
}

void Line::setLineWeight(int w)
{
    m_line->m_lineWeight->setValue(w);
}

int Line::lineStyle()
{
    return m_line->m_lineStyle->value().toInt();
}
void Line::setLineStyle(int s)
{
    if (s < 0 || s > 5) {
        s = 1;
    }
    m_line->m_lineStyle->setValue(s);
}

QPointF Line::startPosition()
{
    return m_line->m_start.toPoint();
}

void Line::setStartPosition(const QPointF& p)
{
    m_line->m_start.setPointPos(p);
}

QPointF Line::endPosition()
{
    return m_line->m_end.toPoint();
}

void Line::setEndPosition(const QPointF& p)
{
    m_line->m_end.setPointPos(p);
}
}
