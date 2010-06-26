/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2008-2009 Inge Wallin    <inge@lysator.liu.se>
   Copyright (C) 2010 Carlos Licea    <carlos@kdab.com>
   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
     Contact: Suresh Chande suresh.chande@nokia.com

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
#include "DataSet.h"

// Qt
#include <QAbstractItemModel>
#include <QString>
#include <QPen>
#include <QColor>

// KDE
#include <KLocale>

// KDChart
#include <KDChartDataValueAttributes>
#include <KDChartPieAttributes>
#include <KDChartTextAttributes>
#include <KDChartRelativePosition>
#include <KDChartPosition>
#include <KDChartAbstractDiagram>
#include <KDChartMeasure>
#include "KDChartModel.h"

// KChart
#include "ChartProxyModel.h"
#include "Axis.h"
#include "PlotArea.h"
#include "Surface.h"

// KOffice
#include <KoXmlNS.h>
#include <KoOdfGraphicStyles.h>
#include <KoGenStyle.h>
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfWorkaround.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>

using namespace KChart;

class DataSet::Private
{
public:
    Private( DataSet *parent );
    ~Private();

    void         updateSize();
    bool         hasOwnChartType() const;
    ChartType    effectiveChartType() const;
    bool         isValidDataPoint( const QPoint &point ) const;
    QVariant     data( const CellRegion &region, int index ) const;

    QBrush defaultBrush() const;
    QBrush defaultBrush( int section ) const;

    QPen defaultPen() const;

    // Determines what sections of a cell region lie in rect
    void sectionsInRect( const CellRegion &region, const QRect &rect,
                         int &start, int &end ) const;
    void dataChanged( KDChartModel::DataRole role, const QRect &rect ) const;

    DataSet      *parent;

    ChartType     chartType;
    ChartSubtype  chartSubType;

    Axis *attachedAxis;
    bool showMeanValue;
    QPen meanValuePen;
    bool showLabels;
    bool showLowerErrorIndicator;
    bool showUpperErrorIndicator;
    QPen errorIndicatorPen;
    ErrorCategory errorCategory;
    qreal errorPercentage;
    qreal errorMargin;
    qreal lowerErrorLimit;
    qreal upperErrorLimit;
    // Determines whether pen has been set
    bool penIsSet;
    // Determines whether brush has been set
    bool brushIsSet;
    QPen pen;
    QBrush brush;

    // Returns an instance of DataValueAttributes with sane default values in
    // relation to KChart
    KDChart::DataValueAttributes defaultDataValueAttributes();

    KDChart::PieAttributes pieAttributes;
    KDChart::DataValueAttributes dataValueAttributes;

    QMap<int, QPen> pens;
    QMap<int, QBrush> brushes;
    QMap<int, KDChart::PieAttributes> sectionsPieAttributes;
    QMap<int, KDChart::DataValueAttributes> sectionsDataValueAttributes;
    QMap<int, bool> sectionsShowLabels;

    int num;

    // The different CellRegions for a dataset
    // Note: These are all 1-dimensional, i.e. vectors.
    CellRegion labelDataRegion; // one cell that holds the label
    CellRegion yDataRegion;     // normal y values
    CellRegion xDataRegion;     // x values -- only for scatter & bubble charts
    CellRegion customDataRegion;// used for bubble width in bubble charts
    // FIXME: Remove category region from DataSet - this is not the place
    // it belongs to.
    CellRegion categoryDataRegion; // x labels -- same for all datasets

    ChartProxyModel *model;
    KDChart::AbstractDiagram *kdDiagram;
    int kdDataSetNumber;

    KDChartModel *kdChartModel;

    int size;
    bool blockSignals;
};

DataSet::Private::Private( DataSet *parent )
{
    this->parent = parent;
    num = -1;
    chartType = LastChartType;
    chartSubType = NoChartSubtype;
    kdChartModel = 0;
    kdDataSetNumber = -1;
    showMeanValue = false;
    showLabels = false;
    showLowerErrorIndicator = false;
    showUpperErrorIndicator = false;
    errorPercentage = 0.0;
    errorMargin = 0.0;
    lowerErrorLimit = 0.0;
    upperErrorLimit = 0.0;
    brush = QColor( Qt::white );
    pen = QPen( Qt::black );
    kdDiagram = 0;
    attachedAxis = 0;
    size = 0;
    blockSignals = false;
    penIsSet = false;
    brushIsSet = false;
    dataValueAttributes = defaultDataValueAttributes();
}

