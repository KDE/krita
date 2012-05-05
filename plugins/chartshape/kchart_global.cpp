/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "kchart_global.h"

namespace KChart {

bool isPolar(ChartType type)
{
    switch (type)
    {
    case CircleChartType:
    case RingChartType:
    case RadarChartType:
    case FilledRadarChartType:
        return true;
    default:
        return false;
    }
    return false;
}

bool isCartesian(ChartType type)
{
    return !isPolar(type);
}

int numDimensions(ChartType type)
{
    int dimensions = 1;

    switch (type) {
    case BarChartType:
    case LineChartType:
    case AreaChartType:
    case CircleChartType:
    case RingChartType:
    case RadarChartType:
    case FilledRadarChartType:
        dimensions = 1;
        break;
    case ScatterChartType:
    case SurfaceChartType:
        dimensions = 2;
        break;
    case BubbleChartType:
        dimensions = 3;
        break;
    case StockChartType:
        // High, Low, Close. Also supported by KD Chart are Open, High, Low,
        // Close, but we only use the first so far.
        dimensions = 3;
        break;
    case GanttChartType:
        // FIXME: Figure out correct number of dimensions
        dimensions = 1;
        break;
    case LastChartType:
        dimensions = 1;
    }
    
    return dimensions;
}

} // namespace KChart
