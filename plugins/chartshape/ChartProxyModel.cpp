/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

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


// Own
#include "ChartProxyModel.h"

// Qt
#include <QRegion>
#include <QPoint>

// KDE
#include <KDebug>

// KOffice
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <interfaces/KoChartModel.h>

// KChart
#include "Axis.h"
#include "DataSet.h"
#include "TableSource.h"
#include "OdfLoadingHelper.h"

using namespace KChart;


// ================================================================
//                     Class ChartProxyModel::Private


class ChartProxyModel::Private {
public:
    Private( ChartProxyModel *parent, TableSource *source );
    ~Private();

    ChartProxyModel *const q;

    TableSource *const tableSource;

    /// Set to true if we're in the process of loading data from ODF.
    /// Used to avoid repeatedly updating data.
    bool isLoading;

    bool             firstRowIsLabel;
    bool             firstColumnIsLabel;
    Qt::Orientation  dataDirection;
    int              dataDimensions;
    
    CellRegion categoryDataRegion;

    QVector< CellRegion > dataSetRegions;
    
    QList<DataSet*>  dataSets;
    QList<DataSet*>  removedDataSets;
    
    CellRegion       selection;

    /**
     * Discards old and creates new data sets from the current region selection.
     */
    void rebuildDataMap();

    /**
     * Extracts a list of data sets (with x data region, y data region, etc.
     * assigned) from the current d->selection.
     *
     * As a side effect, this method sets d->categoryDataRegion.
     */
    QList<DataSet*> createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle );
};

ChartProxyModel::Private::Private( ChartProxyModel *parent, TableSource *source )
    : q( parent )
    , tableSource( source )
    , isLoading( false )
{
    firstRowIsLabel    = false;
    firstColumnIsLabel = false;
    dataDimensions     = 1;

    // Determines what orientation the data points in a data series
    // have when multiple data sets are created from one source
    // region. For example, vertical means that each column in the source
    // region is assigned to one data series.
    // Default to Qt::Vertical, as that's what OOo does also.
    dataDirection      = Qt::Vertical;
}

ChartProxyModel::Private::~Private()
{
}


// ================================================================
//                          Class ChartProxyModel


ChartProxyModel::ChartProxyModel( TableSource *source )
    : QAbstractTableModel(),
      d( new Private( this, source ) )
{
    connect( source, SIGNAL( tableAdded( Table* ) ),
             this,   SLOT( addTable( Table* ) ) );
    connect( source, SIGNAL( tableRemoved( Table* ) ),
             this,   SLOT( removeTable( Table* ) ) );
}

ChartProxyModel::~ChartProxyModel()
{
    delete d;
}

void ChartProxyModel::reset( const CellRegion& region )
{
    d->selection = region;
    d->rebuildDataMap();
}

void ChartProxyModel::Private::rebuildDataMap()
{
    // Don't do anything while we're loading from ODF.
    // ChartProxyModel::endLoading() will get back to us.
    if ( isLoading )
        return;
    q->beginResetModel();
    q->invalidateDataSets();
    dataSets = createDataSetsFromRegion( removedDataSets );
    q->endResetModel();
}

void ChartProxyModel::addTable( Table *table )
{
    QAbstractItemModel *model = table->model();
    connect( model, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
             this,  SLOT( dataChanged( QModelIndex, QModelIndex ) ) );
}

void ChartProxyModel::removeTable( Table *table )
{
    QAbstractItemModel *model = table->model();
    model->disconnect( this );
}

/**
 * Returns a row of a given region (i.e. a part of it with height 1), cutting
 * off the first @a colOffset cells in that row.
 *
 * Examples: extractRow( A1:C2, 0, 0 ) --> A1:C1
 *           extractRow( A1:C2, 1, 0 ) --> A2:C2
 *           extractRow( A1:C2, 0, 1 ) --> B1:C1
 *
 * See notes in createDataSetsFromRegion() for further details.
 *
 * @param region The region to extract the row from
 * @param row The number of the row, starting with 0
 * @param colOffset How many of the first columns to cut from the resulting row
 */
static CellRegion extractRow( const CellRegion &region, int row, int colOffset )
{
    foreach( const QRect &rect, region.rects() ) {
        if ( row >= rect.height() ) {
            row -= rect.height();
            continue;
        }
        QPoint topLeft = rect.topLeft() + QPoint( colOffset, row );
        QRect row( topLeft, QSize( rect.width() - colOffset, 1 ) );
        return CellRegion( region.table(), row );
    }

    return CellRegion();
}