DataSet::Private::~Private()
{
}

KDChart::DataValueAttributes DataSet::Private::defaultDataValueAttributes()
{
    KDChart::DataValueAttributes attr;
    KDChart::TextAttributes textAttr = attr.textAttributes();
    KDChart::Measure fontSize = textAttr.fontSize();
    fontSize.setValue( 10 );
    // Don't change font size with chart size
    fontSize.setCalculationMode( KDChartEnums::MeasureCalculationModeAbsolute );
    textAttr.setFontSize( fontSize );
    // Draw text horizontally
    textAttr.setRotation( 0 );
    attr.setTextAttributes( textAttr );
    // Set positive value position
    KDChart::RelativePosition positivePosition = attr.positivePosition();
    positivePosition.setAlignment( Qt::AlignCenter | Qt::AlignBottom );
    positivePosition.setReferencePosition( KDChartEnums::PositionNorth );
    positivePosition.setHorizontalPadding( 0.0 );
    positivePosition.setVerticalPadding( -100.0 );
    attr.setPositivePosition( positivePosition );
    // Set negative value position
    KDChart::RelativePosition negativePosition = attr.negativePosition();
    negativePosition.setAlignment( Qt::AlignCenter | Qt::AlignTop );
    negativePosition.setReferencePosition( KDChartEnums::PositionSouth );
    negativePosition.setHorizontalPadding( 0.0 );
    negativePosition.setVerticalPadding( 100.0 );
    attr.setNegativePosition( negativePosition );
    // No decimal digits by default
    attr.setDecimalDigits( 0 );
    // Show all values, even if they overlap
    attr.setShowOverlappingDataLabels( true );
    // Yes, data point labels can repeatedly have the same text. (e.g. the same value)
    attr.setShowRepetitiveDataLabels( true );

    return attr;
}

void DataSet::Private::updateSize()
{
    int newSize = 0;
    newSize = qMax( newSize, xDataRegion.cellCount() );
    newSize = qMax( newSize, yDataRegion.cellCount() );
    newSize = qMax( newSize, customDataRegion.cellCount() );
    newSize = qMax( newSize, categoryDataRegion.cellCount() );

    if ( size != newSize ) {
        size = newSize;
        if ( !blockSignals && kdChartModel )
            kdChartModel->dataSetSizeChanged( parent, size );
    }
}

bool DataSet::Private::hasOwnChartType() const
{
    return chartType != LastChartType;
}

/**
 * Returns the effective chart type of this data set, i.e.
 * returns the chart type of the diagram this data set is
 * attached to if no chart type is set, or otherwise this data
 * set's chart type.
 */
ChartType DataSet::Private::effectiveChartType() const
{
    if ( hasOwnChartType() )
        return chartType;

    Q_ASSERT( attachedAxis );
    return attachedAxis->plotArea()->chartType();
}

bool DataSet::Private::isValidDataPoint( const QPoint &point ) const
{
    if ( point.y() < 0 || point.x() < 0 )
        return false;

    // We can't point to horizontal and vertical header data at the same time
    if ( point.x() == 0 && point.y() == 0 )
        return false;

    return true;
}

QVariant DataSet::Private::data( const CellRegion &region, int index ) const
{
    if ( !region.isValid() )
        return QVariant();

    QAbstractItemModel *model = this->model->sourceModel();
    if ( !model )
        return QVariant();

    // The result
    QVariant data;

    // Convert the given index in this dataset to a data point in the
    // source model.
    QPoint dataPoint = region.pointAtIndex( index );

    // FIXME: Why not use this immediately if true?
    const bool verticalHeaderData   = dataPoint.x() == 0;
    const bool horizontalHeaderData = dataPoint.y() == 0;

    // Check if the data point is valid
    const bool validDataPoint = isValidDataPoint( dataPoint );

    // Remove, since it makes kspread crash when inserting a chart for
    // a 1x1 cell region.
    //Q_ASSERT( validDataPoint );
    if ( !validDataPoint )
        return QVariant();

    // The top-left point is (1,1). (0,y) or (x,0) refers to header data.
    const int row = dataPoint.y() - 1;
    const int col = dataPoint.x() - 1;

    if ( verticalHeaderData )
        data = model->headerData( row, Qt::Vertical );
    else if ( horizontalHeaderData )
        data = model->headerData( col, Qt::Horizontal );
    else {
        const QModelIndex &index = model->index( row, col );
        // FIXME: This causes a crash in KSpread when a document is loaded.
        //Q_ASSERT( index.isValid() );
        if ( index.isValid() )
            data = model->data( index );
    }

    return data;
}

