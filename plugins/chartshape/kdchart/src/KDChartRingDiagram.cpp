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

#include "KDChartRingDiagram.h"
#include "KDChartRingDiagram_p.h"

#include "KDChartAttributesModel.h"
#include "KDChartPaintContext.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartPieAttributes.h"
#include "KDChartDataValueAttributes.h"

#include <QPainter>

#include <KDABLibFakes>

using namespace KDChart;

RingDiagram::Private::Private()
    : relativeThickness( false )
    , expandWhenExploded( false )
{
}

RingDiagram::Private::~Private() {}

#define d d_func()

RingDiagram::RingDiagram( QWidget* parent, PolarCoordinatePlane* plane ) :
    AbstractPieDiagram( new Private(), parent, plane )
{
    init();
}

RingDiagram::~RingDiagram()
{
}

void RingDiagram::init()
{
}

/**
  * Creates an exact copy of this diagram.
  */
RingDiagram * RingDiagram::clone() const
{
    return new RingDiagram( new Private( *d ) );
}

bool RingDiagram::compare( const RingDiagram* other )const
{
    if( other == this ) return true;
    if( ! other ){
        return false;
    }
    /*
    qDebug() <<"\n             RingDiagram::compare():";
            // compare own properties
    qDebug() << (type() == other->type());
    qDebug() << (relativeThickness()  == other->relativeThickness());
    qDebug() << (expandWhenExploded() == other->expandWhenExploded());
    */
    return  // compare the base class
            ( static_cast<const AbstractPieDiagram*>(this)->compare( other ) ) &&
            // compare own properties
            (relativeThickness()  == other->relativeThickness()) &&
            (expandWhenExploded() == other->expandWhenExploded());
}

void RingDiagram::setRelativeThickness( bool relativeThickness )
{
    d->relativeThickness = relativeThickness;
}

bool RingDiagram::relativeThickness() const
{
    return d->relativeThickness;
}

void RingDiagram::setExpandWhenExploded( bool expand )
{
	d->expandWhenExploded = expand;
}

bool RingDiagram::expandWhenExploded() const
{
	return d->expandWhenExploded;
}

const QPair<QPointF, QPointF> RingDiagram::calculateDataBoundaries () const
{
    if ( !checkInvariants( true ) ) return QPair<QPointF, QPointF>( QPointF( 0, 0 ), QPointF( 0, 0 ) );

    const PieAttributes attrs( pieAttributes( model()->index( 0, 0, rootIndex() ) ) );

    QPointF bottomLeft ( QPointF( 0, 0 ) );
    QPointF topRight;
    // If we explode, we need extra space for the pie slice that has
    // the largest explosion distance.
    if ( attrs.explode() ) {
    	const int rCount = rowCount();
        const int colCount = columnCount();
        qreal maxExplode = 0.0;
        for( int i = 0; i < rCount; ++i ){
        	qreal maxExplodeInThisRow = 0.0;
        	for( int j = 0; j < colCount; ++j ){
        		const PieAttributes columnAttrs( pieAttributes( model()->index( i, j, rootIndex() ) ) );
        		//qDebug() << columnAttrs.explodeFactor();
        		maxExplodeInThisRow = qMax( maxExplodeInThisRow, columnAttrs.explodeFactor() );
        	}
        	maxExplode += maxExplodeInThisRow;

        	// FIXME: What if explode factor of inner ring is > 1.0 ?
        	if ( !d->expandWhenExploded )
        		break;
        }
        // explode factor is relative to width (outer r - inner r) of one ring
        maxExplode /= ( rCount + 1);
        topRight = QPointF( 1.0+maxExplode, 1.0+maxExplode );
    }else{
        topRight = QPointF( 1.0, 1.0 );
    }
    return QPair<QPointF, QPointF> ( bottomLeft,  topRight );
}

void RingDiagram::paintEvent( QPaintEvent* )
{
    QPainter painter ( viewport() );
    PaintContext ctx;
    ctx.setPainter ( &painter );
    ctx.setRectangle( QRectF ( 0, 0, width(), height() ) );
    paint ( &ctx );
}

void RingDiagram::resizeEvent( QResizeEvent* )
{
}

static QRectF buildReferenceRect( const PolarCoordinatePlane* plane )
{
    QRectF contentsRect;
    QPointF referencePointAtTop = plane->translate( QPointF( 1, 0 ) );
    QPointF temp = plane->translate( QPointF( 0, 0 ) ) - referencePointAtTop;
    const double offset = temp.y();
    referencePointAtTop.setX( referencePointAtTop.x() - offset );
    contentsRect.setTopLeft( referencePointAtTop );
    contentsRect.setBottomRight( referencePointAtTop + QPointF( 2*offset, 2*offset) );
    return contentsRect;
}
/*

*/


