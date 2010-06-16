/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartLineDiagram.h"
#include "KDChartLineDiagram_p.h"

#include "KDChartBarDiagram.h"
#include "KDChartPalette.h"
#include "KDChartPosition.h"
#include "KDChartTextAttributes.h"
#include "KDChartAttributesModel.h"
#include "KDChartAbstractGrid.h"
#include "KDChartDataValueAttributes.h"

#include <KDABLibFakes>

#include "KDChartNormalLineDiagram_p.h"
#include "KDChartStackedLineDiagram_p.h"
#include "KDChartPercentLineDiagram_p.h"

#include <QDebug>
#include <QPainter>
#include <QString>
#include <QPainterPath>
#include <QPen>
#include <QVector>

using namespace KDChart;

LineDiagram::Private::Private()
{
}

LineDiagram::Private::~Private() {}


#define d d_func()


LineDiagram::LineDiagram( QWidget* parent, CartesianCoordinatePlane* plane ) :
    AbstractCartesianDiagram( new Private(), parent, plane )
{
    init();
}

void LineDiagram::init()
{
    d->diagram = this;
    d->normalDiagram = new NormalLineDiagram( this );
    d->stackedDiagram = new StackedLineDiagram( this );
    d->percentDiagram = new PercentLineDiagram( this );
    d->implementor = d->normalDiagram;
    d->centerDataPoints = false;
}

LineDiagram::~LineDiagram()
{
}

/**
  * Creates an exact copy of this diagram.
  */
LineDiagram * LineDiagram::clone() const
{
    LineDiagram* newDiagram = new LineDiagram( new Private( *d ) );
    newDiagram->setType( type() );
    return newDiagram;
}


bool LineDiagram::compare( const LineDiagram* other )const
{
    if( other == this ) return true;
    if( ! other ){
        return false;
    }
    /*
    qDebug() <<"\n             LineDiagram::compare():";
            // compare own properties
    qDebug() << (type() == other->type());
    */
    return  // compare the base class
            ( static_cast<const AbstractCartesianDiagram*>(this)->compare( other ) ) &&
            // compare own properties
            (type()             == other->type()) &&
            (centerDataPoints() == other->centerDataPoints());
}

/**
  * Sets the line diagram's type to \a type
  * \sa LineDiagram::LineType
  */
void LineDiagram::setType( const LineType type )
{
    if ( d->implementor->type() == type ) return;
   if ( type != LineDiagram::Normal && datasetDimension() > 1 ) {
       Q_ASSERT_X ( false, "setType()",
                    "This line chart type can't be used with multi-dimensional data." );
       return;
   }
   switch( type ) {
   case Normal:
       d->implementor = d->normalDiagram;
       break;
   case Stacked:
       d->implementor = d->stackedDiagram;
       break;
   case Percent:
       d->implementor = d->percentDiagram;
       break;
   default:
       Q_ASSERT_X( false, "LineDiagram::setType", "unknown diagram subtype" );
   };

   // d->lineType = type;
   Q_ASSERT( d->implementor->type() == type );

   // AbstractAxis settings - see AbstractDiagram and CartesianAxis
   setPercentMode( type == LineDiagram::Percent );
   setDataBoundariesDirty();
   emit layoutChanged( this );
   emit propertiesChanged();
}

/**
  * @return the type of the line diagram
  */
LineDiagram::LineType LineDiagram::type() const
{
    return d->implementor->type();
}

void LineDiagram::setCenterDataPoints( bool center )
{
	d->centerDataPoints = center;
	emit propertiesChanged();
}

bool LineDiagram::centerDataPoints() const
{
	return d->centerDataPoints;
}

/**
  * Sets the global line attributes to \a la
  */
void LineDiagram::setLineAttributes( const LineAttributes& la )
{
    d->attributesModel->setModelData(
        qVariantFromValue( la ),
        LineAttributesRole );
    emit propertiesChanged();
}

/**
  * Sets the line attributes of data set \a column to \a la
  */
void LineDiagram::setLineAttributes(
        int column,
    const LineAttributes& la )
{
    d->attributesModel->setHeaderData(
            column,
            Qt::Vertical,
            qVariantFromValue( la ),
            LineAttributesRole );
    emit propertiesChanged();
}

/**
  * Resets the line attributes of data set \a column
  */
void LineDiagram::resetLineAttributes( int column )
{
    d->attributesModel->resetHeaderData(
            column, Qt::Vertical, LineAttributesRole );
    emit propertiesChanged();
}

