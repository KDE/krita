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

using namespace KChart;


// ================================================================
//                     Class ChartProxyModel::Private


class ChartProxyModel::Private {
public:
    Private();
    ~Private();

    bool             firstRowIsLabel;
    bool             firstColumnIsLabel;
    Qt::Orientation  dataDirection;
    int              dataDimensions;
    
    QString categoryDataRegion;

    QVector< CellRegion > dataSetRegions;
    
    QList<DataSet*>  dataSets;
    QList<DataSet*>  removedDataSets;
    
    QVector<QRect>   selection;

    bool automaticDataSetCreation;
};

ChartProxyModel::Private::Private()
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


ChartProxyModel::ChartProxyModel()
    : QAbstractProxyModel( 0 ),
      d( new Private )
{
}

ChartProxyModel::~ChartProxyModel()
{
    delete d;
}


void ChartProxyModel::setAutomaticDataSetCreation( bool enable )
{
    d->automaticDataSetCreation = enable;
}

bool ChartProxyModel::automaticDataSetCreation() const
{
    return d->automaticDataSetCreation;
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
    if ( !d->automaticDataSetCreation )
        return;

    invalidateDataSets();
    d->dataSets = createDataSetsFromRegion( d->removedDataSets );
}

QList<DataSet*> ChartProxyModel::createDataSetsFromRegion( QList<DataSet*> dataSetsToRecycle )
{
    QList<DataSet*> createdDataSets;
    QVector<QRect> dataRegions;

    if ( d->selection.isEmpty() ) {
        const QRect dataBoundingRect( QPoint( 1, 1 ),
                                      QSize( sourceModel()->columnCount(),
                                             sourceModel()->rowCount() ) );
        dataRegions.append( dataBoundingRect );
    }
    else
        dataRegions = d->selection;
    
    int createdDataSetCount = 0;
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
        if ( ! categoryDataRegion().isEmpty() ) {
            category = CellRegion( categoryDataRegion() );
        } else if ( d->firstRowIsLabel && j.hasNext() ) {
            j.next();
            
            category = CellRegion( j.value() );
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
            
            dataSet->setNumber( createdDataSetCount );
            //dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );

            CellRegion labelDataRegion;

            CellRegion xDataRegion;
            // In case of > 1 data dimensions, x data appears before y data
            if ( d->dataDimensions > 1 )
                xDataRegion = CellRegion( j.value() );

            //qDebug() << "Creating data set with region" << j.value();
            if ( d->firstColumnIsLabel ) {
                CellRegion tmpRegion = CellRegion( j.value() );
                QPoint labelDataPoint = tmpRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( labelDataPoint );
            }

            if ( d->dataDimensions > 1 && j.hasNext() )
                j.next();
            
            CellRegion yDataRegion( j.value() );

            if ( d->firstColumnIsLabel ) {
                xDataRegion.subtract( xDataRegion.pointAtIndex( 0 ) );
                yDataRegion.subtract( yDataRegion.pointAtIndex( 0 ) );
            }
            
            if ( d->dataDimensions > 2 && j.hasNext() )
                j.next();
            // adding support for third dimension, even if the existing scheme does not scale if we have need for aditional dimensions
            CellRegion zDataRegion( j.value() );

            if ( d->firstColumnIsLabel ) {
                zDataRegion.subtract( zDataRegion.pointAtIndex( 0 ) );
            }

            dataSet->setXDataRegion( xDataRegion );
            dataSet->setYDataRegion( yDataRegion );
            dataSet->setCustomDataRegion( zDataRegion );
            dataSet->setCategoryDataRegion( category );
            dataSet->setLabelDataRegion( labelDataRegion );            
            dataSet->blockSignals( false );
            ++createdDataSetCount;
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
        if ( ! categoryDataRegion().isEmpty() ) {
            category = CellRegion( categoryDataRegion() );
        } else if ( d->firstColumnIsLabel && j.hasNext() ) {
            j.next();
            
            category = CellRegion( j.value() );
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
            
            dataSet->setNumber( createdDataSetCount );
            //dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );

            CellRegion labelDataRegion;
            
            CellRegion xDataRegion;
            // In case of > 1 data dimensions, x data appears before y data
            if ( d->dataDimensions > 1 )
                xDataRegion = CellRegion( j.value() );

            //qDebug() << "Creating data set with region" << j.value();
            if ( d->firstRowIsLabel ) {
                CellRegion tmpRegion = CellRegion( j.value() );
                QPoint labelDataPoint = tmpRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( labelDataPoint );
            }

            if ( d->dataDimensions > 1 && j.hasNext() )
                j.next();
            
            CellRegion yDataRegion( j.value() );

            if ( d->firstRowIsLabel ) {
                xDataRegion.subtract( xDataRegion.pointAtIndex( 0 ) );
                yDataRegion.subtract( yDataRegion.pointAtIndex( 0 ) );
            }
            
            if ( d->dataDimensions > 2 && j.hasNext() )
                j.next();
            
            CellRegion zDataRegion( j.value() );

            if ( d->firstRowIsLabel ) {
                zDataRegion.subtract( zDataRegion.pointAtIndex( 0 ) );
            }

            dataSet->setXDataRegion( xDataRegion );
            dataSet->setYDataRegion( yDataRegion );
            dataSet->setCustomDataRegion( zDataRegion );
            dataSet->setLabelDataRegion( labelDataRegion );
            dataSet->setCategoryDataRegion( category );
            dataSet->blockSignals( false );
            ++createdDataSetCount;
        }
    }

    return createdDataSets;
}


