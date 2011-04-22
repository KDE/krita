/****************************************************************************
** Copyright (C) 2001-2011 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDChartAttributesModel.h"
#include "KDChartPalette.h"
#include "KDChartGlobal.h"

#include <QDebug>
#include <QPen>
#include <QPointer>

#include <KDChartTextAttributes>
#include <KDChartFrameAttributes>
#include <KDChartBackgroundAttributes>
#include <KDChartDataValueAttributes>
#include <KDChartMarkerAttributes>
#include <KDChartBarAttributes>
#include <KDChartStockBarAttributes>
#include <KDChartLineAttributes>
#include <KDChartPieAttributes>
#include <KDChartAbstractThreeDAttributes>
#include <KDChartThreeDBarAttributes>
#include <KDChartThreeDLineAttributes>
#include <KDChartThreeDPieAttributes>
#include <KDChartGridAttributes>
#include <KDChartValueTrackerAttributes>

#include <KDABLibFakes>


using namespace KDChart;

AttributesModel::AttributesModel( QAbstractItemModel* model, QObject * parent/* = 0 */ )
  : AbstractProxyModel( parent ),
    mPaletteType( PaletteTypeDefault )
{
    setSourceModel(model);
    setDefaultForRole( KDChart::DataValueLabelAttributesRole,
                       DataValueAttributes::defaultAttributesAsVariant() );
}

AttributesModel::~AttributesModel()
{
}

void AttributesModel::initFrom( const AttributesModel* other )
{
    if( other == this || ! other ) return;

    mDataMap = other->mDataMap;
    mHorizontalHeaderDataMap = other->mHorizontalHeaderDataMap;
    mVerticalHeaderDataMap = other->mVerticalHeaderDataMap;
    mModelDataMap = other->mModelDataMap;
    mDefaultsMap =  other->mDefaultsMap;

    setPaletteType( other->paletteType() );
}

