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
#include "krscriptchart.h"

#include <krchartdata.h>

namespace Scripting
{

Chart::Chart(KRChartData *c)
{
    m_chart = c;
}


Chart::~Chart()
{
}

QPointF Chart::position()
{
    return m_chart->m_pos.toPoint();
}
void Chart::setPosition(const QPointF& p)
{
    m_chart->m_pos.setPointPos(p);
}

QSizeF Chart::size()
{
    return m_chart->m_size.toPoint();
}
void Chart::setSize(const QSizeF& s)
{
    m_chart->m_size.setPointSize(s);
}

QString Chart::dataSource()
{
    return m_chart->m_dataSource->value().toString();
}

void Chart::setDataSource(const QString &ds)
{
    m_chart->m_dataSource->setValue(ds);
}

bool Chart::threeD()
{
    return m_chart->m_threeD->value().toBool();
}

void Chart::setThreeD(bool td)
{
    m_chart->m_threeD->setValue(td);
}

bool Chart::legendVisible()
{
    return m_chart->m_displayLegend->value().toBool();
}

void Chart::setLegendVisible(bool v)
{
    m_chart->m_displayLegend->setValue(v);
}

int Chart::colorScheme()
{
    return m_chart->m_colorScheme->value().toInt();
}

void Chart::setColorScheme(int cs)
{
    m_chart->m_colorScheme->setValue(cs);
}

QColor Chart::backgroundColor()
{
    return m_chart->m_backgroundColor->value().value<QColor>();
}

void Chart::setBackgroundColor(const QColor &bc)
{
    m_chart->m_backgroundColor->setValue(bc);
}

QString Chart::xAxisTitle()
{
    return m_chart->m_xTitle->value().toString();
}

void Chart::setXAxisTitle(const QString &t)
{
    m_chart->m_xTitle->setValue(t);
}

QString Chart::yAxisTitle()
{
    return m_chart->m_yTitle->value().toString();
}

void Chart::setYAxisTitle(const QString &t)
{
    m_chart->m_yTitle->setValue(t);
}
}