void DataSet::Private::sectionsInRect( const CellRegion &region, const QRect &rect,
                                       int &start, int &end ) const
{
    QVector<QRect> dataRegions = region.rects();

    start = -1;
    end = -1;

    if ( region.orientation() == Qt::Horizontal ) {
        QPoint  topLeft  = rect.topLeft();
        QPoint  topRight = rect.topRight();

        int totalWidth = 0;
        int i;

        for ( i = 0; i < dataRegions.size(); i++ ) {
            if ( dataRegions[i].contains( topLeft ) ) {
                start = totalWidth + topLeft.x() - dataRegions[i].topLeft().x();
                break;
            }
            totalWidth += dataRegions[i].width();
        }

        for ( ; i < dataRegions.size(); i++ ) {
            if ( dataRegions[i].contains( topRight ) ) {
                end = totalWidth + topRight.x() - dataRegions[i].topLeft().x();
                break;
            }

            totalWidth += dataRegions[i].width();
        }
    }
    else {
        QPoint  topLeft    = rect.topLeft();
        QPoint  bottomLeft = rect.bottomLeft();

        int totalHeight = 0;
        int i;
        for ( i = 0; i < dataRegions.size(); i++ ) {
            if ( dataRegions[i].contains( topLeft ) ) {
                start = totalHeight + topLeft.y() - dataRegions[i].topLeft().y();
                break;
            }

            totalHeight += dataRegions[i].height();
        }

        for ( ; i < dataRegions.size(); i++ ) {
            if ( dataRegions[i].contains( bottomLeft ) ) {
                end = totalHeight + bottomLeft.y() - dataRegions[i].topLeft().y();
                break;
            }

            totalHeight += dataRegions[i].height();
        }
    }
}

QBrush DataSet::Private::defaultBrush() const
{
    Q_ASSERT( kdDiagram );
    Qt::Orientation modelDataDirection = kdChartModel->dataDirection();
    if ( modelDataDirection == Qt::Vertical )
        return defaultDataSetColor( kdDataSetNumber );
    // FIXME: What to return in the other case?
    return QBrush();
}

QBrush DataSet::Private::defaultBrush( int section ) const
{
    Q_ASSERT( kdDiagram );
    Qt::Orientation modelDataDirection = kdChartModel->dataDirection();
    // Horizontally aligned diagrams have a specific color per category
    // See for example pie or ring charts. A pie chart contains a single
    // data set, but the slices default to different brushes.
    if ( modelDataDirection == Qt::Horizontal )
        return defaultDataSetColor( section );
    // Vertically aligned diagrams default to one brush per data set
    return defaultBrush();
}

QPen DataSet::Private::defaultPen() const
{
    QPen pen( Qt::black );
    ChartType chartType = effectiveChartType();
    if ( chartType == LineChartType ||
         chartType == ScatterChartType )
        pen = QPen( defaultDataSetColor( kdDataSetNumber ) );

    return pen;
}


DataSet::DataSet( ChartProxyModel *proxyModel )
    : d( new Private( this ) )
{
    d->model = proxyModel;
}

DataSet::~DataSet()
{
    if ( d->attachedAxis )
        d->attachedAxis->detachDataSet( this, true );

    delete d;
}


ChartType DataSet::chartType() const
{
    return d->chartType;
}

ChartSubtype DataSet::chartSubType() const
{
    return d->chartSubType;
}

Axis *DataSet::attachedAxis() const
{
    return d->attachedAxis;
}

// FIXME: Should this method also be called proxyModel?
ChartProxyModel *DataSet::model() const
{
    return d->model;
}