bool AttributesModel::compare( const AttributesModel* other )const
{
    if( other == this ) return true;
    if( ! other ){
        //qDebug() << "AttributesModel::compare() cannot compare to Null pointer";
        return false;
    }

    {
        if (mDataMap.count() != other->mDataMap.count()){
            //qDebug() << "AttributesModel::compare() dataMap have different sizes";
            return false;
        }
        QMap<int, QMap<int, QMap<int, QVariant> > >::const_iterator itA = mDataMap.constBegin();
        QMap<int, QMap<int, QMap<int, QVariant> > >::const_iterator itB = other->mDataMap.constBegin();
        while (itA != mDataMap.constEnd()) {
            if ((*itA).count() != (*itB).count()){
                //qDebug() << "AttributesModel::compare() dataMap/map have different sizes";
                return false;
            }
            QMap<int, QMap<int, QVariant> >::const_iterator it2A = (*itA).constBegin();
            QMap<int, QMap<int, QVariant> >::const_iterator it2B = (*itB).constBegin();
            while (it2A != itA->constEnd()) {
                if ((*it2A).count() != (*it2B).count()){
                    //qDebug() << "AttributesModel::compare() dataMap/map/map have different sizes:"
                    //        << (*it2A).count() << (*it2B).count();
                    return false;
                }
                QMap<int, QVariant>::const_iterator it3A = (*it2A).constBegin();
                QMap<int, QVariant>::const_iterator it3B = (*it2B).constBegin();
                while (it3A != it2A->constEnd()) {
                    if ( it3A.key() != it3B.key() ){
                        //qDebug( "AttributesModel::compare()\n"
                        //        "   dataMap[%i, %i] values have different types.  A: %x  B: %x",
                        //        itA.key(), it2A.key(), it3A.key(), it3B.key());
                        return false;
                    }
                    if ( ! compareAttributes( it3A.key(), it3A.value(), it3B.value() ) ){
                        //qDebug( "AttributesModel::compare()\n"
                        //        "   dataMap[%i, %i] values are different. Role: %x", itA.key(), it2A.key(), it3A.key());
                        return false;
                    }
                    ++it3A;
                    ++it3B;
                }
                ++it2A;
                ++it2B;
            }
            ++itA;
            ++itB;
        }
    }
    {
        if (mHorizontalHeaderDataMap.count() != other->mHorizontalHeaderDataMap.count()){
            //qDebug() << "AttributesModel::compare() horizontalHeaderDataMap have different sizes";
            return false;
        }
        QMap<int, QMap<int, QVariant> >::const_iterator itA = mHorizontalHeaderDataMap.constBegin();
        QMap<int, QMap<int, QVariant> >::const_iterator itB = other->mHorizontalHeaderDataMap.constBegin();
        while (itA != mHorizontalHeaderDataMap.constEnd()) {
            if ((*itA).count() != (*itB).count()){
                //qDebug() << "AttributesModel::compare() horizontalHeaderDataMap/map have different sizes";
                return false;
            }
            QMap<int, QVariant>::const_iterator it2A = (*itA).constBegin();
            QMap<int, QVariant>::const_iterator it2B = (*itB).constBegin();
            while (it2A != itA->constEnd()) {
                if ( it2A.key() != it2B.key() ){
                    //qDebug( "AttributesModel::compare()\n"
                    //        "   horizontalHeaderDataMap[ %i ] values have different types.  A: %x  B: %x",
                    //        itA.key(), it2A.key(), it2B.key());
                    return false;
                }
                if ( ! compareAttributes( it2A.key(), it2A.value(), it2B.value() ) ){
                    //qDebug( "AttributesModel::compare()\n"
                    //        "   horizontalHeaderDataMap[ %i ] values are different. Role: %x", itA.key(), it2A.key() );
                    return false;
                }
                ++it2A;
                ++it2B;
            }
            ++itA;
            ++itB;
        }
    }
    {
        if (mVerticalHeaderDataMap.count() != other->mVerticalHeaderDataMap.count()){
            //qDebug() << "AttributesModel::compare() verticalHeaderDataMap have different sizes";
            return false;
        }
        QMap<int, QMap<int, QVariant> >::const_iterator itA = mVerticalHeaderDataMap.constBegin();
        QMap<int, QMap<int, QVariant> >::const_iterator itB = other->mVerticalHeaderDataMap.constBegin();
        while (itA != mVerticalHeaderDataMap.constEnd()) {
            if ((*itA).count() != (*itB).count()){
                //qDebug() << "AttributesModel::compare() verticalHeaderDataMap/map have different sizes";
                return false;
            }
            QMap<int, QVariant>::const_iterator it2A = (*itA).constBegin();
            QMap<int, QVariant>::const_iterator it2B = (*itB).constBegin();
            while (it2A != itA->constEnd()) {
                if ( it2A.key() != it2B.key() ){
                    //qDebug( "AttributesModel::compare()\n"
                    //        "   verticalHeaderDataMap[ %i ] values have different types.  A: %x  B: %x",
                    //        itA.key(), it2A.key(), it2B.key());
                    return false;
                }
                if ( ! compareAttributes( it2A.key(), it2A.value(), it2B.value() ) ){
                    //qDebug( "AttributesModel::compare()\n"
                    //        "   verticalHeaderDataMap[ %i ] values are different. Role: %x", itA.key(), it2A.key() );
                    return false;
                }
                ++it2A;
                ++it2B;
            }
            ++itA;
            ++itB;
        }
    }
    {
        if (mModelDataMap.count() != other->mModelDataMap.count()){
            //qDebug() << "AttributesModel::compare() modelDataMap have different sizes:" << mModelDataMap.count() << other->mModelDataMap.count();
            return false;
        }
        QMap<int, QVariant>::const_iterator itA = mModelDataMap.constBegin();
        QMap<int, QVariant>::const_iterator itB = other->mModelDataMap.constBegin();
        while (itA != mModelDataMap.constEnd()) {
            if ( itA.key() != itB.key() ){
                //qDebug( "AttributesModel::compare()\n"
                //        "   modelDataMap values have different types.  A: %x  B: %x",
                //        itA.key(), itB.key());
                return false;
            }
            if ( ! compareAttributes( itA.key(), itA.value(), itB.value() ) ){
                //qDebug( "AttributesModel::compare()\n"
                //        "   modelDataMap values are different. Role: %x", itA.key() );
                return false;
            }
            ++itA;
            ++itB;
        }
    }
    if (paletteType() != other->paletteType()){
        //qDebug() << "AttributesModel::compare() palette types are different";
        return false;
    }
    return true;
}