void RingDiagram::paint( PaintContext* ctx )
{
    // note: Not having any data model assigned is no bug
    //       but we can not draw a diagram then either.
    if ( !checkInvariants(true) )
        return;

    const PieAttributes attrs( pieAttributes() );

	const int rCount = rowCount();
    const int colCount = columnCount();

    QRectF contentsRect( buildReferenceRect( polarCoordinatePlane() ) );
    contentsRect = ctx->rectangle();
    if( contentsRect.isEmpty() )
        return;

    DataValueTextInfoList list;

    d->startAngles = QVector< QVector<qreal> >( rCount, QVector<qreal>( colCount ) );
    d->angleLens = QVector< QVector<qreal> >( rCount, QVector<qreal>( colCount ) );

    // compute position
    d->size = qMin( contentsRect.width(), contentsRect.height() ); // initial size

    // if the pies explode, we need to give them additional space =>
    // make the basic size smaller
    qreal totalOffset = 0.0;
    for( int i = 0; i < rCount; ++i ){
        qreal maxOffsetInThisRow = 0.0;
    	for( int j = 0; j < colCount; ++j ){
    		const PieAttributes cellAttrs( pieAttributes( model()->index( i, j, rootIndex() ) ) );
    		//qDebug() << cellAttrs.explodeFactor();
    		const qreal explode = cellAttrs.explode() ? cellAttrs.explodeFactor() : 0.0;
    		maxOffsetInThisRow = qMax( maxOffsetInThisRow, cellAttrs.gapFactor( false ) + explode );
    	}
		if ( !d->expandWhenExploded )
			maxOffsetInThisRow -= (qreal)i;
		if ( maxOffsetInThisRow > 0.0 )
			totalOffset += maxOffsetInThisRow;

    	// FIXME: What if explode factor of inner ring is > 1.0 ?
    	//if ( !d->expandWhenExploded )
    	//	break;
    }

    // explode factor is relative to width (outer r - inner r) of one ring
    if ( rCount > 0 )
    	totalOffset /= ( rCount + 1 );
    d->size /= ( 1.0 + totalOffset );


    qreal x = ( contentsRect.width() == d->size ) ? 0.0 : ( ( contentsRect.width() - d->size ) / 2.0 );
    qreal y = ( contentsRect.height() == d->size ) ? 0.0 : ( ( contentsRect.height() - d->size ) / 2.0 );
    d->position = QRectF( x, y, d->size, d->size );
    d->position.translate( contentsRect.left(), contentsRect.top() );

    const PolarCoordinatePlane * plane = polarCoordinatePlane();

    bool atLeastOneValue = false; // guard against completely empty tables
    QVariant vValY;

    d->clearListOfAlreadyDrawnDataValueTexts();
    for ( int iRow = 0; iRow < rCount; ++iRow ) {
            const qreal sum = valueTotals( iRow );
            if( sum == 0.0 ) //nothing to draw
                continue;
            qreal currentValue = plane ? plane->startPosition() : 0.0;
            const qreal sectorsPerValue = 360.0 / sum;

            for ( int iColumn = 0; iColumn < colCount; ++iColumn ) {
    	        // is there anything at all at this column?
    	        bool bOK;
    	        const double cellValue = qAbs( model()->data( model()->index( iRow, iColumn, rootIndex() ) )
    	            .toDouble( &bOK ) );

    	        if( bOK ){
    	            d->startAngles[ iRow ][ iColumn ] = currentValue;
    	            d->angleLens[ iRow ][ iColumn ] = cellValue * sectorsPerValue;
    	            atLeastOneValue = true;
    	        } else { // mark as non-existent
    	            d->angleLens[ iRow ][ iColumn ] = 0.0;
    	            if ( iColumn > 0.0 )
    	                d->startAngles[ iRow ][ iColumn ] = d->startAngles[ iRow ][ iColumn - 1 ];
    	            else
    	                d->startAngles[ iRow ][ iColumn ] = currentValue;
    	        }
    	        //qDebug() << "d->startAngles["<<iColumn<<"] == " << d->startAngles[ iColumn ]
    	        //         << " +  d->angleLens["<<iColumn<<"]" << d->angleLens[ iColumn ]
    	        //         << " = " << d->startAngles[ iColumn ]+d->angleLens[ iColumn ];

    	        currentValue = d->startAngles[ iRow ][ iColumn ] + d->angleLens[ iRow ][ iColumn ];

    	        drawOnePie( ctx->painter(), iRow, iColumn, granularity() );
    	    }
        }
}

#if defined ( Q_WS_WIN)
#define trunc(x) ((int)(x))
#endif

/**
  Internal method that draws one of the pies in a pie chart.

  \param painter the QPainter to draw in
  \param dataset the dataset to draw the pie for
  \param pie the pie to draw
  */
