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
#include "Legend.h"

using namespace KChart;
using namespace KDChart;


ChartTypeCommand::ChartTypeCommand(ChartShape* chart)
    : m_chart(chart)
    , m_oldType(BarChartType)
    , m_newType(BarChartType)
    , m_oldSubtype(NormalChartSubtype)
    , m_newSubtype(NormalChartSubtype)
{
}

ChartTypeCommand::~ChartTypeCommand()
{
}

void ChartTypeCommand::redo()
{
    // save the old type
    m_oldType    = m_chart->chartType();
    m_oldSubtype = m_chart->chartSubType();
    if (m_oldType == m_newType && m_oldSubtype == m_newSubtype)
        return;


    // Actually do the work
    m_chart->setChartType(m_newType);
    m_chart->setChartSubType(m_newSubtype);
    m_chart->update();
    m_chart->legend()->update();
}

void ChartTypeCommand::undo()
{
    if (m_oldType == m_newType && m_oldSubtype == m_newSubtype)
        return;

    m_chart->setChartType(m_oldType);
    m_chart->setChartSubType(m_oldSubtype);
    m_chart->update();
    m_chart->legend()->update();
}


void ChartTypeCommand::setChartType(ChartType type, ChartSubtype subtype)
{
    m_newType    = type;
    m_newSubtype = subtype;

    switch(type) {
    case BarChartType:
        switch(subtype) {
        case NormalChartSubtype:
            setText(i18nc("(qtundo-format)", "Normal Bar Chart"));
            break;
        case StackedChartSubtype:
            setText(i18nc("(qtundo-format)", "Stacked Bar Chart"));
            break;
        case PercentChartSubtype:
            setText(i18nc("(qtundo-format)", "Percent Bar Chart"));
            break;
        default:
            Q_ASSERT("Invalid bar chart subtype!");
        }
        break;
    case LineChartType:
        switch(subtype) {
        case NormalChartSubtype:
            setText(i18nc("(qtundo-format)", "Normal Line Chart"));
            break;
        case StackedChartSubtype:
            setText(i18nc("(qtundo-format)", "Stacked Line Chart"));
            break;
        case PercentChartSubtype:
            setText(i18nc("(qtundo-format)", "Percent Line Chart"));
            break;
        default:
            Q_ASSERT("Invalid line chart subtype!");
        }
        break;
    case AreaChartType:
        switch(subtype) {
        case NormalChartSubtype:
            setText(i18nc("(qtundo-format)", "Normal Area Chart"));
            break;
        case StackedChartSubtype:
            setText(i18nc("(qtundo-format)", "Stacked Area Chart"));
            break;
        case PercentChartSubtype:
            setText(i18nc("(qtundo-format)", "Percent Area Chart"));
            break;
        default:
            Q_ASSERT("Invalid area chart subtype!");
        }
        break;
    case CircleChartType:
        setText(i18nc("(qtundo-format)", "Circle Chart"));
        break;
    case RingChartType:
        setText(i18nc("(qtundo-format)", "Ring Chart"));
        break;
    case ScatterChartType:
        setText(i18nc("(qtundo-format)", "Scatter Chart"));
        break;
    case RadarChartType:
        setText(i18nc("(qtundo-format)", "Radar Chart"));
        break;
    case FilledRadarChartType:
        setText(i18nc("(qtundo-format)", "Filled Radar Chart"));
        break;
    case StockChartType:
        setText(i18nc("(qtundo-format)", "Stock Chart"));
        break;
    case BubbleChartType:
        setText(i18nc("(qtundo-format)", "Bubble Chart"));
        break;
    case SurfaceChartType:
        setText(i18nc("(qtundo-format)", "Surface Chart"));
        break;
    case GanttChartType:
        setText(i18nc("(qtundo-format)", "Gantt Chart"));
        break;
    case LastChartType:
    default:
        break;
    }
}