bool AttributesModel::compareAttributes(
        int role, const QVariant& a, const QVariant& b )const
{
    if( isKnownAttributesRole( role ) ){
        switch( role ) {
            case DataValueLabelAttributesRole:
                return (qVariantValue<DataValueAttributes>( a ) ==
                        qVariantValue<DataValueAttributes>( b ));
            case DatasetBrushRole:
                return (qVariantValue<QBrush>( a ) ==
                        qVariantValue<QBrush>( b ));
            case DatasetPenRole:
                return (qVariantValue<QPen>( a ) ==
                        qVariantValue<QPen>( b ));
            case ThreeDAttributesRole:
                // As of yet there is no ThreeDAttributes class,
                // and the AbstractThreeDAttributes class is pure virtual,
                // so we ignore this role for now.
                // (khz, 04.04.2007)
                /*
                return (qVariantValue<ThreeDAttributes>( a ) ==
                        qVariantValue<ThreeDAttributes>( b ));
                */
                break;
            case LineAttributesRole:
                return (qVariantValue<LineAttributes>( a ) ==
                        qVariantValue<LineAttributes>( b ));
            case ThreeDLineAttributesRole:
                return (qVariantValue<ThreeDLineAttributes>( a ) ==
                        qVariantValue<ThreeDLineAttributes>( b ));
            case BarAttributesRole:
                return (qVariantValue<BarAttributes>( a ) ==
                        qVariantValue<BarAttributes>( b ));
            case StockBarAttributesRole:
                return (qVariantValue<StockBarAttributes>( a ) ==
                        qVariantValue<StockBarAttributes>( b ));
            case ThreeDBarAttributesRole:
                return (qVariantValue<ThreeDBarAttributes>( a ) ==
                        qVariantValue<ThreeDBarAttributes>( b ));
            case PieAttributesRole:
                return (qVariantValue<PieAttributes>( a ) ==
                        qVariantValue<PieAttributes>( b ));
            case ThreeDPieAttributesRole:
                return (qVariantValue<ThreeDPieAttributes>( a ) ==
                        qVariantValue<ThreeDPieAttributes>( b ));
            case ValueTrackerAttributesRole:
                return (qVariantValue<ValueTrackerAttributes>( a ) ==
                        qVariantValue<ValueTrackerAttributes>( b ));
            case DataHiddenRole:
                return (qVariantValue<bool>( a ) ==
                        qVariantValue<bool>( b ));
            default:
                Q_ASSERT( false ); // all of our own roles need to be handled
                break;
        }
    }else{
        return (a == b);
    }
    return true;
}


QVariant AttributesModel::headerData ( int section,
                                       Qt::Orientation orientation,
                                       int role/* = Qt::DisplayRole */ ) const
{
  if( sourceModel() ) {
      const QVariant sourceData = sourceModel()->headerData( section, orientation, role );
      if ( sourceData.isValid() ) return sourceData;
  }

  if( orientation == Qt::Horizontal && role == ColumnDataRole )
  {
    // it seems the source model doesn't like the idea of handing out all the column data at once...
    // so we have to do it manually.
    QVariantList result;
    const int rows = sourceModel()->rowCount();
    for( int row = 0; row < rows; ++row )
        result.push_back( sourceModel()->index( row, section ).data() );

    return result;
  }


  // the source model didn't have data set, let's use our stored values
  const QMap<int, QMap<int, QVariant> >& map = orientation == Qt::Horizontal ? mHorizontalHeaderDataMap : mVerticalHeaderDataMap;
  if ( map.contains( section ) ) {
      const QMap<int, QVariant> &dataMap = map[ section ];
      if ( dataMap.contains( role ) ) {
          return dataMap[ role ];
      }
  }

  return defaultHeaderData( section, orientation, role );
}


QVariant AttributesModel::defaultHeaderData ( int section, Qt::Orientation orientation, int role ) const
{
  // Default values if nothing else matches
  switch ( role ) {
  case Qt::DisplayRole:
      //TODO for KDChart 3.0: Change to "return QString::number( section+1 );"
      return QLatin1String( orientation == Qt::Vertical ?  "Series " : "Item " ) + QString::number( section ) ;

  case KDChart::DatasetBrushRole: {
      if ( paletteType() == PaletteTypeSubdued )
          return Palette::subduedPalette().getBrush( section );
      else if ( paletteType() == PaletteTypeRainbow )
          return Palette::rainbowPalette().getBrush( section );
      else if ( paletteType() == PaletteTypeDefault )
          return Palette::defaultPalette().getBrush( section );
      else
          qWarning("Unknown type of fallback palette!");
  } break;
  case KDChart::DatasetPenRole: {
      // default to the color set for the brush (or it's defaults)
      // but only if no per model override was set
      if ( !modelData( role ).isValid() ) {
          QBrush brush = qVariantValue<QBrush>( headerData( section, orientation, DatasetBrushRole ) );
          return QPen( brush.color() );
      }
  } break;
  default:
      break;
  }

  return QVariant();
}

// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
QVariant AttributesModel::data( int role ) const
{
  if ( isKnownAttributesRole( role ) ) {
      // check if there is something set at global level
      QVariant v = modelData( role );

      // else return the default setting, if any
      if ( !v.isValid() )
          v = defaultsForRole( role );
      return v;
  }
  return QVariant();
}


// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
QVariant AttributesModel::data( int column, int role ) const
{
  if ( isKnownAttributesRole( role ) ) {
      // check if there is something set for the column (dataset)
      QVariant v;
      v = headerData( column, Qt::Horizontal, role );

      // check if there is something set at global level
      if ( !v.isValid() )
          v = data( role ); // includes automatic fallback to default
      return v;
  }
  return QVariant();
}


QVariant AttributesModel::data( const QModelIndex& index, int role ) const
{
    //qDebug() << "AttributesModel::data(" << index << role << ")";
    if( index.isValid() ) {
        Q_ASSERT( index.model() == this );
    }

    if( sourceModel() == 0 )
        return QVariant();

    if( index.isValid() )
    {
        const QVariant sourceData = sourceModel()->data( mapToSource(index), role );
        if( sourceData.isValid() )
            return sourceData;
    }

    // check if we are storing a value for this role at this cell index
    if( mDataMap.contains( index.column() ) )
    {
        const QMap< int,  QMap< int, QVariant > >& colDataMap = mDataMap[ index.column() ];
        if( colDataMap.contains( index.row() ) ) 
        {
            const QMap< int, QVariant >& dataMap = colDataMap[ index.row() ];
            if( dataMap.contains( role ) )
            {
              const QVariant v = dataMap[ role ];
              if( v.isValid() )
                  return v;
            }
        }
    }
    // check if there is something set for the column (dataset), or at global level
    if( index.isValid() )
        return data( index.column(), role ); // includes automatic fallback to default

    return QVariant();
}


bool AttributesModel::isKnownAttributesRole( int role ) const
{
    bool oneOfOurs = false;
    switch( role ) {
        // fallthrough intended
    case DataValueLabelAttributesRole:
    case DatasetBrushRole:
    case DatasetPenRole:
    case ThreeDAttributesRole:
    case LineAttributesRole:
    case ThreeDLineAttributesRole:
    case BarAttributesRole:
    case StockBarAttributesRole:
    case ThreeDBarAttributesRole:
    case PieAttributesRole:
    case ThreeDPieAttributesRole:
    case ValueTrackerAttributesRole:
    case DataHiddenRole:
        oneOfOurs = true;
    default:
        break;
    }
    return oneOfOurs;
}

QVariant AttributesModel::defaultsForRole( int role ) const
{
    // returns default-constructed QVariant if not found
    return mDefaultsMap.value( role );
}

bool AttributesModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
    if ( !isKnownAttributesRole( role ) ) {
        return sourceModel()->setData( mapToSource(index), value, role );
    } else {
        QMap< int,  QMap< int, QVariant> > &colDataMap = mDataMap[ index.column() ];
        QMap<int, QVariant> &dataMap = colDataMap[ index.row() ];
        //qDebug() <<  "AttributesModel::setData" <<"role" << role << "value" << value;
        dataMap.insert( role, value );
        emit attributesChanged( index, index );
        return true;
    }
}

bool AttributesModel::resetData ( const QModelIndex & index, int role )
{
    return setData ( index, QVariant(), role );
}

