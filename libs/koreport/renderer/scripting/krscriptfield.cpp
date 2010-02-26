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
#include "krscriptfield.h"

namespace Scripting
{
Field::Field(KRFieldData *f)
{
    m_field = f;
}


Field::~Field()
{
}

QString Field::source()
{
    return m_field->controlSource();
}

void Field::setSource(const QString& s)
{
    m_field->setControlSource(s);
}

int Field::horizontalAlignment()
{
    QString a = m_field->m_horizontalAlignment->value().toString();

    if (a.toLower() == "left") {
        return -1;
    } else if (a.toLower() == "center") {
        return 0;
    } else if (a.toLower() == "right") {
        return 1;
    }
    return -1;
}
void Field::setHorizonalAlignment(int a)
{
    switch (a) {
    case -1:
        m_field->m_horizontalAlignment->setValue("left");
        break;
    case 0:
        m_field->m_horizontalAlignment->setValue("center");
        break;
    case 1:
        m_field->m_horizontalAlignment->setValue("right");
        break;
    default:
        m_field->m_horizontalAlignment->setValue("left");
        break;
    }
}

int Field::verticalAlignment()
{
    QString a = m_field->m_horizontalAlignment->value().toString();

    if (a.toLower() == "top") {
        return -1;
    } else if (a.toLower() == "middle") {
        return 0;
    } else if (a.toLower() == "bottom") {
        return 1;
    }
    return -1;
}
void Field::setVerticalAlignment(int a)
{
    switch (a) {
    case -1:
        m_field->m_verticalAlignment->setValue("top");
        break;
    case 0:
        m_field->m_verticalAlignment->setValue("middle");
        break;
    case 1:
        m_field->m_verticalAlignment->setValue("bottom");
        break;
    default:
        m_field->m_verticalAlignment->setValue("middle");
        break;
    }
}

QColor Field::backgroundColor()
{
    return m_field->m_backgroundColor->value().value<QColor>();
}
void Field::setBackgroundColor(const QColor& c)
{
    m_field->m_backgroundColor->setValue(c);
}

QColor Field::foregroundColor()
{
    return m_field->m_foregroundColor->value().value<QColor>();
}
void Field::setForegroundColor(const QColor& c)
{
    m_field->m_foregroundColor->setValue(c);
}

int Field::backgroundOpacity()
{
    return m_field->m_backgroundOpacity->value().toInt();
}
void Field::setBackgroundOpacity(int o)
{
    m_field->m_backgroundOpacity->setValue(o);
}

QColor Field::lineColor()
{
    return m_field->m_lineColor->value().value<QColor>();
}
void Field::setLineColor(const QColor& c)
{
    m_field->m_lineColor->setValue(c);
}

int Field::lineWeight()
{
    return m_field->m_lineWeight->value().toInt();
}
void Field::setLineWeight(int w)
{
    m_field->m_lineWeight->setValue(w);
}

int Field::lineStyle()
{
    return m_field->m_lineStyle->value().toInt();
}
void Field::setLineStyle(int s)
{
    if (s < 0 || s > 5) {
        s = 1;
    }
    m_field->m_lineStyle->setValue(s);
}

QPointF Field::position()
{
    return m_field->m_pos.toPoint();
}
void Field::setPosition(const QPointF &p)
{
    m_field->m_pos.setPointPos(p);
}

QSizeF Field::size()
{
    return m_field->m_size.toPoint();
}
void Field::setSize(const QSizeF &s)
{
    m_field->m_size.setPointSize(s);
}
}
