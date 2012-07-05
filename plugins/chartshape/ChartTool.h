/* This file is part of the KDE project
 *
 * Copyright (C) 2007      Inge Wallin  <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef KCHART_CHARTTOOL_H
#define KCHART_CHARTTOOL_H

// Calligra
#include <KoToolBase.h>

// KChart
#include "ChartShape.h"

class QAction;


namespace KChart
{


/**
 * This is the tool for the chart shape, which is a flake-based plugin.
 */

class CHARTSHAPELIB_EXPORT ChartTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit ChartTool(KoCanvasBase *canvas);
    ~ChartTool();

    /// reimplemented from superclass
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented from superclass
    virtual void mousePressEvent(KoPointerEvent *event) ;
    /// reimplemented from superclass
    virtual void mouseMoveEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented from superclass
    virtual void deactivate();
    /// reimplemented from superclass
    virtual QWidget *createOptionWidget();

private slots:
    void setChartType(ChartType type, ChartSubtype subtype);
    void setChartSubType(ChartSubtype subtype);
    void setThreeDMode(bool threeD);
    void setDataDirection(Qt::Orientation);
    void setShowTitle(bool show);
    void setShowSubTitle(bool show);
    void setShowFooter(bool show);

    // Datasets
    void setDataSetXDataRegion(DataSet *dataSet, const CellRegion &region);
    void setDataSetYDataRegion(DataSet *dataSet, const CellRegion &region);
    void setDataSetCustomDataRegion(DataSet *dataSet, const CellRegion &region);
    void setDataSetLabelDataRegion(DataSet *dataSet, const CellRegion &region);
    void setDataSetCategoryDataRegion(DataSet *dataSet, const CellRegion &region);
    
    void setDataSetChartType(DataSet *dataSet, ChartType type);
    void setDataSetChartSubType(DataSet *dataSet, ChartSubtype subType);

    void setDataSetShowCategory(DataSet *dataSet, bool b);
    void setDataSetShowNumber(DataSet *dataSet, bool b);
    void setDataSetShowPercent(DataSet *dataSet, bool b);
    void setDataSetShowSymbol(DataSet *dataSet, bool b);

    void setDataSetPen(DataSet *dataSet, const QColor& color);
    void setDataSetBrush(DataSet *dataSet, const QColor& color);
    void setDataSetMarker(DataSet *dataSet, OdfMarkerStyle style);
    void setDataSetAxis(DataSet *dataSet, Axis *axis);
    
    // Plot Area
    void setGapBetweenBars(int percent);
    void setGapBetweenSets(int percent);
    void setPieExplodeFactor(DataSet *dataSet, int percent);
    
    // Axes
    void addAxis(AxisDimension, const QString& title = "");
    void removeAxis(Axis *axis);
    void setAxisShowTitle(Axis *axis, bool show);
    void setAxisTitle(Axis *axis, const QString& title);
    void setAxisShowGridLines(Axis *axis, bool b = true);
    void setAxisUseLogarithmicScaling(Axis *axis, bool b = true);
    void setAxisStepWidth(Axis *axis, qreal width);
    void setAxisSubStepWidth(Axis *axis, qreal width);
    void setAxisUseAutomaticStepWidth(Axis *axis, bool automatic);
    void setAxisUseAutomaticSubStepWidth(Axis *axis, bool automatic);

    // Legend
    void setShowLegend(bool b);
    void setLegendTitle(const QString& title);
    void setLegendFont(const QFont& font);
    void setLegendFontSize(int size);
    void setLegendOrientation(Qt::Orientation);
    void setLegendAlignment(Qt::Alignment);
    void setLegendFixedPosition(Position position);
    void setLegendBackgroundColor(const QColor& color);
    void setLegendFrameColor(const QColor& color);
    void setLegendShowFrame(bool show);

    // Called upon shape manager's selectionChanged() signal
    void shapeSelectionChanged();

private:
    class Private;
    Private * const d;
};

} // namespace KChart


#endif // KCHART_CHARTTOOL_H