bool AttributesModel::setHeaderData ( int section, Qt::Orientation orientation,
                                      const QVariant & value, int role )
{
    if( sourceModel() != 0 && headerData( section, orientation, role ) == value )
        return true;
    if ( !isKnownAttributesRole( role ) ) {
        return sourceModel()->setHeaderData( section, orientation, value, role );
    } else {
        QMap<int,  QMap<int, QVariant> > &sectionDataMap
            = orientation == Qt::Horizontal ? mHorizontalHeaderDataMap : mVerticalHeaderDataMap;
        QMap<int, QVariant> &dataMap = sectionDataMap[ section ];
        dataMap.insert( role, value );
        if( sourceModel() ){
            emit attributesChanged( index( 0, section, QModelIndex() ),
                                    index( rowCount( QModelIndex() ), section, QModelIndex() ) );
            emit headerDataChanged( orientation, section, section );
            if ( section != -1 )
                emit dataChanged( index( 0, section, QModelIndex() ),
                                        index( rowCount( QModelIndex() ) - 1, section, QModelIndex() ) );
        }
        return true;
    }
}

bool AttributesModel::resetHeaderData ( int section, Qt::Orientation orientation, int role )
{
    return setHeaderData ( section, orientation, QVariant(), role );
}

void AttributesModel::setPaletteType( AttributesModel::PaletteType type )
{
    mPaletteType = type;
}

AttributesModel::PaletteType AttributesModel::paletteType() const
{
    return mPaletteType;
}

bool KDChart::AttributesModel::setModelData( const QVariant value, int role )
{
    mModelDataMap.insert( role, value );
    if( sourceModel() ){
        emit attributesChanged( index( 0, 0, QModelIndex() ),
                                index( rowCount( QModelIndex() ),
                                       columnCount( QModelIndex() ), QModelIndex() ) );
    }
    return true;
}

QVariant KDChart::AttributesModel::modelData( int role ) const
{
    return mModelDataMap.value( role, QVariant() );
}

int AttributesModel::rowCount( const QModelIndex& index ) const
{
    if ( sourceModel() ) {
        return sourceModel()->rowCount( mapToSource(index) );
    } else {
        return 0;
    }
}

int AttributesModel::columnCount( const QModelIndex& index ) const
{
    if ( sourceModel() ) {
        return sourceModel()->columnCount( mapToSource(index) );
    } else {
        return 0;
    }
}