/**
 * Returns a column of a given region, cutting off the first @a rowOffset
 * rows in that column.
 *
 * Examples: extractColumn( A1:C2, 0, 0 )       --> A1:A2
 *           extractColumn( A1:C2;D1;F2, 0, 0 ) --> A1:A2;D1:D2
 *
 * See notes in createDataSetsFromRegion() for further details.
 *
 * @param region The region to extract the row from
 * @param col The number of the column, starting with 0
 * @param rowOffset How many of the first rows to cut from the resulting column
 */
static CellRegion extractColumn( const CellRegion &region, int col, int rowOffset )
{
    CellRegion result( region.table() );
    foreach( const QRect &rect, region.rects() ) {
        if ( col >= rect.width() )
            continue;
        QPoint topLeft = rect.topLeft() + QPoint( col, rowOffset );
        QRect col( topLeft, QSize( 1, rect.height() - rowOffset ) );
        result.add( col );
    }

    return result;
}

QList<DataSet*> ChartProxyModel::Private::createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle )
{
    if ( !selection.isValid() )
        return QList<DataSet*>();

    QList<DataSet*> createdDataSets;

    // What this algorithm does:
    //
    // First it calculates the number of rows and columns we'd have if we'd
    // stack up all subregions on top of each other. So something like
    //
    // aaa   bb
    //   cccc
    //
    // would become
    //
    // aaa
    // bb
    // cccc
    //
    // assuming that the order is aaa,bb,cccc
    //
    // The methods extractRow() and extractColumn() are then used to extract
    // the rows and columns from this very construct.
    int rows = 0;
    int cols = 0;
    foreach( const QRect &rect, selection.rects() ) {
        rows += rect.height();
        cols = qMax( cols, rect.width() );
    }

    // In the end, the contents of this list will look something like this:
    // ( Category-Data, X-Data, Y-Data, Y-Data, Y-Data )
    // Semantic seperation of the regions will follow later.
    QList<CellRegion> dataRegions;
    // This region exlusively contains (global) data set labels, i.e.
    // one label per data set (thus in opposite data direction)
    CellRegion labelRegion;

    // Determines how many individual rows/columns will be assigned per data set.
    // It is at least one, but if there's more than one data dimension, the x
    // data is shared among all data sets, thus - 1.
    int regionsPerDataSet = qMax( 1, dataDimensions - 1 );

    // Fill dataRegions and set categoryRegion.
    // Note that here, we don't exactly know yet what region will be used for
    // what data set, we also don't know yet what data these regions contain.
    int rowOffset = firstRowIsLabel ? 1 : 0;
    int colOffset = firstColumnIsLabel ? 1 : 0;

    // When x data is present, it occupies the first non-header row/column
    if ( dataDimensions > 1 && dataDirection == Qt::Horizontal )
        rowOffset++;
    if ( dataDimensions > 1 && dataDirection == Qt::Vertical )
        colOffset++;

    // This is the logic that extracts all the subregions from selection
    // that are later used for the data sets
    if ( dataDirection == Qt::Horizontal ) {
        if ( firstColumnIsLabel )
            labelRegion = extractColumn( selection, 0, rowOffset );
        for ( int i = 0; i < rows; i++ )
            dataRegions.append( extractRow( selection, i, colOffset ) );
    } else {
        if ( firstRowIsLabel )
            labelRegion = extractRow( selection, 0, colOffset);
        for ( int i = 0; i < cols; i++ )
            dataRegions.append( extractColumn( selection, i, rowOffset ) );
    }

    bool useCategories =
            dataDirection == Qt::Horizontal && firstRowIsLabel ||
            dataDirection == Qt::Vertical && firstColumnIsLabel;

    // Regions shared by all data sets: categories and x-data
    categoryDataRegion = CellRegion(); // member variable
    CellRegion xData;
    if ( !dataRegions.isEmpty() && useCategories )
        categoryDataRegion = dataRegions.takeFirst();
    if ( !dataRegions.isEmpty() && dataDimensions > 1 )
        xData = dataRegions.takeFirst();

    int dataSetNumber = 0;
    // Now assign all dataRegions to a number of data sets.
    // Here they're semantically seperated into x data, y data, etc.
    while ( !dataRegions.isEmpty() ) {
        // Get a data set instance we can use
        DataSet *dataSet;
        if ( !dataSetsToRecycle.isEmpty() )
            dataSet = dataSetsToRecycle.takeFirst();
        else
            dataSet = new DataSet( dataSetNumber );

        // category and x data are "global" regions shared among all data sets
        dataSet->setCategoryDataRegion( categoryDataRegion );
        dataSet->setXDataRegion( xData );
        // Last row/column of this data set contains label (row/column
        // immediately before the next data set, thus (.. + 1) * .. - 1)
        int labelRowCol = (dataSetNumber + 1) * regionsPerDataSet - 1;
        if ( labelRegion.hasPointAtIndex( labelRowCol ) ) {
            QPoint point( labelRegion.pointAtIndex( labelRowCol ) );
            dataSet->setLabelDataRegion( CellRegion( selection.table(), point ) );
        }
        else
            dataSet->setLabelDataRegion( CellRegion() );

        // regions per data set: y data, custom data (e.g. bubble width)
        dataSet->setYDataRegion( dataRegions.takeFirst() );

        if ( !dataRegions.isEmpty() && dataDimensions > 2 )
            dataSet->setCustomDataRegion( dataRegions.takeFirst() );
        else
            dataSet->setCustomDataRegion( CellRegion() );

        createdDataSets.append( dataSet );

        // Increment number at the very end!
        dataSetNumber++;
    }

    return createdDataSets;
}

