/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <inge@lysator.liu.se>

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

#ifndef KCHART_KDCHARTCONVERTIONS_H
#define KCHART_KDCHARTCONVERTIONS_H


// KDChart
#include <KDChartEnums>
#include <KDChartCartesianAxis>

// KChart
#include "kchart_global.h"


namespace KChart
{

extern KDChart::CartesianAxis::Position PositionToKDChartAxisPosition(Position position);

extern QString PositionToString(Position position);
extern KDChartEnums::PositionValue PositionToKDChartPositionValue(Position position);
extern Position KDChartPositionValueToPosition(KDChartEnums::PositionValue position);

extern Qt::Orientation LegendExpansionToQtOrientation(LegendExpansion expansion);
extern LegendExpansion QtOrientationToLegendExpansion(Qt::Orientation orientation);

} // Namespace KChart

#endif
