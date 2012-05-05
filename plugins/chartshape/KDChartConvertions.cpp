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


// Own
#include "KDChartConvertions.h"

// KDChart
#include <KDChartEnums>
#include <KDChartCartesianAxis>


namespace KChart {

KDChart::CartesianAxis::Position PositionToKDChartAxisPosition(Position position)
{
    switch (position) {
    case BottomPosition:
        return KDChart::CartesianAxis::Bottom;
    case TopPosition:
        return KDChart::CartesianAxis::Top;
    case EndPosition:
        return KDChart::CartesianAxis::Right;
    case StartPosition:
        return KDChart::CartesianAxis::Left;
    }
    
    Q_ASSERT("Unknown KDChart::CartesianAxis::Position!");
    return KDChart::CartesianAxis::Bottom;
}

// Used to save e.g. legend-position attribute to ODF. Do not change these strings.
QString PositionToString(Position position)
{
    switch (position) {
    case StartPosition:
        return QString("start");
    case TopPosition:
        return QString("top");
    case BottomPosition:
        return QString("bottom");
    case TopStartPosition:
        return QString("top-start");
    case BottomStartPosition:
        return QString("bottom-start");
    case TopEndPosition:
        return QString("top-end");
    case BottomEndPosition:
        return QString("bottom-end");
    case EndPosition:
        return QString("end");
    case CenterPosition:
        return QString("center");
    case FloatingPosition:
        return QString();
    }
    
    Q_ASSERT("Unknown Position!");
    return QString();
}

KDChartEnums::PositionValue PositionToKDChartPositionValue(Position position)
{
    switch (position) {
    case StartPosition:
        return KDChartEnums::PositionWest;
    case TopPosition:
        return KDChartEnums::PositionNorth;
    case BottomPosition:
        return KDChartEnums::PositionSouth;
    case TopStartPosition:
        return KDChartEnums::PositionNorthWest;
    case BottomStartPosition:
        return KDChartEnums::PositionSouthWest;
    case TopEndPosition:
        return KDChartEnums::PositionNorthEast;
    case BottomEndPosition:
        return KDChartEnums::PositionSouthEast;
    case EndPosition:
        return KDChartEnums::PositionEast;
    case CenterPosition:
        return KDChartEnums::PositionCenter;
    case FloatingPosition:
        return KDChartEnums::PositionFloating;
    }
    
    Q_ASSERT("Unknown Position!");
    return KDChartEnums::PositionEast;
}

Position KDChartPositionValueToPosition(KDChartEnums::PositionValue position)
{
    switch (position) {
    case KDChartEnums::PositionNorthWest:
        return TopStartPosition;
    case KDChartEnums::PositionNorth:
        return TopPosition;
    case KDChartEnums::PositionNorthEast:
        return TopEndPosition;
    case KDChartEnums::PositionEast:
        return EndPosition;
    case KDChartEnums::PositionSouthEast:
        return BottomEndPosition;
    case KDChartEnums::PositionSouth:
        return BottomPosition;
    case KDChartEnums::PositionSouthWest:
        return BottomStartPosition;
    case KDChartEnums::PositionWest:
        return StartPosition;
    case KDChartEnums::PositionCenter:
        return CenterPosition;
    case KDChartEnums::PositionFloating:
        return FloatingPosition;
        
    // These are unsupported values
    case KDChartEnums::PositionUnknown:
        return FloatingPosition;
    }
    
    Q_ASSERT("Unknown KDChartEnums::PositionValue!");
    return FloatingPosition;
}

Qt::Orientation LegendExpansionToQtOrientation(LegendExpansion expansion)
{
    switch (expansion) {
    case WideLegendExpansion:
        return Qt::Horizontal;
    case HighLegendExpansion:
        return Qt::Vertical;
        
    // KDChart doesn't allow a balanced expansion
    case BalancedLegendExpansion:
        return Qt::Vertical;
    }
    
    Q_ASSERT("Unknown Qt::Orientation!");
    return Qt::Vertical;
}

LegendExpansion QtOrientationToLegendExpansion(Qt::Orientation orientation)
{
    switch (orientation) {
    case Qt::Horizontal:
        return WideLegendExpansion;
    case Qt::Vertical:
        return HighLegendExpansion;
    }
    
    Q_ASSERT("Unknown LegendExpansion!");
    return HighLegendExpansion;
}

} // Namespace KChart