void RingDiagram::drawOnePie( QPainter* painter,
        uint dataset, uint pie,
        qreal granularity )
{
    // Is there anything to draw at all?
    const qreal angleLen = d->angleLens[ dataset ][ pie ];
    if ( angleLen ) {
        const QModelIndex index( model()->index( dataset, pie, rootIndex() ) );
        const PieAttributes attrs( pieAttributes( index ) );

        drawPieSurface( painter, dataset, pie, granularity );
    }
}

void RingDiagram::resize( const QSizeF& )
{
}

/**
  Internal method that draws the surface of one of the pies in a pie chart.

  \param painter the QPainter to draw in
  \param dataset the dataset to draw the pie for
  \param pie the pie to draw
  */
void RingDiagram::drawPieSurface( QPainter* painter,
        uint dataset, uint pie,
        qreal granularity )
{
    // Is there anything to draw at all?
    qreal angleLen = d->angleLens[ dataset ][ pie ];
    if ( angleLen ) {
        qreal startAngle = d->startAngles[ dataset ][ pie ];

        QModelIndex index( model()->index( dataset, pie, rootIndex() ) );
        const PieAttributes attrs( pieAttributes( index ) );

    	const int rCount = rowCount();
    	const int colCount = columnCount();

    	int iPoint = 0;

        QRectF drawPosition = d->position;//piePosition( dataset, pie );

        painter->setRenderHint ( QPainter::Antialiasing );
        painter->setBrush( brush( index ) );
        painter->setPen( pen( index ) );
//        painter->setPen( pen );
        //painter->setPen( Qt::red );
        if ( angleLen == 360 ) {
            // full circle, avoid nasty line in the middle
            // FIXME: Draw a complete ring here
            //painter->drawEllipse( drawPosition );
        } else {
            bool perfectMatch = false;

            qreal circularGap = 0.0;

            if ( attrs.gapFactor( true ) > 0.0 )
            {
	            // FIXME: Measure in degrees!
	            circularGap = attrs.gapFactor( true );
	            //qDebug() << "gapFactor=" << attrs.gapFactor( false );
            }

            QPolygonF poly;

            qreal degree = 0;

            qreal actualStartAngle = startAngle + circularGap;
            qreal actualAngleLen = angleLen - 2 * circularGap;

            qreal totalRadialExplode = 0.0;
            qreal maxRadialExplode = 0.0;

            qreal totalRadialGap = 0.0;
            qreal maxRadialGap = 0.0;
            for( uint i = rCount - 1; i > dataset; --i ){
            	qreal maxRadialExplodeInThisRow = 0.0;
            	qreal maxRadialGapInThisRow = 0.0;
            	for( int j = 0; j < colCount; ++j ){
            		const PieAttributes cellAttrs( pieAttributes( model()->index( i, j, rootIndex() ) ) );
            		//qDebug() << cellAttrs.explodeFactor();
            		if ( d->expandWhenExploded )
            			maxRadialGapInThisRow = qMax( maxRadialGapInThisRow, cellAttrs.gapFactor( false ) );
            		if ( !cellAttrs.explode() )
            			continue;
            		// Don't use a gap for the very inner circle
                	if ( d->expandWhenExploded )
                		maxRadialExplodeInThisRow = qMax( maxRadialExplodeInThisRow, cellAttrs.explodeFactor() );
            	}
            	maxRadialExplode += maxRadialExplodeInThisRow;
            	maxRadialGap += maxRadialGapInThisRow;

            	// FIXME: What if explode factor of inner ring is > 1.0 ?
            	//if ( !d->expandWhenExploded )
            	//	break;
            }

            totalRadialGap = maxRadialGap + attrs.gapFactor( false );
            totalRadialExplode = attrs.explode() ? maxRadialExplode + attrs.explodeFactor() : maxRadialExplode;

            while ( degree <= actualAngleLen ) {
            	const QPointF p = pointOnCircle( drawPosition, dataset, pie, false, actualStartAngle + degree, totalRadialGap, totalRadialExplode );
                poly.append( p );
                degree += granularity;
                iPoint++;
            }
            if( ! perfectMatch ){
                poly.append( pointOnCircle( drawPosition, dataset, pie, false, actualStartAngle + actualAngleLen, totalRadialGap, totalRadialExplode ) );
                iPoint++;
            }

            // The center point of the inner brink
            const QPointF innerCenterPoint( poly[ int(iPoint / 2) ] );
	    
            actualStartAngle = startAngle + circularGap;
            actualAngleLen = angleLen - 2 * circularGap;

            degree = actualAngleLen;

            const int lastInnerBrinkPoint = iPoint;
            while ( degree >= 0 ){
                poly.append( pointOnCircle( drawPosition, dataset, pie, true, actualStartAngle + degree, totalRadialGap, totalRadialExplode ) );
                perfectMatch = (degree == 0);
                degree -= granularity;
                iPoint++;
            }
            // if necessary add one more point to fill the last small gap
            if( ! perfectMatch ){
                poly.append( pointOnCircle( drawPosition, dataset, pie, true, actualStartAngle, totalRadialGap, totalRadialExplode ) );
                iPoint++;
            }

            // The center point of the outer brink
            const QPointF outerCenterPoint( poly[ lastInnerBrinkPoint + int((iPoint - lastInnerBrinkPoint) / 2) ] );
            //qDebug() << poly;
            //find the value and paint it
            //fix value position
            const qreal sum = valueTotals( dataset );
            painter->drawPolygon( poly );

            const QPointF centerPoint = (innerCenterPoint + outerCenterPoint) / 2.0;

            paintDataValueText( painter, index, centerPoint, angleLen*sum / 360  );

        }
    }
}