/**
  * Sets the line attributes for the model index \a index to \a la
  */
void LineDiagram::setLineAttributes(
        const QModelIndex& index,
    const LineAttributes& la )
{
    d->attributesModel->setData(
            d->attributesModel->mapFromSource(index),
    qVariantFromValue( la ),
    LineAttributesRole );
    emit propertiesChanged();
}

/**
 * Remove any explicit line attributes settings that might have been specified before.
 */
void LineDiagram::resetLineAttributes( const QModelIndex & index )
{
    d->attributesModel->resetData(
            d->attributesModel->mapFromSource(index), LineAttributesRole );
    emit propertiesChanged();
}

/**
  * @return the global line attribute set
  */
LineAttributes LineDiagram::lineAttributes() const
{
    return qVariantValue<LineAttributes>(
        d->attributesModel->data( KDChart::LineAttributesRole ) );
}

/**
  * @return the line attribute set of data set \a column
  */
LineAttributes LineDiagram::lineAttributes( int column ) const
{
    const QVariant attrs(
            d->attributesModel->headerData( column, Qt::Vertical,
                                            LineAttributesRole ) );
    if( attrs.isValid() )
        return qVariantValue< LineAttributes >( attrs );
    return lineAttributes();
}

/**
  * @return the line attribute set of the model index \a index
  */
LineAttributes LineDiagram::lineAttributes(
    const QModelIndex& index ) const
{
    return qVariantValue<LineAttributes>(
        d->attributesModel->data(
            d->attributesModel->mapFromSource(index),
            KDChart::LineAttributesRole ) );
}

/**
  * Sets the global 3D line attributes to \a la
  */
void LineDiagram::setThreeDLineAttributes(
    const ThreeDLineAttributes& la )
{
    setDataBoundariesDirty();
    d->attributesModel->setModelData(
        qVariantFromValue( la ),
        ThreeDLineAttributesRole );
   emit propertiesChanged();
}

/**
  * Sets the 3D line attributes of data set \a column to \a ta
  */
void LineDiagram::setThreeDLineAttributes(
    int column,
    const ThreeDLineAttributes& la )
{
    setDataBoundariesDirty();
    d->attributesModel->setHeaderData(
        column,
        Qt::Vertical,
        qVariantFromValue( la ),
        ThreeDLineAttributesRole );
   emit propertiesChanged();
}

/**
  * Sets the 3D line attributes of model index \a index to \a la
  */
void LineDiagram::setThreeDLineAttributes(
    const QModelIndex & index,
    const ThreeDLineAttributes& la )
{
    setDataBoundariesDirty();
    d->attributesModel->setData(
        d->attributesModel->mapFromSource(index),
        qVariantFromValue( la ),
        ThreeDLineAttributesRole );
   emit propertiesChanged();
}

/**
  * @return the global 3D line attributes
  */
ThreeDLineAttributes LineDiagram::threeDLineAttributes() const
{
    return qVariantValue<ThreeDLineAttributes>(
        d->attributesModel->data( KDChart::ThreeDLineAttributesRole ) );
}

/**
  * @return the 3D line attributes of data set \a column
  */
ThreeDLineAttributes LineDiagram::threeDLineAttributes( int column ) const
{
    const QVariant attrs(
            d->attributesModel->headerData( column, Qt::Vertical,
                                            ThreeDLineAttributesRole ) );
    if( attrs.isValid() )
        return qVariantValue< ThreeDLineAttributes >( attrs );
    return threeDLineAttributes();
}

/**
  * @return the 3D line attributes of the model index \a index
  */
ThreeDLineAttributes LineDiagram::threeDLineAttributes( const QModelIndex& index ) const
{
    return qVariantValue<ThreeDLineAttributes>(
        d->attributesModel->data(
            d->attributesModel->mapFromSource( index ),
            KDChart::ThreeDLineAttributesRole ) );
}

double LineDiagram::threeDItemDepth( const QModelIndex& index ) const
{
    return threeDLineAttributes( index ).validDepth();
}

double LineDiagram::threeDItemDepth( int column ) const
{
    return qVariantValue<ThreeDLineAttributes>(
        d->attributesModel->headerData (
            column,
            Qt::Vertical,
            KDChart::ThreeDLineAttributesRole ) ).validDepth();
}

/**
  * Sets the value tracker attributes of the model index \a index to \a va
  */
