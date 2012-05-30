/* This file is part of the KDE project

   Copyright 2007 Inge Wallin <inge@lysator.liu.se>

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


#ifndef KCHART_GLOBAL_H
#define KCHART_GLOBAL_H

namespace KChart
{

// Chart types for OpenDocument
enum ChartType {
    BarChartType,
    LineChartType,
    AreaChartType,
    CircleChartType,		// Pie in KDChart
    RingChartType,
    ScatterChartType,
    RadarChartType,		    // Polar in KDChart
    FilledRadarChartType,   // Polar in KDChart
    StockChartType,
    BubbleChartType,
    SurfaceChartType,
    GanttChartType,
    LastChartType               // Not an actual type, just a place holder
};
const int NUM_CHARTTYPES = int (LastChartType);

bool isPolar(ChartType type);
bool isCartesian(ChartType type);
int numDimensions(ChartType type);


// Chart subtypes, applicable to Bar, Line, Area, and Radar
enum ChartSubtype {
    NoChartSubtype,             // for charts with no subtypes
    NormalChartSubtype,         // For bar, line, area and radar charts
    StackedChartSubtype,
    PercentChartSubtype
};

enum AxisDimension {
    XAxisDimension,
    YAxisDimension,
    ZAxisDimension
};

struct ChartTypeOptions
{
    ChartSubtype subtype;
};

enum Position {
    StartPosition,
    TopPosition,
    EndPosition,
    BottomPosition,
    TopStartPosition,
    TopEndPosition,
    BottomStartPosition,
    BottomEndPosition,
    CenterPosition,

    FloatingPosition
};

enum LegendExpansion {
	WideLegendExpansion,
	HighLegendExpansion,
	BalancedLegendExpansion
};

enum ErrorCategory {
    NoErrorCategory,
    VarianceErrorCategory,
    StandardDeviationErrorCategory,
    StandardErrorErrorCategory,
    PercentageErrorCategory,
    ErrorMarginErrorCategory,
    ConstantErrorCategory
};

enum LabelType {
    TitleLabelType,
    SubTitleLabelType,
    FooterLabelType
};

} // Namespace KChart

#endif