/**
  * Auxiliary method returning a point to a given boundary
  * rectangle of the enclosed ellipse and an angle.
  */
QPointF RingDiagram::pointOnCircle( const QRectF& rect, int dataset, int pie, bool outer, qreal angle, qreal totalGapFactor, qreal totalExplodeFactor )
{
    qreal angleLen = d->angleLens[ dataset ][ pie ];
    qreal startAngle = d->startAngles[ dataset ][ pie ];
    QModelIndex index( model()->index( dataset, pie, rootIndex() ) );
    const PieAttributes attrs( pieAttributes( index ) );

	const int rCount = rowCount();

    //const qreal gapFactor = attrs.gapFactor( false );

    //qDebug() << "##" << attrs.explode();
    //if ( attrs.explodeFactor() != 0.0 )
    //	qDebug() << attrs.explodeFactor();


    qreal level = outer ? (rCount - dataset - 1) + 2 : (rCount - dataset - 1) + 1;


    //maxExplode /= rCount;

    //qDebug() << "dataset=" << dataset << "maxExplode=" << maxExplode;

    //level += maxExplode;

	const qreal offsetX = rCount > 0 ? level * rect.width() / ( ( rCount + 1 ) * 2 ) : 0.0;
	const qreal offsetY = rCount > 0 ? level * rect.height() / ( ( rCount + 1 ) * 2 ): 0.0;
	const qreal centerOffsetX = rCount > 0 ? totalExplodeFactor * rect.width() / ( ( rCount + 1 ) * 2 ) : 0.0;
	const qreal centerOffsetY = rCount > 0 ? totalExplodeFactor * rect.height() / ( ( rCount + 1 ) * 2 ): 0.0;
	const qreal gapOffsetX = rCount > 0 ? totalGapFactor * rect.width() / ( ( rCount + 1 ) * 2 ) : 0.0;
	const qreal gapOffsetY = rCount > 0 ? totalGapFactor * rect.height() / ( ( rCount + 1 ) * 2 ): 0.0;

    qreal explodeAngleRad = DEGTORAD( angle );
    qreal cosAngle = cos( explodeAngleRad );
    qreal sinAngle = -sin( explodeAngleRad );
    qreal explodeAngleCenterRad = DEGTORAD( startAngle + angleLen / 2.0 );
    qreal cosAngleCenter = cos( explodeAngleCenterRad );
    qreal sinAngleCenter = -sin( explodeAngleCenterRad );
    return QPointF( ( offsetX + gapOffsetX ) * cosAngle + centerOffsetX * cosAngleCenter + rect.center().x(),
    				( offsetY + gapOffsetY ) * sinAngle + centerOffsetY * sinAngleCenter + rect.center().y() );
}

/*virtual*/
double RingDiagram::valueTotals() const
{
	const int rCount = rowCount();
    const int colCount = columnCount();
    double total = 0.0;
    for ( int i = 0; i < rCount; ++i ) {
    	for ( int j = 0; j < colCount; ++j ) {
    		total += qAbs(model()->data( model()->index( 0, j, rootIndex() ) ).toDouble());
    		//qDebug() << model()->data( model()->index( 0, j, rootIndex() ) ).toDouble();
    	}
    }
    return total;
}

double RingDiagram::valueTotals( int dataset ) const
{
    const int colCount = columnCount();
    double total = 0.0;
    for ( int j = 0; j < colCount; ++j ) {
      total += qAbs(model()->data( model()->index( dataset, j, rootIndex() ) ).toDouble());
      //qDebug() << model()->data( model()->index( 0, j, rootIndex() ) ).toDouble();
    }
    return total;
}

/*virtual*/
double RingDiagram::numberOfValuesPerDataset() const
{
    return model() ? model()->columnCount( rootIndex() ) : 0.0;
}

double RingDiagram::numberOfDatasets() const
{
    return model() ? model()->rowCount( rootIndex() ) : 0.0;
}

/*virtual*/
double RingDiagram::numberOfGridRings() const
{
    return 1;
}