bool DataSet::showMeanValue() const
{
    return d->showMeanValue;
}

QPen DataSet::meanValuePen() const
{
    return d->meanValuePen;
}

bool DataSet::showLowerErrorIndicator() const
{
    return d->showLowerErrorIndicator;
}

bool DataSet::showUpperErrorIndicator() const
{
    return d->showUpperErrorIndicator;
}

QPen DataSet::errorIndicatorPen() const
{
    return d->errorIndicatorPen;
}

ErrorCategory DataSet::errorCategory() const
{
    return d->errorCategory;
}

qreal DataSet::errorPercentage() const
{
    return d->errorPercentage;
}

qreal DataSet::errorMargin() const
{
    return d->errorMargin;
}

qreal DataSet::lowerErrorLimit() const
{
    return d->lowerErrorLimit;
}

qreal DataSet::upperErrorLimit() const
{
    return d->upperErrorLimit;
}


void DataSet::setChartType( ChartType type )
{
    if ( type == d->chartType )
        return;

    Axis  *axis = d->attachedAxis;
    if ( axis )
        axis->detachDataSet( this );

    d->chartType = type;

    if ( axis )
        axis->attachDataSet( this );
}

void DataSet::setChartSubType( ChartSubtype subType )
{
    if ( subType == d->chartSubType )
        return;

    Axis *axis = d->attachedAxis;
    axis->detachDataSet( this );

    d->chartSubType = subType;

    axis->attachDataSet( this );
}


void DataSet::setAttachedAxis( Axis *axis )
{
    d->attachedAxis = axis;
}

bool DataSet::showLabels( int section /* = -1 */ ) const
{
    if ( section >= 0 )
        return d->sectionsShowLabels[ section ];
    return d->showLabels;
}

void DataSet::setShowLabels( bool showLabels, int section /* = -1 */ )
{
    if ( section >= 0 )
        d->sectionsShowLabels[ section ] = showLabels;
    else
        d->showLabels = showLabels;
}

QPen DataSet::pen() const
{
    return d->penIsSet ? d->pen : d->defaultPen();
}

QBrush DataSet::brush() const
{
    return d->brushIsSet ? d->brush : d->defaultBrush();
}

QPen DataSet::pen( int section ) const
{
    if ( d->pens.contains( section ) )
        return d->pens[ section ];
    return pen();
}

KDChart::PieAttributes DataSet::pieAttributes() const
{
    return d->pieAttributes;
}

QBrush DataSet::brush( int section ) const
{
    if ( d->brushes.contains( section ) )
        return d->brushes[ section ];
    if ( d->brushIsSet )
        return brush();
    return d->defaultBrush( section );
}

KDChart::PieAttributes DataSet::pieAttributes( int section ) const
{
    if( d->sectionsPieAttributes.contains( section ) )
        return d->sectionsPieAttributes[ section ];
    return pieAttributes();
}

KDChart::DataValueAttributes DataSet::dataValueAttributes( int section /* = -1 */ ) const
{
    if ( d->sectionsDataValueAttributes.contains( section ) )
        return d->sectionsDataValueAttributes[ section ];
    return d->dataValueAttributes;
}

void DataSet::setPen( const QPen &pen )
{
    d->pen = pen;
    d->penIsSet = true;
    if ( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this );
}

void DataSet::setBrush( const QBrush &brush )
{
    d->brush = brush;
    d->brushIsSet = true;
    if ( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this );
}

void DataSet::setPieExplodeFactor( int factor )
{
    KDChart::PieAttributes pieAttributes;
    pieAttributes.setExplodeFactor( (qreal)factor / (qreal)100 );
    d->pieAttributes = pieAttributes;
    if( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this );
}

void DataSet::setPen( int section, const QPen &pen )
{
    d->pens[ section ] = pen;
    if ( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::PenDataRole, section );
}

void DataSet::setBrush( int section, const QBrush &brush )
{
    d->brushes[ section ] = brush;
    if ( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::BrushDataRole, section );
}

void DataSet::setPieExplodeFactor( int section, int factor )
{
    KDChart::PieAttributes pieAttributes;
    pieAttributes.setExplodeFactor( (qreal)factor / (qreal)100 );
    d->sectionsPieAttributes[ section ] = pieAttributes;
    if( d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::PieAttributesRole, section);
}

