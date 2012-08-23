/* This file is part of the KDE project
   Copyright 2012 Brijesh Patel <brijesh3105@gmail.com>

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

#ifndef KCHART_DATASET_COMMAND
#define KCHART_DATASET_COMMAND

// Qt
#include <kundo2command.h>

// KChart
#include "kchart_global.h"


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

class DataSet;
class ChartShape;
class Axis;

class DatasetCommand : public KUndo2Command
{
public:
    /**
     * Constructor.
     */
    DatasetCommand(DataSet* dataSet, ChartShape* chart);

    /**
     * Destructor.
     */
    virtual ~DatasetCommand();

    /**
     * Executes the actual operation.
     */
    virtual void redo();

    /**
     * Executes the actual operation in reverse order.
     */
    virtual void undo();

    void setDataSetChartType(ChartType type, ChartSubtype subtype);

    void setDataSetShowCategory(bool show);
    void setDataSetShowNumber(bool show);
    void setDataSetShowPercent(bool show);
    void setDataSetShowSymbol(bool show);

    void setDataSetPen(const QColor& color);
    void setDataSetBrush(const QColor& color);
    void setDataSetMarker(OdfMarkerStyle style);
    void setDataSetAxis(Axis *axis);

private:
    DataSet *m_dataSet;
    ChartShape *m_chart;

    ChartType m_oldType;
    ChartType m_newType;
    ChartSubtype m_oldSubtype;
    ChartSubtype m_newSubtype;

    bool m_oldShowCategory;
    bool m_newShowCategory;
    bool m_oldShowNumber;
    bool m_newShowNumber;
    bool m_oldShowPercent;
    bool m_newShowPercent;
    bool m_oldShowSymbol;
    bool m_newShowSymbol;

    QColor m_oldPenColor;
    QColor m_newPenColor;
    QColor m_oldBrushColor;
    QColor m_newBrushColor;
    OdfMarkerStyle m_oldMarkerStyle;
    OdfMarkerStyle m_newMarkerStyle;

    Axis *m_oldAxis;
    Axis *m_newAxis;
};

} // namespace KChart

#endif // KCHART_DATASET_COMMAND
