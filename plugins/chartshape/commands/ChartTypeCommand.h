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

#ifndef KCHART_CHART_TYPE_COMMAND
#define KCHART_CHART_TYPE_COMMAND

// Qt
#include <kundo2command.h>

// KChart
#include "kchart_global.h"
//#include "ChartShape.h"


#if 0
namespace KDChart
{
class AbstractCoordinatePlane;
class AbstractDiagram;
class Chart;
}
#endif


namespace KChart
{

class ChartShape;

/**
 * Chart type replacement command.
 */
class ChartTypeCommand : public KUndo2Command
{
public:
    /**
     * Constructor.
     */
    explicit ChartTypeCommand(ChartShape *chart);

    /**
     * Destructor.
     */
    virtual ~ChartTypeCommand();

    /**
     * Executes the actual operation.
     */
    virtual void redo();

    /**
     * Executes the actual operation in reverse order.
     */
    virtual void undo();

    /**
     * Sets the new chart type.
     */
    void setChartType(ChartType type, ChartSubtype subType);


private:
    ChartShape                        *m_chart;
    //KDChart::Chart                    *m_chart;
    ChartType                       m_oldType;
    ChartType                       m_newType;
    ChartSubtype                    m_oldSubtype;
    ChartSubtype                    m_newSubtype;
    //KDChart::AbstractCoordinatePlane  *m_oldCoordinatePlane;
    //KDChart::AbstractDiagram          *m_oldDiagram;
};

} // namespace KChart

#endif // KCHART_CHART_TYPE_COMMAND