QColor DataSet::color() const
{
    return d->brush.color();
}

void DataSet::setColor( const QColor &color )
{
    QBrush brush = d->brush;
    brush.setColor( color );
    setBrush( brush );
}

int DataSet::number() const
{
    return d->num;
}

void DataSet::setNumber( int num )
{
    if ( !d->blockSignals && d->attachedAxis )
        d->attachedAxis->detachDataSet( this );

    d->num = num;

    if ( !d->blockSignals && d->attachedAxis )
        d->attachedAxis->attachDataSet( this );
}

void DataSet::setShowMeanValue( bool show )
{
    d->showMeanValue = show;
}

void DataSet::setMeanValuePen( const QPen &pen )
{
    d->meanValuePen = pen;
}

void DataSet::setShowLowerErrorIndicator( bool show )
{
    d->showLowerErrorIndicator = show;
}

void DataSet::setShowUpperErrorIndicator( bool show )
{
    d->showUpperErrorIndicator = show;
}

void DataSet::setShowErrorIndicators( bool lower, bool upper )
{
    setShowLowerErrorIndicator( lower );
    setShowUpperErrorIndicator( upper );
}

void DataSet::setErrorIndicatorPen( const QPen &pen )
{
    d->errorIndicatorPen = pen;
}

void DataSet::setErrorCategory( ErrorCategory category )
{
    d->errorCategory = category;
}

void DataSet::setErrorPercentage( qreal percentage )
{
    d->errorPercentage = percentage;
}

void DataSet::setErrorMargin( qreal margin )
{
    d->errorMargin = margin;
}

void DataSet::setLowerErrorLimit( qreal limit )
{
    d->lowerErrorLimit = limit;
}

void DataSet::setUpperErrorLimit( qreal limit )
{
    d->upperErrorLimit = limit;
}

QVariant DataSet::xData( int index ) const
{
    return d->data( d->xDataRegion, index );
}

QVariant DataSet::yData( int index ) const
{
    return d->data( d->yDataRegion, index );
}

QVariant DataSet::customData( int index ) const
{
    return d->data( d->customDataRegion, index );
}

QVariant DataSet::categoryData( int index ) const
{
    // There's no cell that holds this category's data
    // (i.e., the region is either too short or simply empty)
    if ( !d->categoryDataRegion.hasPointAtIndex( index ) )
        return QString::number( index + 1 );

    const QVariant data = d->data( d->categoryDataRegion, index );
    // The cell contains valid data
    if ( data.isValid() )
        return data;
    // The cell is empty
    return QString( "" );
}

QVariant DataSet::labelData() const
{
    QString label;

    const int cellCount = d->labelDataRegion.cellCount();
    for ( int i = 0; i < cellCount; i++ )
        label += d->data( d->labelDataRegion, i ).toString();

    if ( label.isEmpty() )
        label = i18n( "Series %1", number() + 1 );

    return QVariant( label );
}


CellRegion DataSet::xDataRegion() const
{
    return d->xDataRegion;
}

CellRegion DataSet::yDataRegion() const
{
    return d->yDataRegion;
}

CellRegion DataSet::customDataRegion() const
{
    return d->customDataRegion;
}

CellRegion DataSet::categoryDataRegion() const
{
    return d->categoryDataRegion;
}

CellRegion DataSet::labelDataRegion() const
{
    return d->labelDataRegion;
}

QString DataSet::xDataRegionString() const
{
    return CellRegion::regionToString( d->xDataRegion.rects() );
}

QString DataSet::yDataRegionString() const
{
    return CellRegion::regionToString( d->yDataRegion.rects() );
}

QString DataSet::customDataRegionString() const
{
    return CellRegion::regionToString( d->customDataRegion.rects() );
}

QString DataSet::categoryDataRegionString() const
{
    return CellRegion::regionToString( d->categoryDataRegion.rects() );
}

QString DataSet::labelDataRegionString() const
{
    return CellRegion::regionToString( d->labelDataRegion.rects() );
}


void DataSet::setXDataRegion( const CellRegion &region )
{
    d->xDataRegion = region;
    d->updateSize();

    if ( !d->blockSignals && d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::XDataRole, 0, size() - 1 );
}

