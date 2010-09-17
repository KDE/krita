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

using namespace KChart;


// ================================================================
//                     Class ChartProxyModel::Private


class ChartProxyModel::Private {
public:
    Private( ChartProxyModel *parent, TableSource *source );
    ~Private();

    ChartProxyModel *const q;

    TableSource *const tableSource;

    bool             firstRowIsLabel;
    bool             firstColumnIsLabel;
    Qt::Orientation  dataDirection;
    int              dataDimensions;
    
    CellRegion categoryDataRegion;

    QVector< CellRegion > dataSetRegions;
    
    QList<DataSet*>  dataSets;
    QList<DataSet*>  removedDataSets;
    
    CellRegion       selection;

    bool automaticDataSetCreation;
    int createdDataSetCount;
};

ChartProxyModel::Private::Private( ChartProxyModel *parent, TableSource *source )
    : q( parent )
    , tableSource( source )
    , createdDataSetCount( 0 )
{
    firstRowIsLabel    = false;
    firstColumnIsLabel = false;
    dataDimensions     = 1;
    automaticDataSetCreation = true;

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
    beginResetModel();

    d->selection = region;
    invalidateDataSets();
    d->dataSets = createDataSetsFromRegion( d->removedDataSets );

    endResetModel();
}

#if QT_VERSION < 0x040600
void ChartProxyModel::beginResetModel()
{
}

void ChartProxyModel::endResetModel()
{
    reset();
}
#endif

