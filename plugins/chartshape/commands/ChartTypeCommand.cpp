/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ChartTypeCommand.h"

// KDE
#include <kdebug.h>
#include <klocalizedstring.h>

// KDChart
#include "KDChartAbstractCoordinatePlane"
#include "KDChartBarDiagram"
#include "KDChartChart"
#include "KDChartLineDiagram"
#include "KDChartPieDiagram"
#include "KDChartPolarDiagram"
#include "KDChartRingDiagram"

// KChart
#include "ChartShape.h"

using namespace KChart;
using namespace KDChart;


ChartTypeCommand::ChartTypeCommand(ChartShape* chart)
    : m_chart(chart)
    , m_oldType( BarChartType )
    , m_newType( BarChartType )
    , m_oldSubtype( NormalChartSubtype )
    , m_newSubtype( NormalChartSubtype )
{
}

ChartTypeCommand::~ChartTypeCommand()
{
}

void ChartTypeCommand::redo()
{
    //kDebug(35001) << m_newType;
 
   // save the old type
    m_oldType    = m_chart->chartType();
    m_oldSubtype = m_chart->chartSubType();
    if ( m_oldType == m_newType && m_oldSubtype == m_newSubtype )
        return;

    // Actually do the work
    m_chart->setChartType( m_newType );
    m_chart->setChartSubType( m_newSubtype );
}

void ChartTypeCommand::undo()
{
    if ( m_oldType == m_newType && m_oldSubtype == m_newSubtype )
        return;

    //kDebug(35001) << m_oldType;
    m_chart->setChartType( m_oldType );
    m_chart->setChartSubType( m_oldSubtype );
}


void ChartTypeCommand::setChartType(ChartType type, ChartSubtype subtype)
{
    m_newType    = type;
    m_newSubtype = subtype;

    switch (type) {
    case BarChartType:
        setText(i18n("Bar Chart"));
        break;
    case LineChartType:
        setText(i18n("Line Chart"));
        break;
    case AreaChartType:
        setText(i18n("Area Chart"));
        break;
    case CircleChartType:
        setText(i18n("Circle Chart"));
        break;
    case RingChartType:
        setText(i18n("Ring Chart"));
        break;
    case ScatterChartType:
        setText(i18n("Scatter Chart"));
        break;
    case RadarChartType:
        setText(i18n("Radar Chart"));
        break;
    case StockChartType:
        setText(i18n("Stock Chart"));
        break;
    case BubbleChartType:
        setText(i18n("Bubble Chart"));
        break;
    case SurfaceChartType:
        setText(i18n("Surface Chart"));
        break;
    case GanttChartType:
        setText(i18n("Gantt Chart"));
        break;
    case LastChartType:
    default:
        break;
    }
}