void DataSet::setYDataRegion( const CellRegion &region )
{
    d->yDataRegion = region;
    d->updateSize();

    if ( !d->blockSignals && d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::YDataRole, 0, size() - 1 );
}

void DataSet::setCustomDataRegion( const CellRegion &region )
{
    d->customDataRegion = region;
    d->updateSize();

    if ( !d->blockSignals && d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::CustomDataRole, 0, size() - 1 );
}

void DataSet::setCategoryDataRegion( const CellRegion &region )
{
    d->categoryDataRegion = region;
    d->updateSize();
}

void DataSet::setLabelDataRegion( const CellRegion &region )
{
    d->labelDataRegion = region;
    d->updateSize();

    if ( !d->blockSignals && d->kdChartModel )
        d->kdChartModel->dataSetChanged( this, KDChartModel::LabelDataRole, 0, size() - 1 );
}

void DataSet::setXDataRegionString( const QString &string )
{
    setXDataRegion( CellRegion::stringToRegion( string ) );
}

void DataSet::setYDataRegionString( const QString &string )
{
    setYDataRegion( CellRegion::stringToRegion( string ) );
}

void DataSet::setCustomDataRegionString( const QString &string )
{
    setCustomDataRegion( CellRegion::stringToRegion( string ) );
}

void DataSet::setCategoryDataRegionString( const QString &string )
{
    setCategoryDataRegion( CellRegion::stringToRegion( string ) );
}

void DataSet::setLabelDataRegionString( const QString &string )
{
    setLabelDataRegion( CellRegion::stringToRegion( string ) );
}


int DataSet::size() const
{
    return d->size > 0 ? d->size : 1;
}

void DataSet::Private::dataChanged( KDChartModel::DataRole role, const QRect &rect ) const
{
    if ( blockSignals || !kdChartModel )
        return;

    const CellRegion *cellRegion = 0;
    switch ( role ) {
    case KDChartModel::YDataRole:
        cellRegion = &yDataRegion;
        break;
    case KDChartModel::XDataRole:
        cellRegion = &xDataRegion;
        break;
    case KDChartModel::CategoryDataRole:
        cellRegion = &categoryDataRegion;
        break;
    case KDChartModel::LabelDataRole:
        cellRegion = &labelDataRegion;
        break;
    case KDChartModel::CustomDataRole:
        cellRegion = &customDataRegion;
        break;
    // TODO
    case KDChartModel::ZDataRole:
    case KDChartModel::BrushDataRole:
    case KDChartModel::PenDataRole:
    case KDChartModel::PieAttributesRole:
        return;
    }

    int start, end;
    sectionsInRect( *cellRegion, rect, start, end );

    kdChartModel->dataSetChanged( parent, role, start, end );
}

void DataSet::yDataChanged( const QRect &region ) const
{
    d->dataChanged( KDChartModel::YDataRole, region );
}

void DataSet::xDataChanged( const QRect &region ) const
{
    d->dataChanged( KDChartModel::XDataRole, region );
}

void DataSet::customDataChanged( const QRect &region ) const
{
    d->dataChanged( KDChartModel::CustomDataRole, region );
}

void DataSet::labelDataChanged( const QRect &region ) const
{
    d->dataChanged( KDChartModel::LabelDataRole, region );
}

void DataSet::categoryDataChanged( const QRect &region ) const
{
    d->dataChanged( KDChartModel::CategoryDataRole, region );
}

int DataSet::dimension() const
{
    const ChartType chartType = d->effectiveChartType();
    // FIXME BUG: Ring, Surface
    switch ( chartType ) {
    case BarChartType:
    case AreaChartType:
    case LineChartType:
    case CircleChartType:
    case RadarChartType:
    case SurfaceChartType:
        return 1;

    case RingChartType:
    case ScatterChartType:
    case GanttChartType:
        return 2;

    case BubbleChartType:
        return 3;

    case StockChartType:
        return 4;

        // We can only determine the dimension if
        // a chart type is set
    case LastChartType:
        return 0;
    }

    // Avoid warnings from the compiler.
    return 0;
}

void DataSet::setKdDiagram( KDChart::AbstractDiagram *diagram )
{
    d->kdDiagram = diagram;
}

