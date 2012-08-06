/* This file is part of the KDE project
 *
 * Copyright (C) 2007, 2010  Inge Wallin <inge@lysator.liu.se>
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

// Own
#include "ChartTool.h"

// Qt
#include <QAction>
#include <QGridLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QPen>
#include <QBrush>
#include <QPainter>

// KDE
#include <KLocale>
#include <KDebug>

// Calligra
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>

// KDChart
#include <KDChartChart>
#include <KDChartCartesianAxis>
#include <KDChartGridAttributes>
#include <KDChartAbstractCartesianDiagram>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartPosition>

// KChart
#include "Surface.h"
#include "PlotArea.h"
#include "Axis.h"
#include "DataSet.h"
#include "Legend.h"
#include "ChartProxyModel.h"
#include "ChartConfigWidget.h"
#include "KDChartConvertions.h"
#include "commands/ChartTypeCommand.h"


using namespace KChart;


class ChartTool::Private
{
public:
    Private();
    ~Private();

    ChartShape  *shape;
    QModelIndex  datasetSelection;
    QPen         datasetSelectionPen;
    QBrush       datasetSelectionBrush;

    void setDataSetShowLabel(DataSet *dataSet, bool *number, bool *percentage, bool *category, bool *symbol);
};

ChartTool::Private::Private()
    : shape(0)
{
}

ChartTool::Private::~Private()
{
}

ChartTool::ChartTool(KoCanvasBase *canvas)
    : KoToolBase(canvas),
      d(new Private())
{
    // Create QActions here.
#if 0
    QActionGroup *group = new QActionGroup(this);
    m_foo  = new QAction(koIcon("this-action"), i18n("Do something"), this);
    m_foo->setCheckable(true);
    group->addAction(m_foo);
    connect(m_foo, SIGNAL(toggled(bool)), this, SLOT(catchFoo(bool)));

    m_bar  = new QAction(koIcon("that-action"), i18n("Do something else"), this);
    m_bar->setCheckable(true);
    group->addAction(m_bar);
    connect(m_foo, SIGNAL(toggled(bool)), this, SLOT(catchBar(bool)));

#endif
    connect(canvas->shapeManager()->selection(), SIGNAL(selectionChanged()),
            this, SLOT(shapeSelectionChanged()));
}

ChartTool::~ChartTool()
{
    delete d;
}

void ChartTool::shapeSelectionChanged()
{
    KoShape *selectedShape = 0;
    
    // Get the chart shape that the tool is working on. 
    // Let d->shape point to it.
    d->shape = 0; // to be sure we don't deal with an old value if nothing is found
    KoSelection  *selection = canvas()->shapeManager()->selection();
    foreach (KoShape *shape, selection->selectedShapes()) {
        // Find out which type of shape that the user clicked on.
        // We support several here, since the chart shape is comprised
        // of several subshapes (plotarea, legend)
        d->shape = dynamic_cast<ChartShape*>(shape);
        if (!d->shape) {
            PlotArea *plotArea = dynamic_cast<PlotArea*>(shape);
            if (plotArea) {
                selectedShape = plotArea;
                d->shape = plotArea->parent();
            }
            else {
                Legend *legend = dynamic_cast<Legend*>(shape);
                if (legend) {
                    selectedShape = legend;
                    d->shape = dynamic_cast<ChartShape*>(shape->parent());
                }
            }
        // The selected shape is the chart
        } else
            selectedShape = shape;
        
        // Insert the values from the selected shape (note: not only
        // chart shape, but also plotarea or legend) into the tool
        // option widget.
        if (selectedShape) {
            foreach (QWidget *w, optionWidgets()) {
                KoShapeConfigWidgetBase *widget = dynamic_cast<KoShapeConfigWidgetBase*>(w);
                Q_ASSERT(widget);
                if (widget)
                    widget->open(selectedShape);
            }

        // We support only one selected chart at the time, so once
        // we found one, we don't need to search for any more
        // among the selected shapes.
        break;
        }
    }

    // If we couldn't determine a chart shape, then there is nothing to do.
    if (!d->shape) { // none found
        emit done();
        return;
    }
}


void ChartTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void ChartTool::mousePressEvent(KoPointerEvent *event)
{
#if 1  // disabled
    Q_UNUSED(event);
    return;
#else
    // Select dataset
    if (   !d->shape || !d->shape->kdChart() || ! d->shape->kdChart()->coordinatePlane()
        || !d->shape->kdChart()->coordinatePlane()->diagram())
        return;
    QPointF point = event->point - d->shape->position();
    QModelIndex selection = d->shape->kdChart()->coordinatePlane()->diagram()->indexAt(point.toPoint());
    // Note: the dataset will always stay column() due to the transformations being
    // done internally by the ChartProxyModel
    int dataset = selection.column();
    
    if (d->datasetSelection.isValid()) {
        d->shape->kdChart()->coordinatePlane()->diagram()->setPen(d->datasetSelection.column(), d->datasetSelectionPen);
        //d->shape->kdChart()->coordinatePlane()->diagram()->setBrush(d->datasetSelection, d->datasetSelectionBrush);
    }
    if (selection.isValid()) {
        d->datasetSelection = selection;
        
        QPen pen(Qt::DotLine);
        pen.setColor(Qt::darkGray);
        pen.setWidth(1);
        
        d->datasetSelectionBrush = d->shape->kdChart()->coordinatePlane()->diagram()->brush(selection);
        d->datasetSelectionPen   = d->shape->kdChart()->coordinatePlane()->diagram()->pen(dataset);
        
        d->shape->kdChart()->coordinatePlane()->diagram()->setPen(dataset, pen);
        //d->shape->kdChart()->coordinatePlane()->diagram()->setBrush(selection, QBrush(Qt::lightGray));
    }
    ((ChartConfigWidget*)optionWidget())->selectDataset(dataset);
        
    d->shape->update();
#endif
}

void ChartTool::mouseMoveEvent(KoPointerEvent *event)
{
    event->ignore();
}

void ChartTool::mouseReleaseEvent(KoPointerEvent *event)
{
    event->ignore();
}


void ChartTool::activate(ToolActivation, const QSet<KoShape*> &)
{
    useCursor(Qt::ArrowCursor);

    // cause on ChartTool::deactivate we set d->shape to NULL it is needed
    // to call shapeSelectionChanged() even if the selection did not change
    // to be sure d->shape is proper set again.
    shapeSelectionChanged();
}

void ChartTool::deactivate()
{
    d->shape = 0;

    // Tell the config widget to delete all open dialogs.
    //
    // The reason why we want to do that explicitly here is because
    // they are connected to the models, which may disappear when the
    // chart shape is destructed.
    foreach (QWidget *w, optionWidgets()) {
        ChartConfigWidget *configWidget = dynamic_cast<ChartConfigWidget*>(w);
        if (configWidget)
            configWidget->deleteSubDialogs();
    }
}


QWidget *ChartTool::createOptionWidget()
{
    ChartConfigWidget  *widget = new ChartConfigWidget();
    
    connect(widget, SIGNAL(dataSetXDataRegionChanged(DataSet*, const CellRegion&)),
            this,   SLOT(setDataSetXDataRegion(DataSet*, const CellRegion&)));
    connect(widget, SIGNAL(dataSetYDataRegionChanged(DataSet*, const CellRegion&)),
            this,   SLOT(setDataSetYDataRegion(DataSet*, const CellRegion&)));
    connect(widget, SIGNAL(dataSetCustomDataRegionChanged(DataSet*, const CellRegion&)),
            this,   SLOT(setDataSetCustomDataRegion(DataSet*, const CellRegion&)));
    connect(widget, SIGNAL(dataSetLabelDataRegionChanged(DataSet*, const CellRegion&)),
            this,   SLOT(setDataSetLabelDataRegion(DataSet*, const CellRegion&)));
    connect(widget, SIGNAL(dataSetCategoryDataRegionChanged(DataSet*, const CellRegion&)),
            this,   SLOT(setDataSetCategoryDataRegion(DataSet*, const CellRegion&)));
    connect(widget, SIGNAL(dataSetChartTypeChanged(DataSet*, ChartType)),
            this,   SLOT(setDataSetChartType(DataSet*, ChartType)));
    connect(widget, SIGNAL(dataSetChartSubTypeChanged(DataSet*, ChartSubtype)),
            this,   SLOT(setDataSetChartSubType(DataSet*, ChartSubtype)));
    connect(widget, SIGNAL(datasetBrushChanged(DataSet*, const QColor&)),
            this, SLOT(setDataSetBrush(DataSet*, const QColor&)));
    connect(widget, SIGNAL(dataSetMarkerChanged(DataSet*,OdfMarkerStyle)),
            this, SLOT(setDataSetMarker(DataSet*,OdfMarkerStyle)));
    connect(widget, SIGNAL(datasetPenChanged(DataSet*, const QColor&)),
            this, SLOT(setDataSetPen(DataSet*, const QColor&)));
    connect(widget, SIGNAL(datasetShowCategoryChanged(DataSet*, bool)),
            this, SLOT(setDataSetShowCategory(DataSet*, bool)));
    connect(widget, SIGNAL(dataSetShowNumberChanged(DataSet*, bool)),
            this, SLOT(setDataSetShowNumber(DataSet*, bool)));
    connect(widget, SIGNAL(datasetShowPercentChanged(DataSet*, bool)),
            this, SLOT(setDataSetShowPercent(DataSet*, bool)));
    connect(widget, SIGNAL(datasetShowSymbolChanged(DataSet*, bool)),
            this, SLOT(setDataSetShowSymbol(DataSet*, bool)));
    connect(widget, SIGNAL(dataSetAxisChanged(DataSet*, Axis*)),
            this, SLOT(setDataSetAxis(DataSet*, Axis*)));
    connect(widget, SIGNAL(gapBetweenBarsChanged(int)),
            this,   SLOT(setGapBetweenBars(int)));
    connect(widget, SIGNAL(gapBetweenSetsChanged(int)),
            this,   SLOT(setGapBetweenSets(int)));
    connect(widget, SIGNAL(pieExplodeFactorChanged(DataSet*, int)),
            this,   SLOT(setPieExplodeFactor(DataSet*, int)));
    
    connect(widget, SIGNAL(showLegendChanged(bool)),
            this,   SLOT(setShowLegend(bool)));

    connect(widget, SIGNAL(chartTypeChanged(ChartType, ChartSubtype)),
            this,   SLOT(setChartType(ChartType, ChartSubtype)));
    connect(widget, SIGNAL(chartSubTypeChanged(ChartSubtype)),
            this,   SLOT(setChartSubType(ChartSubtype)));
    connect(widget, SIGNAL(threeDModeToggled(bool)),
            this,   SLOT(setThreeDMode(bool)));
    connect(widget, SIGNAL(showTitleChanged(bool)),
            this,   SLOT(setShowTitle(bool)));
    connect(widget, SIGNAL(showSubTitleChanged(bool)),
            this,   SLOT(setShowSubTitle(bool)));
    connect(widget, SIGNAL(showFooterChanged(bool)),
            this,   SLOT(setShowFooter(bool)));

    connect(widget, SIGNAL(axisAdded(AxisDimension, const QString&)),
            this,   SLOT(addAxis(AxisDimension, const QString&)));
    connect(widget, SIGNAL(axisRemoved(Axis*)),
            this,   SLOT(removeAxis(Axis*)));
    connect(widget, SIGNAL(axisTitleChanged(Axis*, const QString&)),
            this,   SLOT(setAxisTitle(Axis*, const QString&)));
    connect(widget, SIGNAL(axisShowTitleChanged(Axis*, bool)),
            this,   SLOT(setAxisShowTitle(Axis*, bool)));
    connect(widget, SIGNAL(axisShowGridLinesChanged(Axis*, bool)),
            this,   SLOT(setAxisShowGridLines(Axis*, bool)));
    connect(widget, SIGNAL(axisUseLogarithmicScalingChanged(Axis*, bool)),
            this,   SLOT(setAxisUseLogarithmicScaling(Axis*, bool)));
    connect(widget, SIGNAL(axisStepWidthChanged(Axis*, qreal)),
            this,   SLOT(setAxisStepWidth(Axis*, qreal)));
    connect(widget, SIGNAL(axisSubStepWidthChanged(Axis*, qreal)),
            this,   SLOT(setAxisSubStepWidth(Axis*, qreal)));
    connect(widget, SIGNAL(axisUseAutomaticStepWidthChanged(Axis*, bool)),
            this,   SLOT(setAxisUseAutomaticStepWidth(Axis*, bool)));
    connect(widget, SIGNAL(axisUseAutomaticSubStepWidthChanged(Axis*, bool)),
            this,   SLOT(setAxisUseAutomaticSubStepWidth(Axis*, bool)));

    connect(widget, SIGNAL(legendTitleChanged(const QString&)),
            this,   SLOT(setLegendTitle(const QString&)));
    connect(widget, SIGNAL(legendFontChanged(const QFont&)),
            this,   SLOT(setLegendFont(const QFont&)));
    connect(widget, SIGNAL(legendFontSizeChanged(int)),
            this,   SLOT(setLegendFontSize(int)));

    connect(widget, SIGNAL(legendOrientationChanged(Qt::Orientation)),
            this,   SLOT(setLegendOrientation(Qt::Orientation)));
    connect(widget, SIGNAL(legendAlignmentChanged(Qt::Alignment)),
            this,   SLOT(setLegendAlignment(Qt::Alignment)));

    connect(widget, SIGNAL(legendFixedPositionChanged(Position)),
            this,   SLOT(setLegendFixedPosition(Position)));
    
    connect(widget, SIGNAL(legendBackgroundColorChanged(const QColor&)) ,
            this,   SLOT(setLegendBackgroundColor(const QColor&)));
    connect(widget, SIGNAL(legendFrameColorChanged(const QColor&)) ,
            this,   SLOT(setLegendFrameColor(const QColor&)));
    connect(widget, SIGNAL(legendShowFrameChanged(bool)) ,
            this,   SLOT(setLegendShowFrame(bool)));

    connect(d->shape, SIGNAL(updateConfigWidget()),
            widget,     SLOT(update()));


    return widget;
}


void ChartTool::setChartType(ChartType type, ChartSubtype subtype)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;
    
    ChartTypeCommand *command = new ChartTypeCommand(d->shape);
    if (command!=0) {
        command->setChartType(type, subtype);
        canvas()->addCommand(command);
    }

    foreach (QWidget *w, optionWidgets())
        w->update();
}


void ChartTool::setChartSubType(ChartSubtype subtype)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->setChartSubType(subtype);
    d->shape->update();
}


void ChartTool::setDataSetXDataRegion(DataSet *dataSet, const CellRegion &region)
{
    if (!dataSet)
        return;

    dataSet->setXDataRegion(region);
}

void ChartTool::setDataSetYDataRegion(DataSet *dataSet, const CellRegion &region)
{
    if (!dataSet)
        return;

    dataSet->setYDataRegion(region);
}

void ChartTool::setDataSetCustomDataRegion(DataSet *dataSet, const CellRegion &region)
{
    if (!dataSet)
        return;

    dataSet->setCustomDataRegion(region);
}

void ChartTool::setDataSetLabelDataRegion(DataSet *dataSet, const CellRegion &region)
{
    if (!dataSet)
        return;

    dataSet->setLabelDataRegion(region);
}

void ChartTool::setDataSetCategoryDataRegion(DataSet *dataSet, const CellRegion &region)
{
    if (!dataSet)
        return;

    dataSet->setCategoryDataRegion(region);
}


void ChartTool::setDataSetChartType(DataSet *dataSet, ChartType type)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(dataSet);
    if (dataSet)
        dataSet->setChartType(type);
    d->shape->update();
    d->shape->legend()->update();
}

void ChartTool::setDataSetChartSubType(DataSet *dataSet, ChartSubtype subType)
{
    Q_ASSERT(dataSet);
    if (dataSet)
        dataSet->setChartSubType(subType);
    d->shape->update();
}


void ChartTool::setDataSetBrush(DataSet *dataSet, const QColor& color)
{
    Q_ASSERT(d->shape);
    if (!dataSet)
        return;

    dataSet->setBrush(QBrush(color));
    d->shape->update();
}
void ChartTool::setDataSetPen(DataSet *dataSet, const QColor& color)
{
    Q_ASSERT(d->shape);
    if (!dataSet)
        return;

    dataSet->setPen(QPen(color));
    d->shape->update();
}

void ChartTool::setDataSetMarker(DataSet *dataSet, OdfMarkerStyle style)
{
    Q_ASSERT(d->shape);
    if (!dataSet)
        return;

    dataSet->setMarkerStyle(style);
    d->shape->update();
}
void ChartTool::setDataSetAxis(DataSet *dataSet, Axis *axis)
{
    Q_ASSERT(d->shape);
    if (!dataSet || !axis)
        return;

    dataSet->attachedAxis()->detachDataSet(dataSet);
    axis->attachDataSet(dataSet);
    d->shape->update();
}

void ChartTool::Private::setDataSetShowLabel(DataSet *dataSet, bool *number, bool *percentage, bool *category, bool *symbol)
{
    Q_ASSERT(shape);
    if (!dataSet)
        return;

    DataSet::ValueLabelType type = dataSet->valueLabelType();
    if (number) type.number = *number;
    if (percentage) type.percentage = *percentage;
    if (category) type.category = *category;
    if (symbol) type.symbol = *symbol;
    dataSet->setValueLabelType(type);

    // its necessary to set this for all data value
    //TODO we need to allow to differ in the UI between the datasets vs
    //     the global setting and then allow to edit them separatly.
    for (int i = 0; i < dataSet->size(); ++i) {
        DataSet::ValueLabelType type = dataSet->valueLabelType(i);
        if (number) type.number = *number;
        if (percentage) type.percentage = *percentage;
        if (category) type.category = *category;
        if (symbol) type.symbol = *symbol;
        dataSet->setValueLabelType(type, i);
    }

    shape->update();
}

void ChartTool::setDataSetShowCategory(DataSet *dataSet, bool b)
{
    d->setDataSetShowLabel(dataSet, 0, 0, &b, 0);
}

void ChartTool::setDataSetShowNumber(DataSet *dataSet, bool b)
{
    d->setDataSetShowLabel(dataSet, &b, 0, 0, 0);
}

void ChartTool::setDataSetShowPercent(DataSet *dataSet, bool b)
{
    d->setDataSetShowLabel(dataSet, 0, &b, 0, 0);
}

void ChartTool::setDataSetShowSymbol(DataSet *dataSet, bool b)
{
    d->setDataSetShowLabel(dataSet, 0, 0, 0, &b);
}

void ChartTool::setThreeDMode(bool threeD)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->setThreeD(threeD);
    d->shape->update();
}

void ChartTool::setShowTitle(bool show)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->showTitle(show);
    d->shape->update();
}

void ChartTool::setShowSubTitle(bool show)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->showSubTitle(show);
    d->shape->update();
}

void ChartTool::setShowFooter(bool show)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->showFooter(show);
    d->shape->update();
}

void ChartTool::setDataDirection(Qt::Orientation direction)
{
    Q_ASSERT(d->shape);
    if (!d->shape)
        return;

    d->shape->proxyModel()->setDataDirection(direction);
    d->shape->relayout();
}


void ChartTool::setLegendTitle(const QString &title)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setTitle(title);
    d->shape->legend()->update();
}

void ChartTool::setLegendFont(const QFont &font)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    // There only is a general font, for the legend items and the legend title
    d->shape->legend()->setFont(font);
    d->shape->legend()->update();
}

void ChartTool::setLegendFontSize(int size)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setFontSize(size);
    d->shape->legend()->update();
}

void ChartTool::setLegendOrientation(Qt::Orientation orientation)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setExpansion(QtOrientationToLegendExpansion(orientation));
    d->shape->legend()->update();
}

void ChartTool::setLegendAlignment(Qt::Alignment alignment)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setAlignment(alignment);
    d->shape->legend()->update();
}

void ChartTool::setLegendFixedPosition(Position position)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setLegendPosition(position);

    foreach (QWidget *w, optionWidgets()) {
        ((ChartConfigWidget*) w)->updateFixedPosition(position);
    }

    d->shape->legend()->update();
}

void ChartTool::setLegendBackgroundColor(const QColor& color)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setBackgroundColor(color);
    d->shape->legend()->update();
}

void ChartTool::setLegendFrameColor(const QColor& color)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setFrameColor(color);
    d->shape->legend()->update();
}

void ChartTool::setLegendShowFrame(bool show)
{
    Q_ASSERT(d->shape);
    Q_ASSERT(d->shape->legend());

    d->shape->legend()->setShowFrame(show);
    d->shape->legend()->update();
}


void ChartTool::addAxis(AxisDimension dimension, const QString& title)
{
    Q_ASSERT(d->shape);

    Axis *axis = new Axis(d->shape->plotArea(), dimension);
    axis->setTitleText(title);
    d->shape->update();
}

void ChartTool::removeAxis(Axis *axis)
{
    Q_ASSERT(d->shape);

    d->shape->plotArea()->removeAxis(axis);
    d->shape->update();
}

void ChartTool::setAxisTitle(Axis *axis, const QString& title)
{
    axis->setTitleText(title);
    d->shape->update();
}

void ChartTool::setAxisShowTitle(Axis *axis, bool show)
{
    Q_ASSERT(d->shape);

    axis->title()->setVisible(show);
    d->shape->update();
}

void ChartTool::setAxisShowGridLines(Axis *axis, bool b)
{
    axis->setShowMajorGrid(b);
    axis->setShowMinorGrid(b);
    d->shape->update();
}

void ChartTool::setAxisUseLogarithmicScaling(Axis *axis, bool b)
{
    axis->setScalingLogarithmic(b);
    d->shape->update();
}

void ChartTool::setAxisStepWidth(Axis *axis, qreal width)
{
    axis->setMajorInterval(width);
    d->shape->update();
}

void ChartTool::setAxisSubStepWidth(Axis *axis, qreal width)
{
    axis->setMinorInterval(width);
    d->shape->update();
}

void ChartTool::setAxisUseAutomaticStepWidth(Axis *axis, bool automatic)
{
    axis->setUseAutomaticMajorInterval(automatic);
    d->shape->update();
}

void ChartTool::setAxisUseAutomaticSubStepWidth(Axis *axis, bool automatic)
{
    axis->setUseAutomaticMinorInterval(automatic);
    d->shape->update();
}


void ChartTool::setGapBetweenBars(int percent)
{
    Q_ASSERT(d->shape);

    d->shape->plotArea()->setGapBetweenBars(percent);
    d->shape->update();
}

void ChartTool::setGapBetweenSets(int percent)
{
    Q_ASSERT(d->shape);

    d->shape->plotArea()->setGapBetweenSets(percent);
    d->shape->update();
}

void ChartTool::setPieExplodeFactor(DataSet *dataSet, int percent)
{
    Q_ASSERT(d->shape);

    dataSet->setPieExplodeFactor(percent);
    d->shape->update();
}

void ChartTool::setShowLegend(bool b)
{
    Q_ASSERT(d->shape);

    d->shape->legend()->setVisible(b);
    d->shape->legend()->update();
}

#include "ChartTool.moc"
