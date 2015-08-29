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
#include "KChartConvertions.h"

// KChart
#include <KChartEnums>
#include <KChartCartesianAxis>


namespace KoChart {

KChart::CartesianAxis::Position PositionToKChartAxisPosition(Position position)
{
    switch (position) {
    case BottomPosition:
        return KChart::CartesianAxis::Bottom;
    case TopPosition:
        return KChart::CartesianAxis::Top;
    case EndPosition:
        return KChart::CartesianAxis::Right;
    case StartPosition:
        return KChart::CartesianAxis::Left;
    }
    
    Q_ASSERT("Unknown KChart::CartesianAxis::Position!");
    return KChart::CartesianAxis::Bottom;
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

KChartEnums::PositionValue PositionToKChartPositionValue(Position position)
{
    switch (position) {
    case StartPosition:
        return KChartEnums::PositionWest;
    case TopPosition:
        return KChartEnums::PositionNorth;
    case BottomPosition:
        return KChartEnums::PositionSouth;
    case TopStartPosition:
        return KChartEnums::PositionNorthWest;
    case BottomStartPosition:
        return KChartEnums::PositionSouthWest;
    case TopEndPosition:
        return KChartEnums::PositionNorthEast;
    case BottomEndPosition:
        return KChartEnums::PositionSouthEast;
    case EndPosition:
        return KChartEnums::PositionEast;
    case CenterPosition:
        return KChartEnums::PositionCenter;
    case FloatingPosition:
        return KChartEnums::PositionFloating;
    }
    
    Q_ASSERT("Unknown Position!");
    return KChartEnums::PositionEast;
}

Position KChartPositionValueToPosition(KChartEnums::PositionValue position)
{
    switch (position) {
    case KChartEnums::PositionNorthWest:
        return TopStartPosition;
    case KChartEnums::PositionNorth:
        return TopPosition;
    case KChartEnums::PositionNorthEast:
        return TopEndPosition;
    case KChartEnums::PositionEast:
        return EndPosition;
    case KChartEnums::PositionSouthEast:
        return BottomEndPosition;
    case KChartEnums::PositionSouth:
        return BottomPosition;
    case KChartEnums::PositionSouthWest:
        return BottomStartPosition;
    case KChartEnums::PositionWest:
        return StartPosition;
    case KChartEnums::PositionCenter:
        return CenterPosition;
    case KChartEnums::PositionFloating:
        return FloatingPosition;
        
    // These are unsupported values
    case KChartEnums::PositionUnknown:
        return FloatingPosition;
    }
    
    Q_ASSERT("Unknown KChartEnums::PositionValue!");
    return FloatingPosition;
}

Qt::Orientation LegendExpansionToQtOrientation(LegendExpansion expansion)
{
    switch (expansion) {
    case WideLegendExpansion:
        return Qt::Horizontal;
    case HighLegendExpansion:
        return Qt::Vertical;
        
    // KChart doesn't allow a balanced expansion
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

} // Namespace KoChart