KDChart::AbstractDiagram *DataSet::kdDiagram() const
{
    return d->kdDiagram;
}

int DataSet::kdDataSetNumber() const
{
    return d->kdDataSetNumber;
}

void DataSet::setKdDataSetNumber( int number )
{
    // FIXME: Is there anything to emit here?
    // In theory, this should be done before any data is retrieved
    // from KDChartModel
    d->kdDataSetNumber = number;
}

void DataSet::setKdChartModel( KDChartModel *model )
{
    d->kdChartModel = model;
}

KDChartModel *DataSet::kdChartModel() const
{
    return d->kdChartModel;
}

void DataSet::blockSignals( bool block )
{
    d->blockSignals = block;
}

void DataSet::setValueLabelType( ValueLabelType type, int section /* = -1 */ )
{
    KDChart::DataValueAttributes &attr = d->dataValueAttributes;
    if ( section >= 0 && !d->sectionsDataValueAttributes.contains( section ) )
        d->sectionsDataValueAttributes[ section ] = d->defaultDataValueAttributes();
    if ( section >= 0 )
        attr = d->sectionsDataValueAttributes[ section ];

    switch ( type ) {
        case NoValueLabel:
            attr.setVisible( false );
            break;
        case RealValueLabel:
            attr.setVisible( true );
            attr.setUsePercentage( false );
            break;
        case PercentageValueLabel:
            attr.setVisible( true );
            attr.setUsePercentage( true );
            attr.setSuffix( "%" );
            break;
    }
}

DataSet::ValueLabelType DataSet::valueLabelType( int section /* = -1 */ ) const
{
    KDChart::DataValueAttributes &attr = d->dataValueAttributes;
    if ( d->sectionsDataValueAttributes.contains( section ) )
        attr = d->sectionsDataValueAttributes[ section ];

    if ( !attr.isVisible() )
        return NoValueLabel;
    if ( !attr.usePercentage() )
        return RealValueLabel;
    return PercentageValueLabel;
}

bool loadBrushAndPen(KoShapeLoadingContext &context, const KoXmlElement &n, QBrush& brush, bool& brushLoaded, QPen& pen, bool& penLoaded)
{
    if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
        KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
        KoStyleStack &styleStack = odfLoadingContext.styleStack();
        styleStack.save();
        styleStack.clear();
        odfLoadingContext.fillStyleStack( n, KoXmlNS::chart, "style-name", "chart" );

        brushLoaded = false;
        penLoaded = false;

        styleStack.setTypeProperties( "graphic" );

        if ( styleStack.hasProperty( KoXmlNS::draw, "stroke" ) ) {
            QString stroke = styleStack.property( KoXmlNS::draw, "stroke" );
            pen = KoOdfGraphicStyles::loadOdfStrokeStyle( styleStack, stroke, odfLoadingContext.stylesReader() );
            penLoaded = true;
        }

        if ( styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) {
            QString fill = styleStack.property( KoXmlNS::draw, "fill" );
            if ( fill == "solid" || fill == "hatch" ) {
                brush = KoOdfGraphicStyles::loadOdfFillStyle( styleStack, fill, odfLoadingContext.stylesReader() );
                brushLoaded = true;
            } else if ( fill == "gradient" ) {
                brush = KoOdfGraphicStyles::loadOdfGradientStyle( styleStack, odfLoadingContext.stylesReader(), QSizeF( 5.0, 60.0 ) );
                brushLoaded = true;
            } else if ( fill == "bitmap" ) {
                brush = Surface::loadOdfPatternStyle( styleStack, odfLoadingContext, QSizeF( 5.0, 60.0 ) );
                brushLoaded = true;
            }
        }

        styleStack.restore();
    }

#ifndef NWORKAROUND_ODF_BUGS
    if( ! penLoaded) {
        penLoaded = KoOdfWorkaround::fixMissingStroke( pen, n, context );
    }
    if( ! brushLoaded) {
        QColor fixedColor = KoOdfWorkaround::fixMissingFillColor( n, context );
        if ( fixedColor.isValid() ) {
            brush = fixedColor;
            brushLoaded = true;
        }
    }
#endif
    return true;
}