void AttributesModel::setSourceModel( QAbstractItemModel* sourceModel )
{
    if( this->sourceModel() != 0 )
    {
        disconnect( this->sourceModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex&)),
                                   this, SLOT( slotDataChanged( const QModelIndex&, const QModelIndex&)));
        disconnect( this->sourceModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotRowsAboutToBeInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotRowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsAboutToBeInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotColumnsAboutToBeInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsAboutToBeRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( slotColumnsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( modelReset() ),
                                   this, SIGNAL( modelReset() ) );
        disconnect( this->sourceModel(), SIGNAL( layoutChanged() ),
                                   this, SIGNAL( layoutChanged() ) );
    }
    QAbstractProxyModel::setSourceModel( sourceModel );
    if( this->sourceModel() != NULL )
    {
        connect( this->sourceModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex&)),
                                this, SLOT( slotDataChanged( const QModelIndex&, const QModelIndex&)));
        connect( this->sourceModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( rowsAboutToBeInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( slotRowsAboutToBeInserted( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( slotRowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( columnsAboutToBeInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( slotColumnsAboutToBeInserted( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( columnsAboutToBeRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( slotColumnsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
        connect( this->sourceModel(), SIGNAL( modelReset() ),
                                this, SIGNAL( modelReset() ) );
        connect( this->sourceModel(), SIGNAL( layoutChanged() ),
                                this, SIGNAL( layoutChanged() ) );
    }
}

void AttributesModel::slotRowsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    beginInsertRows( mapFromSource( parent ), start, end );
}

void AttributesModel::slotColumnsAboutToBeInserted( const QModelIndex& parent, int start, int end )
{
    beginInsertColumns( mapFromSource( parent ), start, end );
}

void AttributesModel::slotRowsInserted( const QModelIndex& parent, int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endInsertRows();
}

void AttributesModel::slotColumnsInserted( const QModelIndex& parent, int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endInsertColumns();
}

void AttributesModel::slotRowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    beginRemoveRows( mapFromSource( parent ), start, end );
}

void AttributesModel::slotColumnsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
    beginRemoveColumns( mapFromSource( parent ), start, end );
}

void AttributesModel::slotRowsRemoved( const QModelIndex& parent, int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    endRemoveRows();
}

void AttributesModel::removeEntriesFromDataMap( int start, int end )
{
    QMap<int, QMap<int, QMap<int, QVariant> > >::iterator it = mDataMap.find( end );
    // check that the element was found
    if ( it != mDataMap.end() )
    {
        ++it;
        QVector< int > indexesToDel;
        for ( int i = start; i < end && it != mDataMap.end(); ++i )
        {
            mDataMap[ i ] = it.value();
            indexesToDel << it.key();
            ++it;
        }
        if ( indexesToDel.isEmpty() )
        {
            for ( int i = start; i < end; ++i )
            {
                indexesToDel << i;
            }
        }
        for ( int i  = 0; i < indexesToDel.count(); ++i )
        {
            mDataMap.remove( indexesToDel[ i ] );
        }
    }
}

void AttributesModel::removeEntriesFromDirectionDataMaps( Qt::Orientation dir, int start, int end )
{
    QMap<int,  QMap<int, QVariant> > &sectionDataMap
        = dir == Qt::Horizontal ? mHorizontalHeaderDataMap : mVerticalHeaderDataMap;
    QMap<int, QMap<int, QVariant> >::iterator it = sectionDataMap.upperBound( end );
    // check that the element was found
    if ( it != sectionDataMap.end() )
    {
        QVector< int > indexesToDel;
        for ( int i = start; i < end && it != sectionDataMap.end(); ++i )
        {
            sectionDataMap[ i ] = it.value();
            indexesToDel << it.key();
            ++it;
        }
        if ( indexesToDel.isEmpty() )
        {
            for ( int i = start; i < end; ++i )
            {
                indexesToDel << i;
            }
        }
        for ( int i  = 0; i < indexesToDel.count(); ++i )
        {
            sectionDataMap.remove( indexesToDel[ i ] );
        }
    }
}

void AttributesModel::slotColumnsRemoved( const QModelIndex& parent, int start, int end )
{
    Q_UNUSED( parent );
    Q_UNUSED( start );
    Q_UNUSED( end );
    Q_ASSERT_X( sourceModel(), "removeColumn", "This should only be triggered if a valid source Model exists! " );
    for ( int i = start; i <= end; ++i )
    {
        mVerticalHeaderDataMap.remove( start );
    }
    removeEntriesFromDataMap( start, end );
    removeEntriesFromDirectionDataMaps( Qt::Horizontal, start, end );
    removeEntriesFromDirectionDataMaps( Qt::Vertical, start, end );

    endRemoveColumns();
}

void AttributesModel::slotDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    emit dataChanged( mapFromSource( topLeft ), mapFromSource( bottomRight ) );
}

/** needed for serialization */
const QMap<int, QMap<int, QMap<int, QVariant> > > AttributesModel::dataMap()const
{
    return mDataMap;
}
/** needed for serialization */
const QMap<int, QMap<int, QVariant> > AttributesModel::horizontalHeaderDataMap()const
{
    return mHorizontalHeaderDataMap;
}
/** needed for serialization */
const QMap<int, QMap<int, QVariant> > AttributesModel::verticalHeaderDataMap()const
{
    return mVerticalHeaderDataMap;
}
/** needed for serialization */
const QMap<int, QVariant> AttributesModel::modelDataMap()const
{
    return mModelDataMap;
}

/** needed for serialization */
void AttributesModel::setDataMap( const QMap<int, QMap<int, QMap<int, QVariant> > > map )
{
    mDataMap = map;
}
/** needed for serialization */
void AttributesModel::setHorizontalHeaderDataMap( const QMap<int, QMap<int, QVariant> > map )
{
    mHorizontalHeaderDataMap = map;
}
/** needed for serialization */
void AttributesModel::setVerticalHeaderDataMap( const QMap<int, QMap<int, QVariant> > map )
{
    mVerticalHeaderDataMap = map;
}
/** needed for serialization */
void AttributesModel::setModelDataMap( const QMap<int, QVariant> map )
{
    mModelDataMap = map;
}

void AttributesModel::setDefaultForRole( int role, const QVariant& value )
{
    if ( value.isValid() ) {
        mDefaultsMap.insert( role, value );
    } else {
        // erase the possibily existing value to not let the map grow:
        QMap<int, QVariant>::iterator it = mDefaultsMap.find( role );
        if ( it != mDefaultsMap.end() ) {
            mDefaultsMap.erase( it );
        }
    }

    Q_ASSERT( defaultsForRole( role ) == value );
}
