/*
 * KoReport Lirary
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
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

#include "KoReportScriptCheck.h"

namespace Scripting
{
    
Check::Check(KoReportItemCheck *c)
{
    m_check = c;
}


Check::~Check()
{
}

bool Check::value()
{
    return m_check->value();
}

void Check::setValue(bool v)
{
    m_check->setValue(v);
}

QString Check::checkStyle()
{
    return m_check->m_checkStyle->value().toString();
}

void Check::setCheckStyle(const QString &style)
{
    m_check->m_checkStyle->setValue(style);
}

QColor Check::foregroundColor()
{
    return m_check->m_foregroundColor->value().value<QColor>();
}
void Check::setForegroundColor(const QColor& c)
{
    m_check->m_foregroundColor->setValue(c);
}

QColor Check::lineColor()
{
    return m_check->m_lineColor->value().value<QColor>();
}
void Check::setLineColor(const QColor& c)
{
    m_check->m_lineColor->setValue(c);
}

int Check::lineWeight()
{
    return m_check->m_lineWeight->value().toInt();
}
void Check::setLineWeight(int w)
{
    m_check->m_lineWeight->setValue(w);
}

int Check::lineStyle()
{
    return m_check->m_lineStyle->value().toInt();
}
void Check::setLineStyle(int s)
{
    if (s < 0 || s > 5) {
        s = 1;
    }
    m_check->m_lineStyle->setValue(s);
}

QPointF Check::position()
{
    return m_check->m_pos.toPoint();
}
void Check::setPosition(const QPointF &p)
{
    m_check->m_pos.setPointPos(p);
}

QSizeF Check::size()
{
    return m_check->m_size.toPoint();
}
void Check::setSize(const QSizeF &s)
{
    m_check->m_size.setPointSize(s);
}
}