bool DataSet::loadOdf( const KoXmlElement &n,
                       KoShapeLoadingContext &context )
{
    KoOdfLoadingContext &odfLoadingContext = context.odfLoadingContext();
    KoStyleStack &styleStack = odfLoadingContext.styleStack();

    {
        QBrush brush(Qt::NoBrush);
        QPen pen(Qt::NoPen);
        bool brushLoaded = false;
        bool penLoaded = false;
        loadBrushAndPen(context, n, brush, brushLoaded, pen, penLoaded);
        if(penLoaded)
            setPen( pen );
        if(brushLoaded)
            setBrush( brush );
        styleStack.save();
        styleStack.setTypeProperties("chart");
        if(styleStack.hasProperty(KoXmlNS::chart, "pie-offset"))
            setPieExplodeFactor( styleStack.property( KoXmlNS::chart, "pie-offset" ).toInt() );
        styleStack.restore();
    }

    if ( n.hasAttributeNS( KoXmlNS::chart, "values-cell-range-address" ) ) {
        const QString region = n.attributeNS( KoXmlNS::chart, "values-cell-range-address", QString() );
        setYDataRegionString( region );
    }
    if ( n.hasAttributeNS( KoXmlNS::chart, "label-cell-address" ) ) {
        const QString region = n.attributeNS( KoXmlNS::chart, "label-cell-address", QString() );
        setLabelDataRegionString( region );
    }
    if ( n.hasAttributeNS( KoXmlNS::chart, "data-label-text" ) ) {
        const QString enable = n.attributeNS( KoXmlNS::chart, "data-label-text", QString() );
        setShowLabels( enable == "true" );
    }
    if ( styleStack.hasProperty(KoXmlNS::chart, "data-label-number" ) ) {
        const QString format = styleStack.property( KoXmlNS::chart, "data-label-number" );
        ValueLabelType type = NoValueLabel;
        if ( format == "value" )
            type = RealValueLabel;
        else if ( format == "percentage" )
            type = PercentageValueLabel;
        setValueLabelType( type );
    }

    // load data points
    KoXmlElement m;
    int loadedDataPointCount = 0;
    forEachElement ( m, n ) {
        if ( m.namespaceURI() != KoXmlNS::chart )
            continue;
        if ( m.localName() != "data-point" )
            continue;
        QBrush brush(Qt::NoBrush);
        QPen pen(Qt::NoPen);
        bool brushLoaded = false;
        bool penLoaded = false;
        loadBrushAndPen(context, m, brush, brushLoaded, pen, penLoaded);
        if(penLoaded)
            setPen( loadedDataPointCount, pen );
        if(brushLoaded)
            setBrush( loadedDataPointCount, brush );

        //load pie explode factor
        styleStack.save();
        odfLoadingContext.fillStyleStack(m, KoXmlNS::chart, "style-name", "chart");
        styleStack.setTypeProperties("chart");
        if(styleStack.hasProperty( KoXmlNS::chart, "pie-offset"))
            setPieExplodeFactor( loadedDataPointCount, styleStack.property( KoXmlNS::chart, "pie-offset" ).toInt() );
        styleStack.restore();

        ++loadedDataPointCount;
    }

    return true;
}

void DataSet::saveOdf( KoShapeSavingContext &context ) const
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();

    bodyWriter.startElement( "chart:series" );

    KoGenStyle style( KoGenStyle::ChartAutoStyle, "chart" );

    style.addProperty( "chart:data-label-text", showLabels() ? "true" : "false"  );
    style.addProperty( "chart:family", ODF_CHARTTYPES[ chartType() ] );

    KoOdfGraphicStyles::saveOdfFillStyle( style, mainStyles, brush() );
    KoOdfGraphicStyles::saveOdfStrokeStyle( style, mainStyles, pen() );

    const QString styleName = mainStyles.insert( style, "ch" );
    bodyWriter.addAttribute( "chart:style-name", styleName );

    // TODO: Save external data sources also
    const QString prefix( "local-table." );

    // Save cell regions
    bodyWriter.addAttribute( "chart:values-cell-range-address", prefix + yDataRegionString() );
    bodyWriter.addAttribute( "chart:label-cell-address", prefix + labelDataRegionString() );

    bodyWriter.endElement(); // chart:series
}
