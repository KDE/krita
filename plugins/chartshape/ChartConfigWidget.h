/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008 Inge Wallin    <inge@lysator.liu.se>

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


#ifndef KCHART_CHART_CONFIG_WIDGET
#define KCHART_CHART_CONFIG_WIDGET


// Calligra
#include <KoShapeConfigWidgetBase.h>

// KDChart

// KChart
#include "ChartShape.h"
#include "ui_ChartConfigWidget.h"


class KoShape;

namespace KDChart
{
    class Position;
    class CartesianAxis;
}

namespace KChart
{
class ChartShape;

/**
 * Chart type configuration widget.
 */
class CHARTSHAPELIB_EXPORT ChartConfigWidget : public KoShapeConfigWidgetBase
{
    Q_OBJECT

public:
    ChartConfigWidget();
    ~ChartConfigWidget();

    void open(KoShape* shape);
    void save();
    KAction* createAction();

    /// reimplemented 
    virtual bool showOnShapeCreate() { return true; }

    /// Delete all open dialogs.
    /// This is called when e.g. the tool is deactivated.
    void deleteSubDialogs();

    void updateMarkers();

public slots:
    void chartTypeSelected(QAction *action);
    void setThreeDMode(bool threeD);
    void update();

    void slotShowTableEditor();
    void slotShowCellRegionDialog();
    void slotShowFormatErrorBarDialog();

    void dataSetChartTypeSelected(QAction *action);
    void datasetMarkerSelected(QAction *action);
    void datasetBrushSelected(const QColor& color);
    void datasetPenSelected(const QColor& color);
    void ui_datasetShowCategoryChanged(bool b);
    void ui_datasetShowErrorBarChanged(bool b);
    void ui_dataSetShowNumberChanged(bool b);
    void ui_datasetShowPercentChanged(bool b);
    void ui_datasetShowSymbolChanged(bool b);
    void ui_dataSetSelectionChanged(int index);
    void ui_dataSetAxisSelectionChanged(int index);
    void ui_dataSetXDataRegionChanged();
    void ui_dataSetYDataRegionChanged();
    void ui_dataSetCustomDataRegionChanged();
    void ui_dataSetLabelDataRegionChanged();
    void ui_dataSetCategoryDataRegionChanged();
    void ui_dataSetSelectionChanged_CellRegionDialog(int index);
    void ui_dataSetHasChartTypeChanged(bool b);
    void ui_dataSetPieExplodeFactorChanged(int percent);
    void ui_dataSetErrorBarTypeChanged();

    void setLegendOrientationIsVertical(bool);
    void setLegendOrientation(int boxEntryIndex);
    void setLegendAlignment(int boxEntryIndex);
    void setLegendFixedPosition(int buttonGroupIndex);
    //void setLegendShowTitle(bool toggled);
    void ui_legendEditFontButtonClicked();
    void ui_legendFontChanged();
    void updateFixedPosition(Position position);
    
    void ui_axisSelectionChanged(int index);
    void ui_axisTitleChanged(const QString& title);
    void ui_axisShowTitleChanged(bool b);
    void ui_axisShowGridLinesChanged(bool b);
    void ui_axisUseLogarithmicScalingChanged(bool b);
    void ui_axisStepWidthChanged(double width);
    void ui_axisUseAutomaticStepWidthChanged(bool b);
    void ui_axisSubStepWidthChanged(double width);
    void ui_axisUseAutomaticSubStepWidthChanged(bool b);
    void ui_axisScalingButtonClicked();
    void ui_axisAdded();
    void ui_addAxisClicked();
    void ui_removeAxisClicked();
    void ui_axisEditFontButtonClicked();
    void ui_axisLabelsFontChanged();

signals:
    void chartTypeChanged(ChartType type, ChartSubtype subType);
    void chartSubTypeChanged(ChartSubtype subType);
    void dataSetChartTypeChanged(DataSet *dataSet, ChartType type);
    void dataSetChartSubTypeChanged(DataSet *dataSet, ChartSubtype subType);
    void threeDModeToggled(bool threeD);
    void showTitleChanged(bool);
    void showSubTitleChanged(bool);
    void showFooterChanged(bool);
    
    void showVerticalLinesChanged(bool b);
    void showHorizontalLinesChanged(bool b);
    
    void dataSetXDataRegionChanged(DataSet *dataSet, const CellRegion &region);
    void dataSetYDataRegionChanged(DataSet *dataSet, const CellRegion &region);
    void dataSetCustomDataRegionChanged(DataSet *dataSet, const CellRegion &region);
    void dataSetCategoryDataRegionChanged(DataSet *dataSet, const CellRegion &region);
    void dataSetLabelDataRegionChanged(DataSet *dataSet, const CellRegion &region);
    
    void datasetPenChanged(DataSet *dataSet, const QColor& color);
    void datasetBrushChanged(DataSet *dataSet, const QColor& color);
    void dataSetMarkerChanged(DataSet *dataSet, OdfMarkerStyle style);
    void datasetShowCategoryChanged(DataSet *dataSet, bool b);
    void dataSetShowNumberChanged(DataSet *dataSet, bool b);
    void datasetShowPercentChanged(DataSet *dataSet, bool b);
    void datasetShowSymbolChanged(DataSet *dataSet, bool b);
    void dataSetAxisChanged(DataSet *dataSet, Axis *axis);
    void gapBetweenBarsChanged(int percent);
    void gapBetweenSetsChanged(int percent);
    void pieExplodeFactorChanged(DataSet *dataSet, int percent);
    
    void showLegendChanged(bool b);

    void axisAdded(AxisDimension, const QString& title);
    void axisRemoved(Axis *axis);
    void axisShowTitleChanged(Axis *axis, bool b);
    void axisTitleChanged(Axis *axis, const QString& title);
    void axisShowGridLinesChanged(Axis *axis, bool b);
    void axisUseLogarithmicScalingChanged(Axis *axis, bool b);
    void axisStepWidthChanged(Axis *axis, qreal width);
    void axisSubStepWidthChanged(Axis *axis, qreal width);
    void axisUseAutomaticStepWidthChanged(Axis *axis, bool automatic);
    void axisUseAutomaticSubStepWidthChanged(Axis *axis, bool automatic);

    void legendTitleChanged(const QString&);
    void legendFontChanged(const QFont& font);
    void legendTitleFontChanged(const QFont& font);
    void legendFontSizeChanged(int size);
    void legendSpacingChanged(int spacing);
    void legendShowLinesToggled(bool toggled);
    void legendOrientationChanged(Qt::Orientation orientation);
    void legendAlignmentChanged(Qt::Alignment alignment);
    void legendFixedPositionChanged(Position position);
    void legendBackgroundColorChanged(const QColor& color);
    void legendFrameColorChanged(const QColor& color);
    void legendShowFrameChanged(bool show);

private:
    void setupDialogs();
    void createActions();
    
    void setPolarChartTypesEnabled(bool enabled);
    void setCartesianChartTypesEnabled(bool enabled);

    class Private;
    Private * const d;
};

}  // namespace KChart


#endif // KCHART_CHART_TYPE_CONFIG_WIDGET