void ChartProxyModel::saveOdf( KoShapeSavingContext &context ) const
{
    foreach ( DataSet *dataSet, d->dataSets )
        dataSet->saveOdf( context );
}

// This loads the properties of the datasets (chart:series).
// FIXME: This is a strange place to load them (the proxy model)
bool ChartProxyModel::loadOdf( const KoXmlElement &element,
                               KoShapeLoadingContext &context )
{
    Q_ASSERT( d->isLoading );

    OdfLoadingHelper *helper = (OdfLoadingHelper*)context.sharedData( OdfLoadingHelperId );
    // If we exclusively use the chart's internal model then all data
    // is taken from there and each data set is automatically assigned
    // the rows it belongs to.
    bool ignoreCellRanges = helper->chartUsesInternalModelOnly;

    beginResetModel();
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    invalidateDataSets();

    // A cell range for all data is optional.
    // If cell ranges are in addition specified for one or more of these
    // data series, they'll be overwritten by these values.
    // Note: In case ignoreCellRanges is true, ChartShape::loadOdf() has
    // already made sure the proxy is reset with data from the internal model.
    if ( !ignoreCellRanges &&
         element.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
    {
        QString cellRangeAddress = element.attributeNS( KoXmlNS::table, "cell-range-address" );
        d->selection = CellRegion( d->tableSource, cellRangeAddress );
    }

    // This is what we'll use as basis for the data sets we "produce" from ODF.
    // This might be data sets that were "instantiated" from the internal
    // table or from an arbitrary selection of other tables as specified
    // in the PlotArea's table:cell-range-address attribute (parsed above).
    QList<DataSet*> createdDataSets = d->createDataSetsFromRegion( d->removedDataSets );
    int loadedDataSetCount = 0;

    KoXmlElement n;
    forEachElement ( n, element ) {
        if ( n.namespaceURI() != KoXmlNS::chart )
            continue;

        if ( n.localName() == "series" ) {
            DataSet *dataSet;
            if ( loadedDataSetCount < createdDataSets.size() ) {
                dataSet = createdDataSets[loadedDataSetCount];
            } else {
                // the datasetnumber needs to be known at construction time, to ensure
                // default colors are set correctly
                dataSet = new DataSet( d->dataSets.size() );
            }
            d->dataSets.append( dataSet );
            dataSet->loadOdf( n, context );

            ++loadedDataSetCount;
        } else {
            qWarning() << "ChartProxyModel::loadOdf(): Unknown tag name \"" << n.localName() << "\"";
        }
    }

    //rebuildDataMap();
    endResetModel();

    styleStack.restore();
    return true;
}


QVariant ChartProxyModel::data( const QModelIndex &index,
                                int role ) const
{
    Q_UNUSED( index );
    Q_UNUSED( role );
    Q_ASSERT( "To be implemented" );
    return QVariant();
}

void ChartProxyModel::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    QPoint topLeftPoint( topLeft.column() + 1, topLeft.row() + 1 );

    // Excerpt from the Qt reference for QRect::bottomRight() which is
    // used for calculating bottomRight.  Note that for historical
    // reasons this function returns
    //   QPoint(left() + width() -1, top() + height() - 1).
    QPoint bottomRightPoint( bottomRight.column() + 1, bottomRight.row() + 1 );
    QRect dataChangedRect = QRect( topLeftPoint,
                                   QSize( bottomRightPoint.x() - topLeftPoint.x() + 1,
                                          bottomRightPoint.y() - topLeftPoint.y() + 1 ) );

    foreach ( DataSet *dataSet, d->dataSets ) {
        if ( dataSet->xDataRegion().intersects( dataChangedRect ) )
            dataSet->xDataChanged( QRect() );

        if ( dataSet->yDataRegion().intersects( dataChangedRect ) )
            dataSet->yDataChanged( QRect() );

        if ( dataSet->categoryDataRegion().intersects( dataChangedRect ) )
            dataSet->categoryDataChanged( QRect() );

        if ( dataSet->labelDataRegion().intersects( dataChangedRect ) )
            dataSet->labelDataChanged( QRect() );

        if ( dataSet->customDataRegion().intersects( dataChangedRect ) )
            dataSet->customDataChanged( QRect() );
    }

    emit dataChanged();
}