void ChartProxyModel::setSourceModel( QAbstractItemModel *sourceModel )
{
    if ( this->sourceModel() == sourceModel )
        return;

    beginResetModel();

    if ( this->sourceModel() ) {
        disconnect( this->sourceModel(), SIGNAL( modelReset() ),
                    this,                SLOT( slotModelReset() ) );
        disconnect( this->sourceModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                    this,                SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
    }

    if ( sourceModel ) {
        connect( sourceModel, SIGNAL( modelReset() ),
                 this,        SLOT( slotModelReset() ) );
        connect( sourceModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                 this,        SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
        connect( sourceModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
    }

    QAbstractProxyModel::setSourceModel( sourceModel );
    
    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::setSourceModel( QAbstractItemModel *model,
                                      const QVector<QRect> &selection )
{
    d->selection = selection;
    setSourceModel( model );
}

void ChartProxyModel::setSelection( const QVector<QRect> &selection )
{
    d->selection = selection;
    //needReset();
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
    // If it is specified, use createDataSetsFromRegion() to automatically
    // turn this data region into consecutive data series.
    // If cell ranges are in addition specified for one or more of these
    // data series, they'll be overwritten by these values.
    if ( element.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
    {
        QString cellRangeAddress = element.attributeNS( KoXmlNS::table, "cell-range-address" );
        setSelection( CellRegion::stringToRegion( cellRangeAddress ) );
        createdDataSets = createDataSetsFromRegion( d->removedDataSets );
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
                                int role /* = Qt::DisplayRole */ ) const
{
    if ( sourceModel() == 0 )
        return QVariant();
    
    QModelIndex sourceIndex = mapToSource( index );
    if ( sourceIndex == QModelIndex() ) {
        qWarning() << "ChartProxyModel::data(): Attempting to request data for invalid source index";
        qWarning() << "ChartProxyModel::data(): Mapping resulted in:";
        qWarning() << index << "-->" << sourceIndex;
        return QVariant();
    }

    QVariant value = sourceModel()->data( sourceIndex, role );
    return value;
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
    if ( sourceModel() == 0 )
        return QVariant();

    orientation = mapToSource( orientation );

    int row    = 0;
    int column = 0;

    if ( orientation == Qt::Horizontal ) {
        if ( !d->firstColumnIsLabel )
            return QVariant();

        // Return the first column in the section-th row
        row = section;
        if ( d->firstRowIsLabel )
            row++;
        // first source row is used for x values
        if ( d->dataDimensions == 2 )
            row++;
    }
    else {
        // orientation == Qt::Vertical here

        if ( !d->firstRowIsLabel )
            return QVariant();

        // Return the section-th column in the first row.
        column = section;
        if ( d->firstColumnIsLabel )
            column++;

        // First source column is used for X values.
        if ( d->dataDimensions == 2 )
            column++;
    }

    // Check for overflow in rows.
    if ( row >= sourceModel()->rowCount() ) {
        qWarning() << "ChartProxyModel::headerData(): Attempting to request header data for row >= rowCount";

        return QVariant();
    }

    // Check for overflow in columns.
    if ( column >= sourceModel()->columnCount() ) {
        qWarning() << "ChartProxyModel::headerData(): Attempting to request header data for column >= columnCount";

        return QVariant();
    }

    return sourceModel()->data( sourceModel()->index( row, column ), role );
}


QMap<int, QVariant> ChartProxyModel::itemData( const QModelIndex &index ) const
{
    return sourceModel()->itemData( mapToSource( index ) );
}


QModelIndex ChartProxyModel::index( int row,
                                    int column,
                                    const QModelIndex &parent /* = QModelIndex() */ ) const
{
    Q_UNUSED( parent );

    return QAbstractItemModel::createIndex( row, column, 0 );
}


QModelIndex ChartProxyModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

QModelIndex ChartProxyModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
    Q_UNUSED( sourceIndex );
    return QModelIndex();
}

QModelIndex ChartProxyModel::mapToSource( const QModelIndex &proxyIndex ) const
{
    Q_UNUSED( proxyIndex );
    return QModelIndex();
}


Qt::Orientation ChartProxyModel::mapFromSource( Qt::Orientation orientation ) const
{
    // In fact, this method does exactly the same thing as
    // mapToSource( Qt::Orientation ), but replacing the code with a
    // call to mapToSource() would just confuse at this point.

    if ( d->dataDirection == Qt::Horizontal )
        return orientation;

    // Orientation is Qt::Horizontal
    // Thus, we need to return the opposite of orientation.
    if ( orientation == Qt::Vertical )
        return Qt::Horizontal;

    return Qt::Vertical;
}


Qt::Orientation ChartProxyModel::mapToSource( Qt::Orientation orientation ) const
{
    if ( d->dataDirection == Qt::Horizontal )
        return orientation;

    // Orientation is Qt::Horizontal.
    // Thus, we need to return the opposite of orientation.
    if ( orientation == Qt::Vertical )
        return Qt::Horizontal;

    return Qt::Vertical;
}

int ChartProxyModel::rowCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    if ( sourceModel() == 0 )
        return 0;

    int rowCount;
    if ( d->dataDirection == Qt::Horizontal )
        rowCount = sourceModel()->rowCount( parent );
    else
        rowCount = sourceModel()->columnCount( parent );

    // Even if the first row is a header - if the data table is empty,
    // we still have 0 rows, not -1

    bool firstRowIsLabel = d->firstRowIsLabel;
    if ( d->dataDirection == Qt::Vertical )
        firstRowIsLabel = d->firstColumnIsLabel;

    if ( rowCount > 0 && firstRowIsLabel )
        rowCount--;
    
    // One row is used for x values
    if ( d->dataDimensions == 2 )
        rowCount--;
    
    rowCount *= d->dataDimensions;

    return rowCount;
}


int ChartProxyModel::columnCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    if ( sourceModel() == 0 )
        return 0;
    
    int columnCount;
    if ( d->dataDirection == Qt::Horizontal )
        columnCount = sourceModel()->columnCount( parent );
    else
        columnCount = sourceModel()->rowCount( parent );

    // Even if the first column is a header - if the data table is empty,
    // we still have 0 columns, not -1

    bool firstColumnIsLabel = d->firstColumnIsLabel;
    if ( d->dataDirection == Qt::Vertical )
        firstColumnIsLabel = d->firstRowIsLabel;

    if ( columnCount > 0 && firstColumnIsLabel )
        columnCount--;
    
    return columnCount;
}

void ChartProxyModel::setFirstRowIsLabel( bool b )
{
    if ( b == d->firstRowIsLabel )
        return;

    beginResetModel();
    
    d->firstRowIsLabel = b;
    
    if ( !sourceModel() )
        return;
    
    rebuildDataMap();
    endResetModel();
}
 

void ChartProxyModel::setFirstColumnIsLabel( bool b )
{
    if ( b == d->firstColumnIsLabel )
        return;

    beginResetModel();
    d->firstColumnIsLabel = b;

    if ( !sourceModel() )
        return;
    
    rebuildDataMap();
    endResetModel();
}

Qt::Orientation ChartProxyModel::dataDirection()
{
    return d->dataDirection;
}

void ChartProxyModel::invalidateDataSets()
{
    foreach ( DataSet *dataSet, d->dataSets )
    {
        if ( dataSet->attachedAxis() ) {
            // Remove data sets 'silently'. Once the last data set
            // has been detached from an axis, the axis will delete
            // all models and diagrams associated with it, thus we
            // do not need to propagate these events to any models.
            dataSet->attachedAxis()->detachDataSet( dataSet, true );
        }
    }

    d->removedDataSets = d->dataSets;
    d->dataSets.clear();
}

void ChartProxyModel::setDataDirection( Qt::Orientation orientation )
{
    if ( d->dataDirection == orientation )
        return;

    beginResetModel();
    d->dataDirection = orientation;

    if ( !sourceModel() )
        return;

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

QString ChartProxyModel::categoryDataRegion() const
{
    return d->categoryDataRegion;
}

void ChartProxyModel::setCategoryDataRegion(const QString& region)
{
    d->categoryDataRegion = region;
}

QList<DataSet*> ChartProxyModel::dataSets() const
{
    return d->dataSets;
}

void ChartProxyModel::slotRowsInserted( const QModelIndex &parent, 
                                        int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );

    beginResetModel();
    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::slotColumnsInserted( const QModelIndex &parent,
                                           int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );

    beginResetModel();
    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::slotRowsRemoved( const QModelIndex &parent,
                                       int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );

    beginResetModel();
    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::slotColumnsRemoved( const QModelIndex &parent,
                                          int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );

    beginResetModel();
    rebuildDataMap();
    endResetModel();
}

void ChartProxyModel::slotModelReset()
{
    rebuildDataMap();
    reset(); // propagate
}

#include "ChartProxyModel.moc"