void LineDiagram::setValueTrackerAttributes( const QModelIndex & index,
                                             const ValueTrackerAttributes & va )
{
    d->attributesModel->setData( d->attributesModel->mapFromSource(index),
                                 qVariantFromValue( va ),
                                 KDChart::ValueTrackerAttributesRole );
    emit propertiesChanged();
}

/**
  * Returns the value tracker attributes of the model index \a index
  */
ValueTrackerAttributes LineDiagram::valueTrackerAttributes(
        const QModelIndex & index ) const
{
    return qVariantValue<ValueTrackerAttributes>( d->attributesModel->data(
            d->attributesModel->mapFromSource( index ),
            KDChart::ValueTrackerAttributesRole ) );
}

void LineDiagram::resizeEvent ( QResizeEvent* )
{
}

const QPair<QPointF, QPointF> LineDiagram::calculateDataBoundaries() const
{
    if ( !checkInvariants( true ) ) return QPair<QPointF, QPointF>( QPointF( 0, 0 ), QPointF( 0, 0 ) );

    // note: calculateDataBoundaries() is ignoring the hidden flags.
    //       That's not a bug but a feature: Hiding data does not mean removing them.
    // For totally removing data from KD Chart's view people can use e.g. a proxy model ...

    // calculate boundaries for different line types Normal - Stacked - Percent - Default Normal
    return d->implementor->calculateDataBoundaries();
}


void LineDiagram::paintEvent ( QPaintEvent*)
{
//qDebug() << "starting LineDiagram::paintEvent ( QPaintEvent*)";
    QPainter painter ( viewport() );
    PaintContext ctx;
    ctx.setPainter ( &painter );
    ctx.setRectangle ( QRectF ( 0, 0, width(), height() ) );
    paint ( &ctx );
//qDebug() << "         LineDiagram::paintEvent ( QPaintEvent*) ended.";
}


double LineDiagram::valueForCellTesting( int row, int column,
                                         bool& bOK,
                                         bool showHiddenCellsAsInvalid ) const
{
    double value;
    if( showHiddenCellsAsInvalid && isHidden( model()->index( row, column, rootIndex() ) ) )
        bOK = false;
    else
        value = d->attributesModel->data(
                    d->attributesModel->index( row, column, attributesModelRootIndex() )
                ).toDouble( &bOK );
    return bOK ? value : 0.0;
}

LineAttributes::MissingValuesPolicy LineDiagram::getCellValues(
      int row, int column,
      bool shiftCountedXValuesByHalfSection,
      double& valueX, double& valueY ) const
{
    LineAttributes::MissingValuesPolicy policy;

    bool bOK = true;
    valueX = ( datasetDimension() > 1 && column > 0 )
             ? valueForCellTesting( row, column-1, bOK, true )
             : ((shiftCountedXValuesByHalfSection ? 0.5 : 0.0) + row);
    if( bOK )
        valueY = valueForCellTesting( row, column, bOK, true );
    if( bOK ){
        policy = LineAttributes::MissingValuesPolicyIgnored;
    }else{
        // missing value: find out the policy
        QModelIndex index = model()->index( row, column, rootIndex() );
        LineAttributes la = lineAttributes( index );
        policy = la.missingValuesPolicy();
    }
    return policy;
}

void LineDiagram::paint( PaintContext* ctx )
{
    // note: Not having any data model assigned is no bug
    //       but we can not draw a diagram then either.
    if ( !checkInvariants( true ) ) return;
    if ( !AbstractGrid::isBoundariesValid(dataBoundaries()) ) return;
    const PainterSaver p( ctx->painter() );
    if( model()->rowCount( rootIndex() ) == 0 || model()->columnCount( rootIndex() ) == 0 )
        return; // nothing to paint for us

    AbstractCoordinatePlane* const plane = ctx->coordinatePlane();
    ctx->setCoordinatePlane( plane->sharedAxisMasterPlane( ctx->painter() ) );


    // paint different line types Normal - Stacked - Percent - Default Normal
    d->implementor->paint( ctx );

    ctx->setCoordinatePlane( plane );
}

void LineDiagram::resize ( const QSizeF& size )
{
    d->compressor.setResolution( static_cast<int>( size.width() * coordinatePlane()->zoomFactorX() ),
                                 static_cast<int>( size.height() * coordinatePlane()->zoomFactorY() ) );
    setDataBoundariesDirty();
}

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
int LineDiagram::numberOfAbscissaSegments () const
{
    return d->attributesModel->rowCount(attributesModelRootIndex());
}

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
int LineDiagram::numberOfOrdinateSegments () const
{
    return d->attributesModel->columnCount(attributesModelRootIndex());
}
