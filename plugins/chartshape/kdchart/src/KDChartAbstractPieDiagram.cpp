/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartAbstractPieDiagram.h"
#include "KDChartAbstractPieDiagram_p.h"

#include "KDChartAttributesModel.h"
#include "KDChartPieAttributes.h"
#include "KDChartThreeDPieAttributes.h"

#include <QMap>

#include <KDABLibFakes>


using namespace KDChart;

AbstractPieDiagram::Private::Private() :
    granularity( 1.0 )
{
}

AbstractPieDiagram::Private::~Private() {}

AbstractPieDiagram::AbstractPieDiagram( QWidget* parent, PolarCoordinatePlane *plane ) :
    AbstractPolarDiagram( new Private(), parent, plane )
{
    init();
}

AbstractPieDiagram::~AbstractPieDiagram()
{
}


void AbstractPieDiagram::init()
{
}


bool AbstractPieDiagram::compare( const AbstractPieDiagram* other )const
{
    if( other == this ) return true;
    if( ! other ){
        //qDebug() << "AbstractPieDiagram::compare() cannot compare to Null pointer";
        return false;
    }
    /*
    qDebug() << "\n             AbstractPieDiagram::compare():";
            // compare own properties
    qDebug() <<
            (granularity() == other->granularity()) &&
            (startPosition() == other->startPosition());
    */
    return  // compare the base class
            ( static_cast<const AbstractPolarDiagram*>(this)->compare( other ) ) &&
            // compare own properties
            (granularity() == other->granularity()) &&
            (startPosition() == other->startPosition());
}


#define d d_func()

void AbstractPieDiagram::setGranularity( qreal value )
{
    d->granularity = value;
}

qreal AbstractPieDiagram::granularity() const
{
    return (d->granularity < 0.05 || d->granularity > 36.0)
            ? 1.0
    : d->granularity;
}


void AbstractPieDiagram::setStartPosition( int degrees )
{
    Q_UNUSED( degrees );
    qWarning() << "Deprecated AbstractPieDiagram::setStartPosition() called, setting ignored.";
}

int AbstractPieDiagram::startPosition() const
{
    qWarning() << "Deprecated AbstractPieDiagram::startPosition() called.";
    return 0;
}

void AbstractPieDiagram::setPieAttributes( const PieAttributes & attrs )
{
    d->attributesModel->setModelData( qVariantFromValue( attrs ), PieAttributesRole );
    emit layoutChanged( this );
}

void AbstractPieDiagram::setPieAttributes( int column, const PieAttributes & attrs )
{
    d->attributesModel->setHeaderData(
        column, Qt::Vertical, qVariantFromValue( attrs ), PieAttributesRole );
    emit layoutChanged( this );
}

void AbstractPieDiagram::setPieAttributes( const QModelIndex & index, const PieAttributes & attrs )
{
	d->attributesModel->setData( index, qVariantFromValue( attrs), PieAttributesRole );
	emit layoutChanged( this );
}

// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
PieAttributes AbstractPieDiagram::pieAttributes() const
{
    return qVariantValue<PieAttributes>(
        d->attributesModel->data( PieAttributesRole ) );
}

// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
PieAttributes AbstractPieDiagram::pieAttributes( int column ) const
{
    const QVariant attrs(
            d->attributesModel->headerData( column, Qt::Vertical,
                                            PieAttributesRole ) );
    if( attrs.isValid() )
        return qVariantValue< PieAttributes >( attrs );
    return pieAttributes();
}

PieAttributes AbstractPieDiagram::pieAttributes( const QModelIndex & index ) const
{
    return qVariantValue<PieAttributes>(
        d->attributesModel->data(
            d->attributesModel->mapFromSource( index ),
            PieAttributesRole ) );
}


void AbstractPieDiagram::setThreeDPieAttributes( const ThreeDPieAttributes & tda )
{
    d->attributesModel->setModelData( qVariantFromValue( tda ), ThreeDPieAttributesRole );
    emit layoutChanged( this );
}

void AbstractPieDiagram::setThreeDPieAttributes( int column, const ThreeDPieAttributes & tda )
{
    d->attributesModel->setHeaderData(
        column, Qt::Vertical, qVariantFromValue( tda ), ThreeDPieAttributesRole );
    emit layoutChanged( this );
}

void AbstractPieDiagram::setThreeDPieAttributes( const QModelIndex & index, const ThreeDPieAttributes & tda )
{
    model()->setData( index, qVariantFromValue( tda ), ThreeDPieAttributesRole );
    emit layoutChanged( this );
}

// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
ThreeDPieAttributes AbstractPieDiagram::threeDPieAttributes() const
{
    return qVariantValue<ThreeDPieAttributes>(
        d->attributesModel->data( ThreeDPieAttributesRole ) );
}

// Note: Our users NEED this method - even if
//       we do not need it at drawing time!
//       (khz, 2006-07-28)
ThreeDPieAttributes AbstractPieDiagram::threeDPieAttributes( int column ) const
{
    const QVariant attrs(
            d->attributesModel->headerData( column, Qt::Vertical,
                                            ThreeDPieAttributesRole ) );
    if( attrs.isValid() )
        return qVariantValue< ThreeDPieAttributes >( attrs );
    return threeDPieAttributes();
}

ThreeDPieAttributes AbstractPieDiagram::threeDPieAttributes( const QModelIndex & index ) const
{
    return qVariantValue<ThreeDPieAttributes>(
        d->attributesModel->data(
            d->attributesModel->mapFromSource( index ),
            ThreeDPieAttributesRole ) );
}