QVariant ChartProxyModel::headerData( int section,
                                      Qt::Orientation orientation,
                                      int role /* = Qt::DisplayRole */ ) const
{
    Q_UNUSED( section );
    Q_UNUSED( orientation );
    Q_UNUSED( role );
    Q_ASSERT( "To be implemented" );
    return QVariant();
}


QModelIndex ChartProxyModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

int ChartProxyModel::rowCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    return d->dataSets.count();
}


int ChartProxyModel::columnCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    // FIXME: Replace this by the actual column count once the proxy is properly being used.
    return INT_MAX;
}

void ChartProxyModel::setFirstRowIsLabel( bool b )
{
    if ( b == d->firstRowIsLabel )
        return;

    d->firstRowIsLabel = b;
    d->rebuildDataMap();
}
 

void ChartProxyModel::setFirstColumnIsLabel( bool b )
{
    if ( b == d->firstColumnIsLabel )
        return;

    d->firstColumnIsLabel = b;
    d->rebuildDataMap();
}

Qt::Orientation ChartProxyModel::dataDirection()
{
    return d->dataDirection;
}

void ChartProxyModel::invalidateDataSets()
{
    d->removedDataSets = d->dataSets;
    d->dataSets.clear();
}

void ChartProxyModel::beginLoading()
{
    Q_ASSERT( !d->isLoading );
    // FIXME: invalidateDataSets() used to be called explicitly at the beginning
    // of ChartShape::loadOdf(). Now beginLoading() is called instead.
    // So, is invalidateDataSets() still necessary here?
    invalidateDataSets();
    d->isLoading = true;
}

void ChartProxyModel::endLoading()
{
    Q_ASSERT( d->isLoading );
    d->isLoading = false;

    d->rebuildDataMap();
}

void ChartProxyModel::setDataDirection( Qt::Orientation orientation )
{
    if ( d->dataDirection == orientation )
        return;

    d->dataDirection = orientation;
    d->rebuildDataMap();
}

void ChartProxyModel::setDataDimensions( int dimensions )
{
    if ( d->dataDimensions == dimensions )
        return;

    d->dataDimensions = dimensions;
    d->rebuildDataMap();
}

bool ChartProxyModel::firstRowIsLabel() const
{
    return d->firstRowIsLabel;
}

bool ChartProxyModel::firstColumnIsLabel() const
{
    return d->firstColumnIsLabel;
}

CellRegion ChartProxyModel::categoryDataRegion() const
{
    return d->categoryDataRegion;
}

void ChartProxyModel::setCategoryDataRegion( const CellRegion &region )
{
    d->categoryDataRegion = region;
}

QList<DataSet*> ChartProxyModel::dataSets() const
{
    return d->dataSets;
}

#include "ChartProxyModel.moc"