void ChartProxyModel::rebuildDataMap()
{
    invalidateDataSets();
    d->dataSets = createDataSetsFromRegion( d->removedDataSets );
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

QList<DataSet*> ChartProxyModel::createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle )
{
    if ( !d->selection.isValid() )
        return QList<DataSet*>();
    
    QList<DataSet*> createdDataSets;
    const QVector<QRect> dataRegions = d->selection.rects();
    Table *table = d->selection.table();
    Q_ASSERT( table );
    
    int& createdDataSetCount = d->createdDataSetCount;
    int number = 0;
    if ( d->dataDirection == Qt::Horizontal ) {
        QMap<int, QVector<QRect> >  rows;
        QMap<int, QVector<QRect> >  sortedRows;

        // Split up region in horizontal rectangles
        // that are sorted from top to bottom
        foreach ( const QRect &rect, dataRegions ) {
            int x = rect.topLeft().x();
            for ( int y = rect.topLeft().y(); y <= rect.bottomLeft().y(); y++ )
            {
                QRect dataRect = QRect( QPoint( x, y ), QSize( rect.width(), 1 ) );
                if ( !rows.contains( y ) )
                    rows.insert( y, QVector<QRect>() );
                rows[y].append( dataRect );
            }
        }
        
        // Sort rectangles in each row from left to right.
        QMapIterator<int, QVector<QRect> >  i( rows );
        while ( i.hasNext() ) {
            i.next();
            int             row = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            
            foreach ( const QRect &rect, unsortedRects ) {
                int index;
                
                for ( index = 0; index < sortedRects.size(); index++ )
                    if ( rect.topLeft().x() <= sortedRects[ index ].topLeft().x() )
                        break;
                
                sortedRects.insert( index, rect );
            }
            
            sortedRows.insert( row, sortedRects );
        }
        
        QMapIterator<int, QVector<QRect> > j( sortedRows );
        
        CellRegion category;
        if ( categoryDataRegion().isValid() ) {
            category = categoryDataRegion();
        } else if ( d->firstRowIsLabel && j.hasNext() ) {
            j.next();
            
            category = CellRegion( table, j.value() );
            if ( d->firstColumnIsLabel )
                category.subtract( category.pointAtIndex( 0 ) );
        }
        
        while ( j.hasNext() ) {
            j.next();
            
            DataSet *dataSet;
            if ( !dataSetsToRecycle.isEmpty() )
                dataSet = dataSetsToRecycle.takeLast();
            else{
                // the datasetnumber needs to be known at construction time, to ensure
                // default colors are set correctly
                dataSet = new DataSet( this, createdDataSetCount );                
            }
            createdDataSets.append( dataSet );

            dataSet->blockSignals( true );
            
            dataSet->setNumber( number );
            //dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );

            CellRegion labelDataRegion;

            CellRegion xDataRegion;
            // In case of > 1 data dimensions, x data appears before y data
            if ( d->dataDimensions > 1 )
                xDataRegion = CellRegion( table, j.value() );

            //qDebug() << "Creating data set with region" << j.value();
            if ( d->firstColumnIsLabel ) {
                CellRegion tmpRegion = CellRegion( table, j.value() );
                QPoint labelDataPoint = tmpRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( table, labelDataPoint );
            }

            if ( d->dataDimensions > 1 && j.hasNext() )
                j.next();
            
            CellRegion yDataRegion( table, j.value() );

            if ( d->firstColumnIsLabel ) {
                xDataRegion.subtract( xDataRegion.pointAtIndex( 0 ) );
                yDataRegion.subtract( yDataRegion.pointAtIndex( 0 ) );
            }
            
            if ( d->dataDimensions > 2 && j.hasNext() )
                j.next();
            // adding support for third dimension, even if the existing scheme does not scale if we have need for aditional dimensions
            CellRegion zDataRegion( table, j.value() );

            if ( d->firstColumnIsLabel ) {
                zDataRegion.subtract( zDataRegion.pointAtIndex( 0 ) );
            }

            dataSet->setXDataRegion( xDataRegion );
            dataSet->setYDataRegion( yDataRegion );
            if ( d->dataDimensions >= 3 )
              dataSet->setCustomDataRegion( zDataRegion );
            dataSet->setCategoryDataRegion( category );
            dataSet->setLabelDataRegion( labelDataRegion );            
            dataSet->blockSignals( false );
            ++createdDataSetCount;
            ++number;
        }
    }
    else {
        // Data direction == Qt::Vertical here.

        QMap<int, QVector<QRect> >  columns;
        QMap<int, QVector<QRect> >  sortedColumns;

        // Split up region in horizontal rectangles
        // that are sorted from top to bottom
        foreach ( const QRect &rect, dataRegions ) {
            int y = rect.topLeft().y();
            for ( int x = rect.topLeft().x(); x <= rect.topRight().x(); x++ ) {
                QRect dataRect = QRect( QPoint( x, y ), QSize( 1, rect.height() ) );
                if ( !columns.contains( x ) )
                    columns.insert( x, QVector<QRect>() );

                columns[x].append( dataRect );
            }
        }
        
        // Sort rectangles in each column from top to bottom
        QMapIterator<int, QVector<QRect> >  i( columns );
        while ( i.hasNext() ) {
            i.next();

            int             row = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            
            foreach ( const QRect &rect, unsortedRects ) {
                int index;
                
                for ( index = 0; index < sortedRects.size(); index++ )
                    if ( rect.topLeft().y() <= sortedRects[ index ].topLeft().y() )
                        break;
                
                sortedRects.insert( index, rect );
            }
            
            sortedColumns.insert( row, sortedRects );
        }
        
        QMapIterator<int, QVector<QRect> > j( sortedColumns );
        
        CellRegion category;
        if ( categoryDataRegion().isValid() ) {
            category = categoryDataRegion();
        } else if ( d->firstColumnIsLabel && j.hasNext() ) {
            j.next();
            
            category = CellRegion( table, j.value() );
            if ( d->firstRowIsLabel )
                category.subtract( category.pointAtIndex( 0 ) );
        }
        
        while ( j.hasNext() ) {
            j.next();

            DataSet *dataSet;
            if ( !dataSetsToRecycle.isEmpty() )
                dataSet = dataSetsToRecycle.takeLast();
            else{
                // the datasetnumber needs to be known at construction time, to ensure
                // default colors are set correctly
                dataSet = new DataSet( this, createdDataSetCount );                
            }
            createdDataSets.append( dataSet );

            dataSet->blockSignals( true );
            
            dataSet->setNumber( number );
            //dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );

            CellRegion labelDataRegion;
            
            CellRegion xDataRegion;
            // In case of > 1 data dimensions, x data appears before y data
            if ( d->dataDimensions > 1 )
                xDataRegion = CellRegion( table, j.value() );

            //qDebug() << "Creating data set with region" << j.value();
            if ( d->firstRowIsLabel ) {
                CellRegion tmpRegion = CellRegion( table, j.value() );
                QPoint labelDataPoint = tmpRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( table, labelDataPoint );
            }

            if ( d->dataDimensions > 1 && j.hasNext() )
                j.next();
            
            CellRegion yDataRegion( table, j.value() );

            if ( d->firstRowIsLabel ) {
                xDataRegion.subtract( xDataRegion.pointAtIndex( 0 ) );
                yDataRegion.subtract( yDataRegion.pointAtIndex( 0 ) );
            }
            
            if ( d->dataDimensions > 2 && j.hasNext() )
                j.next();
            
            CellRegion zDataRegion( table, j.value() );

            if ( d->firstRowIsLabel ) {
                zDataRegion.subtract( zDataRegion.pointAtIndex( 0 ) );
            }

            dataSet->setXDataRegion( xDataRegion );
            dataSet->setYDataRegion( yDataRegion );
            if ( d->dataDimensions >= 3 )
              dataSet->setCustomDataRegion( zDataRegion );
            dataSet->setLabelDataRegion( labelDataRegion );
            dataSet->setCategoryDataRegion( category );
            dataSet->blockSignals( false );
            ++createdDataSetCount;
            ++number;
        }
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
    beginResetModel();
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    invalidateDataSets();

    QList<DataSet*> createdDataSets;
    int loadedDataSetCount = 0;

    // A cell range for all data is optional.
    // If cell ranges are in addition specified for one or more of these
    // data series, they'll be overwritten by these values.
    if ( element.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
    {
        QString cellRangeAddress = element.attributeNS( KoXmlNS::table, "cell-range-address" );
        // FIXME: Do we need to reset everything here? It may be enough to set the cell range address.
        reset( CellRegion( d->tableSource, cellRangeAddress ) );
    }

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
                dataSet = new DataSet( this, d->dataSets.size() );
                dataSet->setNumber( d->dataSets.size() );
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
            dataSet->xDataChanged( dataSet->xDataRegion().intersected( dataChangedRect ).boundingRect() );

        if ( dataSet->yDataRegion().intersects( dataChangedRect ) )
            dataSet->yDataChanged( dataSet->yDataRegion().intersected( dataChangedRect ).boundingRect() );

        if ( dataSet->categoryDataRegion().intersects( dataChangedRect ) )
            dataSet->categoryDataChanged( dataSet->categoryDataRegion().intersected( dataChangedRect ).boundingRect() );

        if ( dataSet->labelDataRegion().intersects( dataChangedRect ) )
            dataSet->labelDataChanged( dataSet->labelDataRegion().intersected( dataChangedRect ).boundingRect() );

        if ( dataSet->customDataRegion().intersects( dataChangedRect ) )
            dataSet->customDataChanged( dataSet->customDataRegion().intersected( dataChangedRect ).boundingRect() );
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

    beginResetModel();
    
    d->firstRowIsLabel = b;
    
    rebuildDataMap();
    endResetModel();
}
 

void ChartProxyModel::setFirstColumnIsLabel( bool b )
{
    if ( b == d->firstColumnIsLabel )
        return;

    beginResetModel();
    d->firstColumnIsLabel = b;
    
    rebuildDataMap();
    endResetModel();
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

void ChartProxyModel::setDataDirection( Qt::Orientation orientation )
{
    if ( d->dataDirection == orientation )
        return;

    beginResetModel();
    d->dataDirection = orientation;

    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::setDataDimensions( int dimensions )
{
    if ( d->dataDimensions == dimensions )
        return;

    beginResetModel();
    d->dataDimensions = dimensions;

    rebuildDataMap();
    endResetModel();
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
