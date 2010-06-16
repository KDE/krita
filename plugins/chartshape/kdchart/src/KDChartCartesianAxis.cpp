/****************************************************************************
 ** Copyright (C) 2007 Klarävdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartCartesianAxis.h"
#include "KDChartCartesianAxis_p.h"

#include <cmath>

#include <QtDebug>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QApplication>

#include "KDChartPaintContext.h"
#include "KDChartChart.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartAbstractGrid.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartLayoutItems.h"
#include "KDChartBarDiagram.h"
#include "KDChartStockDiagram.h"
#include "KDChartLineDiagram.h"
#include "KDChartPrintingParameters.h"

#include <KDABLibFakes>

#include <limits>

using namespace KDChart;

#define d (d_func())

CartesianAxis::CartesianAxis ( AbstractCartesianDiagram* diagram )
    : AbstractAxis ( new Private( diagram, this ), diagram )
{
    init();
}

CartesianAxis::~CartesianAxis ()
{
    // when we remove the first axis it will unregister itself and
    // propagate the next one to the primary, thus the while loop
    while ( d->mDiagram ) {
        AbstractCartesianDiagram *cd = qobject_cast<AbstractCartesianDiagram*>( d->mDiagram );
        cd->takeAxis( this );
    }
    Q_FOREACH( AbstractDiagram *diagram, d->secondaryDiagrams ) {
        AbstractCartesianDiagram *cd = qobject_cast<AbstractCartesianDiagram*>( diagram );
        cd->takeAxis( this );
    }
}

void CartesianAxis::init ()
{
    d->position = Bottom;
    setCachedSizeDirty();
}


bool CartesianAxis::compare( const CartesianAxis* other )const
{
    if( other == this ) return true;
    if( ! other ){
        //qDebug() << "CartesianAxis::compare() cannot compare to Null pointer";
        return false;
    }
    /*
    qDebug() << (position()            == other->position());
    qDebug() << (titleText()           == other->titleText());
    qDebug() << (titleTextAttributes() == other->titleTextAttributes());
    */
    return  ( static_cast<const AbstractAxis*>(this)->compare( other ) ) &&
            ( position()            == other->position() ) &&
            ( titleText()           == other->titleText() ) &&
            ( titleTextAttributes() == other->titleTextAttributes() );
}


void CartesianAxis::setTitleText( const QString& text )
{
    d->titleText = text;
    layoutPlanes();
}

QString CartesianAxis::titleText() const
{
    return d->titleText;
}

void CartesianAxis::setTitleTextAttributes( const TextAttributes &a )
{
    d->titleTextAttributes = a;
    d->useDefaultTextAttributes = false;
    layoutPlanes();
}

TextAttributes CartesianAxis::titleTextAttributes() const
{
    if( hasDefaultTitleTextAttributes() ){
        TextAttributes ta( textAttributes() );
        Measure me( ta.fontSize() );
        me.setValue( me.value() * 1.5 );
        ta.setFontSize( me );
        return ta;
    }
    return d->titleTextAttributes;
}

void CartesianAxis::resetTitleTextAttributes()
{
    d->useDefaultTextAttributes = true;
    layoutPlanes();
}

bool CartesianAxis::hasDefaultTitleTextAttributes() const
{
    return d->useDefaultTextAttributes;
}


void CartesianAxis::setPosition ( Position p )
{
    d->position = p;
    layoutPlanes();
}

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
const
#endif
CartesianAxis::Position CartesianAxis::position() const
{
    return d->position;
}

void CartesianAxis::layoutPlanes()
{
    //qDebug() << "CartesianAxis::layoutPlanes()";
    if( ! d->diagram() || ! d->diagram()->coordinatePlane() ) {
        //qDebug() << "CartesianAxis::layoutPlanes(): Sorry, found no plane.";
        return;
    }
    AbstractCoordinatePlane* plane = d->diagram()->coordinatePlane();
    if( plane ){
        plane->layoutPlanes();
        //qDebug() << "CartesianAxis::layoutPlanes() OK";
    }
}

/*
  void CartesianAxis::paintEvent( QPaintEvent* event )
  {
  Q_UNUSED( event );

  if( ! d->diagram() || ! d->diagram()->coordinatePlane() ) return;

  PaintContext context;
  QPainter painter( this );
  context.setPainter( &painter );
  AbstractCoordinatePlane* plane = d->diagram()->coordinatePlane();
  context.setCoordinatePlane( plane );
  QRectF rect = QRectF ( 1, 1, plane->width() - 3, plane->height() - 3 );
  context.setRectangle( rect );
  d->geometry.setSize( size() );
  paintCtx( &context );
  }
*/


static bool referenceDiagramIsBarDiagram( const AbstractDiagram * diagram )
{
    const AbstractCartesianDiagram * dia =
            qobject_cast< const AbstractCartesianDiagram * >( diagram );
    if( dia && dia->referenceDiagram() )
        dia = dia->referenceDiagram();
    return qobject_cast< const BarDiagram* >( dia ) != 0;
}

static bool referenceDiagramNeedsCenteredAbscissaTicks( const AbstractDiagram *diagram )
{
    const AbstractCartesianDiagram * dia =
            qobject_cast< const AbstractCartesianDiagram * >( diagram );
    if( dia && dia->referenceDiagram() )
        dia = dia->referenceDiagram();
    if ( qobject_cast< const BarDiagram* >( dia ) )
    	return true;
    if ( qobject_cast< const StockDiagram* >( dia ) )
    	return true;

    const LineDiagram * lineDiagram = qobject_cast< const LineDiagram* >( dia );
    return lineDiagram && lineDiagram->centerDataPoints();
}

bool CartesianAxis::isAbscissa() const
{
	const Qt::Orientation diagramOrientation = referenceDiagramIsBarDiagram( d->diagram() ) ? ((BarDiagram*)(d->diagram()))->orientation()
																						    : Qt::Vertical;
	return diagramOrientation == Qt::Vertical ? position() == Bottom || position() == Top
									          : position() == Left   || position() == Right;
}

bool CartesianAxis::isOrdinate() const
{
	const Qt::Orientation diagramOrientation = referenceDiagramIsBarDiagram( d->diagram() ) ? ((BarDiagram*)(d->diagram()))->orientation()
																						    : Qt::Vertical;
	return diagramOrientation == Qt::Vertical ? position() == Left   || position() == Right
									          : position() == Bottom || position() == Top;
}

void CartesianAxis::paint( QPainter* painter )
{
    if( ! d->diagram() || ! d->diagram()->coordinatePlane() ) return;
    PaintContext ctx;
    ctx.setPainter ( painter );
    ctx.setCoordinatePlane( d->diagram()->coordinatePlane() );
    const QRect rect( areaGeometry() );

    //qDebug() << "CartesianAxis::paint( QPainter* painter )  " << " areaGeometry()():" << rect << " sizeHint():" << sizeHint();

    ctx.setRectangle(
        QRectF (
            //QPointF(0, 0),
            QPointF(rect.left(), rect.top()),
            QSizeF(rect.width(), rect.height() ) ) );
    // enabling clipping so that we're not drawing outside
    QRegion clipRegion( rect.adjusted( -1, -1, 1, 1 ) );
    painter->save();
    painter->setClipRegion( clipRegion );
    paintCtx( &ctx );
    painter->restore();
    //qDebug() << "KDChart::CartesianAxis::paint() done.";
}

void CartesianAxis::Private::drawSubUnitRulers( QPainter* painter, CartesianCoordinatePlane* plane, const DataDimension& dim,
                                                const QPointF& rulerRef, const QVector<int>& drawnTicks, const bool diagramIsVertical,
						const RulerAttributes& rulerAttr ) const
{
    const QRect geoRect( axis()->geometry() );
    int nextMayBeTick = 0;
    int mayBeTick = 0;
    int logSubstep = 0;
    qreal f = dim.start;
    qreal fLogSubstep = f;
    const bool isAbscissa = axis()->isAbscissa();
    const bool isLogarithmic = (dim.calcMode == AbstractCoordinatePlane::Logarithmic );
    const int subUnitTickLength = axis()->tickLength( true );

    // Use negative limit to ensure that also the last tick is painted,
    // which is needed if major tick marks are disabled
    while ( dim.end - f > -std::numeric_limits< float >::epsilon() ) {
    	const qreal quotient = f / dim.stepWidth;
    	const bool isMinorTickMark = qAbs(qRound(quotient) - quotient) > std::numeric_limits< float >::epsilon();
    	// 'Drawn' ticks isn't quite the right naming here, it also counts major tick marks, which are not drawn.
        if( drawnTicks.count() > nextMayBeTick )
            mayBeTick = drawnTicks[ nextMayBeTick ];
        // Paint minor tick mark only if there is no major tick mark drawn at this point
        if ( isMinorTickMark || !rulerAttr.showMajorTickMarks() ) {
	        if ( isAbscissa ) {
	            // for the x-axis
	            QPointF topPoint = diagramIsVertical ? QPointF( f, 0 ) : QPointF( 0, f );
	            QPointF bottomPoint( topPoint );
	            // we don't draw the sub ticks, if we are at the same position as a normal tick
	            topPoint = plane->translate( topPoint );
	            bottomPoint = plane->translate( bottomPoint );
	            if ( diagramIsVertical ) {
		            topPoint.setY( rulerRef.y() + subUnitTickLength );
		            bottomPoint.setY( rulerRef.y() );
	            } else {
		            topPoint.setX( rulerRef.x() + subUnitTickLength );
		            bottomPoint.setX( rulerRef.x() );
	            }
	            if( qAbs( mayBeTick - topPoint.x() ) > 1 )
	            {
                    if ( rulerAttr.hasTickMarkPenAt( topPoint.x() ) )
                    	painter->setPen( rulerAttr.tickMarkPen( topPoint.x() ) );
                    else
                    	painter->setPen( rulerAttr.minorTickMarkPen() );
	                painter->drawLine( topPoint, bottomPoint );
	            }
	            else {
	                ++nextMayBeTick;
	            }
	        } else {
	            // for the y-axis

	            QPointF leftPoint = plane->translate( diagramIsVertical ? QPointF( 0, f ) : QPointF( f, 0 ) );
	            //qDebug() << "geoRect:" << geoRect << "   geoRect.top()" << geoRect.top() << "geoRect.bottom()" << geoRect.bottom() << "  translatedValue:" << translatedValue;
	            // we don't draw the sub ticks, if we are at the same position as a normal tick
	            if( qAbs( mayBeTick - diagramIsVertical ? leftPoint.y() : leftPoint.x() ) > 1 ){
	                const qreal translatedValue = leftPoint.y();
	                bool translatedValueIsWithinBoundaries;
	                if ( diagramIsVertical ) {
	                	translatedValueIsWithinBoundaries = translatedValue > geoRect.top() && translatedValue <= geoRect.bottom();
	                } else {
	                	translatedValueIsWithinBoundaries = translatedValue > geoRect.left() && translatedValue <= geoRect.right();
	                }
	                if( translatedValueIsWithinBoundaries ){
	                    QPointF rightPoint = diagramIsVertical ? QPointF( 0, f ) : QPointF( f, 0 );
	                    rightPoint = plane->translate( rightPoint );
	                    if ( diagramIsVertical ) {
		                    leftPoint.setX( rulerRef.x() + subUnitTickLength );
		                    rightPoint.setX( rulerRef.x() );
	                    } else {
		                    leftPoint.setY( rulerRef.y() + (position == Bottom ? subUnitTickLength : -subUnitTickLength) );
		                    rightPoint.setY( rulerRef.y() );
	                    }
	                    if ( rulerAttr.hasTickMarkPenAt( f ) )
	                    	painter->setPen( rulerAttr.tickMarkPen( f ) );
	                    else
	                    	painter->setPen( rulerAttr.minorTickMarkPen() );
	                    painter->drawLine( leftPoint, rightPoint );
	                }
	            } else {
	                ++nextMayBeTick;
	            }
                }
    	}
        if ( isLogarithmic ){
            if( logSubstep == 9 ){
                fLogSubstep *= ( fLogSubstep > 0.0 ) ? 10.0 : 0.1;
                if( fLogSubstep == 0 )
                    fLogSubstep = 0.01;
                logSubstep = 0;
                f = fLogSubstep;
            }
            else
            {
                f += fLogSubstep;
            }
            ++logSubstep;
        }else{
            f += dim.subStepWidth;
        }
    }
}


const TextAttributes CartesianAxis::Private::titleTextAttributesWithAdjustedRotation() const
{
    TextAttributes titleTA( titleTextAttributes );
    if( (position == Left || position == Right) ){
        int rotation = titleTA.rotation() + 270;
        if( rotation >= 360 )
            rotation -= 360;

        // limit the allowed values to 0, 90, 180, 270:
        if( rotation  < 90 )
            rotation = 0;
        else if( rotation  < 180 )
            rotation = 90;
        else if( rotation  < 270 )
            rotation = 180;
        else if( rotation  < 360 )
            rotation = 270;
        else
            rotation = 0;

        titleTA.setRotation( rotation );
    }
    return titleTA;
}


void CartesianAxis::Private::drawTitleText( QPainter* painter, CartesianCoordinatePlane* plane, const QRect& areaGeoRect ) const
{
    const TextAttributes titleTA( titleTextAttributesWithAdjustedRotation() );
    if( titleTA.isVisible() ) {
        TextLayoutItem titleItem( titleText,
                                  titleTA,
                                  plane->parent(),
                                  KDChartEnums::MeasureOrientationMinimum,
                                  Qt::AlignHCenter|Qt::AlignVCenter );
        QPointF point;
        QSize size( titleItem.sizeHint() );
        //FIXME(khz): We definitely need to provide a way that users can decide
        //            the position of an axis title.
        switch( position )
        {
        case Top:
            point.setX( areaGeoRect.left() + areaGeoRect.width() / 2.0);
            point.setY( areaGeoRect.top()  + size.height() / 2 );
            size.setWidth( qMin( size.width(), axis()->geometry().width() ) );
            break;
        case Bottom:
            point.setX( areaGeoRect.left() + areaGeoRect.width() / 2.0);
            point.setY( areaGeoRect.bottom() - size.height() / 2 );
            size.setWidth( qMin( size.width(), axis()->geometry().width() ) );
            break;
        case Left:
            point.setX( areaGeoRect.left() + size.width() / 2 );
            point.setY( areaGeoRect.top() + areaGeoRect.height() / 2.0);
            size.setHeight( qMin( size.height(), axis()->geometry().height() ) );
            break;
        case Right:
            point.setX( areaGeoRect.right() - size.width() / 2 );
            point.setY( areaGeoRect.top() + areaGeoRect.height() / 2.0);
            size.setHeight( qMin( size.height(), axis()->geometry().height() ) );
            break;
        }
        const PainterSaver painterSaver( painter );
        painter->translate( point );
        //if( axis()->isOrdinate() )
        //    painter->rotate( 270.0 );
        titleItem.setGeometry( QRect( QPoint(-size.width() / 2, -size.height() / 2), size ) );
        //painter->drawRect(titleItem.geometry().adjusted(0,0,-1,-1));
        titleItem.paint( painter );
    }
}


static void calculateNextLabel( qreal& labelValue, qreal step, bool isLogarithmic, qreal min )
{
    if ( isLogarithmic ){
        if( step > 0.0 )
            labelValue *= 10.0;
        else
            labelValue /= 10.0;
        if( labelValue == 0.0 )
            labelValue = pow( 10.0, floor( log10( min ) ) );
    }else{
        //qDebug() << "new axis label:" << labelValue << "+" << step << "=" << labelValue+step;
        labelValue += step;
        if( qAbs(labelValue) < 1.0e-15 )
            labelValue = 0.0;
    }
}


void CartesianAxis::paintCtx( PaintContext* context )
{

    Q_ASSERT_X ( d->diagram(), "CartesianAxis::paint",
                 "Function call not allowed: The axis is not assigned to any diagram." );

    CartesianCoordinatePlane* plane = dynamic_cast<CartesianCoordinatePlane*>(context->coordinatePlane());
    Q_ASSERT_X ( plane, "CartesianAxis::paint",
                 "Bad function call: PaintContext::coodinatePlane() NOT a cartesian plane." );

    // note: Not having any data model assigned is no bug
    //       but we can not draw an axis then either.
    if( ! d->diagram()->model() )
        return;

    // Determine the diagram that specifies the orientation of the diagram we're painting here
    // That diagram is the reference diagram, if it exists, or otherwise the diagram itself.
    // Note: In KDChart 2.3 or earlier, only a bar diagram can be vertical instead of horizontal.
    const AbstractCartesianDiagram * refDiagram = qobject_cast< const AbstractCartesianDiagram * >( d->diagram() );
    if( refDiagram && refDiagram->referenceDiagram() )
        refDiagram = refDiagram->referenceDiagram();
    const BarDiagram *barDiagram = qobject_cast< const BarDiagram* >( refDiagram );
    const Qt::Orientation diagramOrientation = barDiagram ? barDiagram->orientation() : Qt::Vertical;
    const bool diagramIsVertical = diagramOrientation == Qt::Vertical;

    /*
     * let us paint the labels at a
     * smaller resolution
     * Same mini pixel value as for
     * Cartesian Grid
     */
    //const qreal MinimumPixelsBetweenRulers = 1.0;
    DataDimensionsList dimensions( plane->gridDimensionsList() );
    //qDebug("CartesianAxis::paintCtx() gets DataDimensionsList.first():   start: %f   end: %f   stepWidth: %f", dimensions.first().start, dimensions.first().end, dimensions.first().stepWidth);

    // test for programming errors: critical
    Q_ASSERT_X ( dimensions.count() == 2, "CartesianAxis::paint",
                 "Error: plane->gridDimensionsList() did not return exactly two dimensions." );
    DataDimension dimX, dimY;
    DataDimension dim;
	// If the diagram is horizontal, we need to inverse the x/y ranges
    if ( diagramIsVertical ) {
    	/*double yStart = dimY.start;
    	double yEnd = dimY.end;
    	dimY.start = dimX.start;
    	dimY.end = dimX.end;
    	dimX.start = yStart;
    	dimX.end = yEnd;*/
    	dimX = AbstractGrid::adjustedLowerUpperRange( dimensions.first(), true, true );
    	dimY = AbstractGrid::adjustedLowerUpperRange( dimensions.last(), true, true );

    	// FIXME
    	// Ugly workaround for dimensions being bound to both, the x coordinate direction and the abscissa
    	//if ( referenceDiagramIsPercentLyingBarDiagram ) {
    	//	dimY.stepWidth = 10.0;
    	//	dimY.subStepWidth = 2.0;
    	//}
    } else {
    	dimX = AbstractGrid::adjustedLowerUpperRange( dimensions.last(), true, true );
    	dimY = AbstractGrid::adjustedLowerUpperRange( dimensions.first(), true, true );
    }
	dim = (isAbscissa() ? dimX : dimY);

    /*
    if(isAbscissa())
        qDebug() << "         " << "Abscissa:" << dimX.start <<".."<<dimX.end <<"  step"<<dimX.stepWidth<<"  sub step"<<dimX.subStepWidth;
    else
        qDebug() << "         " << "Ordinate:" << dimY.start <<".."<<dimY.end <<"  step"<<dimY.stepWidth<<"  sub step"<<dimY.subStepWidth;
    */



    /*
     * let us paint the labels at a
     * smaller resolution
     * Same mini pixel value as for
     * Cartesian Grid
     */
    const qreal MinimumPixelsBetweenRulers = qMin(  dimX.stepWidth,  dimY.stepWidth );//1.0;

    // preparations:
    // - calculate the range that will be displayed:
    const qreal absRange = qAbs( dim.distance() );

    qreal numberOfUnitRulers;
    if ( isAbscissa() ) {
        if( dimX.isCalculated )
            numberOfUnitRulers = absRange / qAbs( dimX.stepWidth ) + 1.0;
        else
            numberOfUnitRulers = d->diagram()->model()->rowCount(d->diagram()->rootIndex()) - 1.0;
    }else{
        numberOfUnitRulers = absRange / qAbs( dimY.stepWidth ) + 1.0;
    }

    //    qDebug() << "absRange" << absRange << "dimY.stepWidth:" << dimY.stepWidth << "numberOfUnitRulers:" << numberOfUnitRulers;

    qreal numberOfSubUnitRulers;
    if ( isAbscissa() ){
        if( dimX.isCalculated )
            numberOfSubUnitRulers = absRange / qAbs( dimX.subStepWidth ) + 1.0;
        else
            numberOfSubUnitRulers = dimX.subStepWidth>0 ? absRange / qAbs( dimX.subStepWidth ) + 1.0 : 0.0;
    }else{
        numberOfSubUnitRulers = absRange / qAbs( dimY.subStepWidth ) + 1.0;
    }

    // - calculate the absolute range in screen pixels:
    const QPointF p1 = plane->translate( diagramIsVertical ? QPointF(dimX.start, dimY.start) : QPointF(dimY.start, dimX.start) );
    const QPointF p2 = plane->translate( diagramIsVertical ? QPointF(dimX.end,   dimY.end)   : QPointF(dimY.end,   dimX.end  ) );

    double screenRange;
    if ( isAbscissa() )
    {
        screenRange = qAbs ( p1.x() - p2.x() );
    } else {
        screenRange = qAbs ( p1.y() - p2.y() );
    }

    const bool useItemCountLabels = isAbscissa() && ! dimX.isCalculated;

    // attributes used to customize ruler appearance
    const RulerAttributes rulerAttr = rulerAttributes();

    const bool drawUnitRulers = rulerAttr.showMajorTickMarks() && (screenRange / ( numberOfUnitRulers / dimX.stepWidth ) > MinimumPixelsBetweenRulers);
    const bool drawSubUnitRulers = rulerAttr.showMinorTickMarks() &&
        (numberOfSubUnitRulers != 0.0) &&
        (screenRange / numberOfSubUnitRulers > MinimumPixelsBetweenRulers);

    const TextAttributes labelTA = textAttributes();
    const bool drawLabels = labelTA.isVisible();

    // - find the reference point at which to start drawing and the increment (line distance);
    QPointF rulerRef;
    const QRect areaGeoRect( areaGeometry() );
    const QRect geoRect( geometry() );
    QRectF rulerRect;
    double rulerWidth;
    double rulerHeight;

    QPainter* const ptr = context->painter();

    //for debugging: if( isAbscissa() )ptr->drawRect(areaGeoRect.adjusted(0,0,-1,-1));
    //qDebug() << "         " << (isAbscissa() ? "Abscissa":"Ordinate") << "axis painting with geometry" << areaGeoRect;

    // FIXME references are of course different for all locations:
    rulerWidth = areaGeoRect.width();
    rulerHeight =  areaGeoRect.height();
    switch( position() )
    {
    case Top:
        rulerRef.setX( areaGeoRect.topLeft().x() );
        rulerRef.setY( areaGeoRect.topLeft().y() + rulerHeight );
        break;
    case Bottom:
        rulerRef.setX( areaGeoRect.bottomLeft().x() );
        rulerRef.setY( areaGeoRect.bottomLeft().y() - rulerHeight );
        break;
    case Right:
        rulerRef.setX( areaGeoRect.bottomRight().x() - rulerWidth );
        rulerRef.setY( areaGeoRect.bottomRight().y() );
        break;
    case Left:
        rulerRef.setX( areaGeoRect.bottomLeft().x() + rulerWidth );
        rulerRef.setY( areaGeoRect.bottomLeft().y() );
        break;
    }

    // set up the lines to paint:

    // set up a map of integer positions,

    // - starting with the fourth
    // - the the halfs
    // - then the tens
    // this will override all halfs and fourth that hit a higher-order ruler
    // MAKE SURE TO START AT (0, 0)!

    // set up a reference point,  a step vector and a unit vector for the drawing:

    const qreal minValueY = dimY.start;
    const qreal maxValueY = dimY.end;
    const qreal minValueX = dimX.start;
    const qreal maxValueX = dimX.end;
    const bool isLogarithmicX = (dimX.calcMode == AbstractCoordinatePlane::Logarithmic );
    const bool isLogarithmicY = (dimY.calcMode == AbstractCoordinatePlane::Logarithmic );
//#define AXES_PAINTING_DEBUG 1
#ifdef AXES_PAINTING_DEBUG
    qDebug() << "CartesianAxis::paint: reference values:" << endl
             << "-- range x/y: " << dimX.distance() << "/" << dimY.distance() << endl
             << "-- absRange: " << absRange << endl
             << "-- numberOfUnitRulers: " << numberOfUnitRulers << endl
             << "-- screenRange: " << screenRange << endl
             << "-- drawUnitRulers: " << drawUnitRulers << endl
             << "-- drawLabels: " << drawLabels << endl
             << "-- ruler reference point:: " << rulerRef << endl
             << "-- minValueX: " << minValueX << "   maxValueX: " << maxValueX << endl
             << "-- minValueY: " << minValueY << "   maxValueY: " << maxValueY << endl
        ;
#endif

    // solving issue #4075 in a quick way:
    ptr->setPen ( PrintingParameters::scalePen( labelTA.pen() ) ); // perhaps we want to add a setter method later?

    //ptr->setPen ( Qt::black );

    const QObject* referenceArea = plane->parent();

    // that QVector contains all drawn x-ticks (so no subticks are drawn there also)
    QVector< int > drawnAbscissaTicks;
    // and that does the same for the y-ticks
    QVector< int > drawnYTicks;

    /*
     * Find out if it is a bar diagram
     * bar diagrams display their data per column
     * we need to handle the last label another way
     * 1 - Last label == QString null ( Header Labels )
     * 2 - Display labels and ticks in the middle of the column
     */

    const bool centerAbscissaTicks = referenceDiagramNeedsCenteredAbscissaTicks( d->diagram() );

    // this draws the unit rulers
    if ( drawUnitRulers ) {
        const QStringList labelsList(      labels() );
        const QStringList shortLabelsList( shortLabels() );
        const int hardLabelsCount  = labelsList.count();
        const int shortLabelsCount = shortLabelsList.count();
        bool useShortLabels = false;


        bool useConfiguredStepsLabels = false;
        QStringList headerLabels;
        if( useItemCountLabels ){
            //qDebug() << (isOrdinate() ? "is Ordinate" : "is Abscissa");
            headerLabels =
                isOrdinate()
                ? d->diagram()->datasetLabels()
                : d->diagram()->itemRowLabels();
            //qDebug() << numberOfUnitRulers;
            // check if configured stepWidth
            useConfiguredStepsLabels = isAbscissa() &&
                    dimX.stepWidth &&
                    (( (headerLabels.count() - 1)/ dimX.stepWidth ) != numberOfUnitRulers);
            if( useConfiguredStepsLabels ) {
                numberOfUnitRulers = ( headerLabels.count() - 1 )/ dimX.stepWidth;
                // we need to register data values for the steps
                // in case it is configured by the user
                QStringList configuredStepsLabels;
                double value = dimX.start;// headerLabels.isEmpty() ? 0.0 : headerLabels.first().toDouble();
                configuredStepsLabels << QString::number( value );

                for( int i = 0; i < numberOfUnitRulers; i++ )
                {
                    //qDebug() << value;
                    value += dimX.stepWidth;
                    configuredStepsLabels.append( d->diagram()->unitPrefix( i, diagramIsVertical ? Qt::Horizontal : Qt::Vertical, true ) +
                                                  QString::number( value ) +
                                                  d->diagram()->unitSuffix( i, diagramIsVertical ? Qt::Horizontal : Qt::Vertical, true ) );
                }
                headerLabels = configuredStepsLabels;
            }

            if (  centerAbscissaTicks )
                headerLabels.append( QString::null );
        }


        const int headerLabelsCount = headerLabels.count();
        //qDebug() << "headerLabelsCount" << headerLabelsCount;

        TextLayoutItem* labelItem =
            drawLabels
            ? new TextLayoutItem( QString::number( minValueY ),
                                  labelTA,
                                  referenceArea,
                                  KDChartEnums::MeasureOrientationMinimum,
                                  Qt::AlignLeft )
            : 0;
        TextLayoutItem* labelItem2 =
            drawLabels
            ? new TextLayoutItem( QString::number( minValueY ),
                                  labelTA,
                                  referenceArea,
                                  KDChartEnums::MeasureOrientationMinimum,
                                  Qt::AlignLeft )
            : 0;
        const QFontMetricsF met(
            drawLabels
            ? labelItem->realFont()
            : QFontMetricsF( QApplication::font(), GlobalMeasureScaling::paintDevice() ) );
        const qreal halfFontHeight = met.height() * 0.5;
        const qreal halfFontWidth = met.averageCharWidth() * 0.5;

        if ( isAbscissa() ) {

            if( !d->annotations.isEmpty() )
            {
                const QList< double > values = d->annotations.keys();
                KDAB_FOREACH( const double v, values )
                {
                   QPointF topPoint = diagramIsVertical ? QPointF( v, 0.0 ) : QPointF( 0.0, v );
                   QPointF bottomPoint = topPoint;
                   topPoint = plane->translate( topPoint );
                   bottomPoint = plane->translate( bottomPoint );
                   if ( diagramIsVertical ) {
                	   topPoint.setY( rulerRef.y() + tickLength() );
                	   bottomPoint.setY( rulerRef.y() );
                   } else {
                	   topPoint.setX( rulerRef.x() + tickLength() );
                	   bottomPoint.setX( rulerRef.x() );
                   }

                   labelItem->setText( d->annotations[ v ] );
                   const QSize size( labelItem->sizeHint() );
                   if ( diagramIsVertical ) {
					labelItem->setGeometry(
					    QRect(
					        QPoint(
					            static_cast<int>( topPoint.x() - size.width() / 2.0 ),
					            static_cast<int>( topPoint.y() +
					                            ( position() == Bottom
					                                ? halfFontHeight
					                                : ((halfFontHeight + size.height()) * -1.0) ) ) ),
					        size ) );
                   } else {
                    labelItem->setGeometry(
                        QRect(
                            QPoint(
                                static_cast<int>( bottomPoint.x() +
                                                ( position() == Right
                                                    ? halfFontWidth
                                                    : (-halfFontWidth - size.width()) ) ),

        	                    static_cast<int>( topPoint.y() - ( size.height() ) * 0.5 ) ),
                            size ) );
                   }

                   QRect labelGeo = labelItem->geometry();
                   // if our item would only half fit, we disable clipping for that one
                   if( labelGeo.left() < geoRect.left() && labelGeo.right() > geoRect.left() )
                       ptr->setClipping( false );
                   else if( labelGeo.left() < geoRect.right() && labelGeo.right() > geoRect.right() )
                       ptr->setClipping( false );

                   labelItem->paint( ptr );
                }
            }

            qreal labelDiff = dimX.stepWidth;
            const int precision = ( QString::number( labelDiff ).section( QLatin1Char('.'), 1,  2 ) ).length();

            // If we have a labels list AND a short labels list, we first find out,
            // if there is enough space for showing ALL of the long labels:
            // If not, use the short labels.
            if( drawLabels && hardLabelsCount > 0 && shortLabelsCount > 0 && d->annotations.isEmpty() ){
                bool labelsAreOverlapping = false;
                int iLabel = 0;
                qreal i = minValueX;
                while ( i < maxValueX-1 && !labelsAreOverlapping )
                {
                    if ( dimX.stepWidth != 1.0 && ! dim.isCalculated )
                    {
                        // Check intersects for the header label - we need to pass the full string
                        // here and not only the i value.
                        if( useConfiguredStepsLabels ){
                            labelItem->setText( customizedLabel(headerLabels[ iLabel   ]) );
                            labelItem2->setText(customizedLabel(headerLabels[ iLabel+1 ]) );
                        }else{
                            //qDebug() << "i + labelDiff " << i + labelDiff;
                            labelItem->setText( customizedLabel(headerLabelsCount > i && i >= 0 ?
                                    headerLabels[static_cast<int>(i)] :
                                    QString::number( i, 'f', precision )) );
                            //           qDebug() << "1 - labelItem->text() " << labelItem->text();
                            //qDebug() << "labelDiff" << labelDiff
                            //        << "  index" << i+labelDiff << "  count" << headerLabelsCount;
                            labelItem2->setText( customizedLabel(headerLabelsCount > i + labelDiff && i + labelDiff >= 0 ?
                                    headerLabels[static_cast<int>(i+labelDiff)] :
                                    QString::number( i + labelDiff, 'f', precision )) );
                            //qDebug() << "2 - labelItem->text() " << labelItem->text();
                            //qDebug() << "labelItem2->text() " << labelItem2->text();
                        }
                    } else {
                        //qDebug() << iLabel << i << "("<<hardLabelsCount<<")   :";
                        const int idx = (iLabel < hardLabelsCount    ) ? iLabel     : 0;
                        const int idx2= (iLabel < hardLabelsCount - 1) ? iLabel + 1 : 0;
                        const int shortIdx =  (iLabel < shortLabelsCount    ) ? iLabel     : 0;
                        const int shortIdx2 = (iLabel < shortLabelsCount - 1) ? iLabel + 1 : 0;
                        labelItem->setText(  customizedLabel(
                                useShortLabels ? shortLabelsList[ shortIdx ] : labelsList[ idx ] ) );
                        labelItem2->setText( customizedLabel(
                                useShortLabels ? shortLabelsList[ shortIdx2 ] : labelsList[ idx2 ] ) );
                    }

                    QPointF firstPos = diagramIsVertical ? QPointF( i, 0.0 ) : QPointF( 0.0, i );
                    firstPos = plane->translate( firstPos );

                    QPointF secondPos = diagramIsVertical ? QPointF( i + labelDiff, 0.0 ) : QPointF( 0.0, i + labelDiff );
                    secondPos = plane->translate( secondPos );

                    labelsAreOverlapping = labelItem->intersects( *labelItem2, firstPos, secondPos );


                    //qDebug() << labelsAreOverlapping;
                    if ( ++iLabel > hardLabelsCount - 1 )
                        iLabel = 0;
                    if ( isLogarithmicX )
                        i *= 10.0;
                    else
                        i += dimX.stepWidth;

                    //qDebug() << iLabel << i << labelsAreOverlapping << firstPos << secondPos.x()-firstPos .x() << labelItem->text() << labelItem2->text();
                }
                //qDebug() << "-----------------------";

                useShortLabels = labelsAreOverlapping;
            }

            //      qDebug() << "initial labelDiff " << labelDiff;
            if ( drawLabels && d->annotations.isEmpty() )
            {
                qreal i = minValueX;
                int iLabel = 0;

                while ( i + labelDiff < maxValueX )
                {
                    //qDebug() << "drawLabels" << drawLabels << "  hardLabelsCount" << hardLabelsCount
                    //        << "  dimX.stepWidth" << dimX.stepWidth << "  dim.isCalculated" << dim.isCalculated;
                    if ( !drawLabels || hardLabelsCount < 1 || ( dimX.stepWidth != 1.0 && ! dim.isCalculated ) )
                    {
                        // Check intersects for the header label - we need to pass the full string
                        // here and not only the i value.
                        if( useConfiguredStepsLabels ){
                            labelItem->setText( customizedLabel(headerLabels[ iLabel   ]) );
                            labelItem2->setText(customizedLabel(headerLabels[ iLabel+1 ]) );
                        }else{
                            //qDebug() << "i + labelDiff " << i + labelDiff;
                            labelItem->setText( customizedLabel(headerLabelsCount > i && i >= 0 ?
                                    headerLabels[static_cast<int>(i)] :
                                    QString::number( i, 'f', precision )) );
                            //           qDebug() << "1 - labelItem->text() " << labelItem->text();
                            //qDebug() << "labelDiff" << labelDiff
                            //        << "  index" << i+labelDiff << "  count" << headerLabelsCount;
                            labelItem2->setText( customizedLabel(headerLabelsCount > i + labelDiff && i + labelDiff >= 0 ?
                                    headerLabels[static_cast<int>(i+labelDiff)] :
                                    QString::number( i + labelDiff, 'f', precision )) );
                            //qDebug() << "2 - labelItem->text() " << labelItem->text();
                            //qDebug() << "labelItem2->text() " << labelItem2->text();
                        }
                    } else {
                        const int idx = (iLabel < hardLabelsCount    ) ? iLabel     : 0;
                        const int idx2= (iLabel < hardLabelsCount - 1) ? iLabel + 1 : 0;
                        const int shortIdx =  (iLabel < shortLabelsCount    ) ? iLabel     : 0;
                        const int shortIdx2 = (iLabel < shortLabelsCount - 1) ? iLabel + 1 : 0;
                        labelItem->setText(  customizedLabel(
                                useShortLabels ? shortLabelsList[ shortIdx ] : labelsList[ idx ] ) );
                        labelItem2->setText( customizedLabel(
                                useShortLabels ? shortLabelsList[ shortIdx2 ] : labelsList[ idx2 ] ) );
                    }

                    QPointF firstPos = diagramIsVertical ? QPointF( i, 0.0 ) : QPointF( 0.0, i );
                    firstPos = plane->translate( firstPos );

                    QPointF secondPos = diagramIsVertical ? QPointF( i + labelDiff, 0.0 ) : QPointF( 0.0, i + labelDiff );
                    secondPos = plane->translate( secondPos );


                    if ( labelItem->intersects( *labelItem2, firstPos, secondPos ) )
                    {
                        i = minValueX;

                        // fix for issue #4179:
                        labelDiff *= 10.0;
                        // old code:
                        // labelDiff += labelDiff;

                        iLabel = 0;
                        //qDebug() << firstPos << secondPos.x()-firstPos .x() << labelItem->text() << labelItem2->text();
                        //qDebug() << labelDiff;
                    }
                    else
                    {
                        i += labelDiff;
                        //qDebug() << firstPos << secondPos.x()-firstPos .x() << labelItem->text() << labelItem2->text();
                        //qDebug() << "ok";
                    }

                    if ( (++iLabel > hardLabelsCount - 1) && !useConfiguredStepsLabels )
                    {
                        iLabel = 0;
                    }
                }
                // fixing bugz issue #5018 without breaking issue #4179:
                if( minValueX + labelDiff > maxValueX )
                    labelDiff = maxValueX - minValueX;
                // This makes sure the first and the last X label are drawn
                // if there is not enouth place to draw some more of them
                // according to labelDiff calculation performed above.
            }

            int idxLabel = 0;
            qreal iLabelF = minValueX;
            //qDebug() << iLabelF;
            qreal i = minValueX;
            qreal labelStep = 0.0;
            //    qDebug() << "dimX.stepWidth:" << dimX.stepWidth  << "labelDiff:" << labelDiff;
            //dimX.stepWidth = 0.5;
            while( i <= maxValueX && d->annotations.isEmpty() )
            {
                // Line charts: we want the first tick to begin at 0.0 not at 0.5 otherwise labels and
                // values does not fit each others
                QPointF topPoint = diagramIsVertical ? QPointF( i + ( centerAbscissaTicks ? 0.5 : 0.0 ), 0.0 ) : QPointF( 0.0, i + ( centerAbscissaTicks ? 0.5 : 0.0 ) );
                QPointF bottomPoint ( topPoint );
                topPoint = plane->translate( topPoint );
                bottomPoint = plane->translate( bottomPoint );
                if ( diagramIsVertical ) {
                	topPoint.setY( rulerRef.y() + tickLength() );
                    bottomPoint.setY( rulerRef.y() );
                } else {
                    bottomPoint.setX( rulerRef.x() - (position() == Left ? tickLength() : -tickLength()) );
                	topPoint.setX( rulerRef.x() );
                }

                const qreal translatedValue = diagramIsVertical ? topPoint.x() : topPoint.y();
                bool bIsVisibleLabel;
                if ( diagramIsVertical )
                	bIsVisibleLabel = ( translatedValue >= geoRect.left() && translatedValue <= geoRect.right() && !isLogarithmicX || i != 0.0 );
                else
                	bIsVisibleLabel = ( translatedValue >= geoRect.top() && translatedValue <= geoRect.bottom() && !isLogarithmicX || i != 0.0 );

                // fix for issue #4179:
                bool painttick = bIsVisibleLabel && labelStep <= 0;;
                // old code:
                // bool painttick = true;

                //Dont paint more ticks than we need
                //when diagram type is Bar
                if (  centerAbscissaTicks && i == maxValueX )
                    painttick = false;

                if ( bIsVisibleLabel && painttick ) {
                    ptr->save();
                    if ( rulerAttr.hasTickMarkPenAt( i ) )
                    	ptr->setPen( rulerAttr.tickMarkPen( i ) );
                    else
                    	ptr->setPen( rulerAttr.majorTickMarkPen() );
                    ptr->drawLine( topPoint, bottomPoint );
                    ptr->restore();
                }

                drawnAbscissaTicks.append( static_cast<int>( diagramIsVertical ? topPoint.x() : topPoint.y() ) );
                if( drawLabels ) {
                    if( bIsVisibleLabel ){
                        if ( isLogarithmicX )
                            labelItem->setText( customizedLabel(QString::number( i ) ) );
                        /* We don't need that
                        * it causes header labels to be skipped even if there is enough
                        * space for them to displayed.
                        * Commenting for now - I need to test more in details - Let me know if I am wrong here.
                        */
                        /*
                        else if( (dimX.stepWidth != 1.0) && ! dimX.isCalculated ) {
                        labelItem->setText( customizedLabel(QString::number( i, 'f', 0 )) );
                        }
                        */
                        else {
                            int idx = idxLabel + static_cast<int>(minValueX);
                            if( hardLabelsCount ){
                                if( useShortLabels ){
                                    if( idx >= shortLabelsList.count() )
                                        idx = 0;
                                }else{
                                    if( idx >= labelsList.count() )
                                        idx = 0;
                                }
                            }
                            labelItem->setText(
                                    customizedLabel(
                                          hardLabelsCount
                                    ? ( useShortLabels    ? shortLabelsList[ idx ] : labelsList[ idx ] )
                                : ( headerLabelsCount ? headerLabels[ idx ] : QString::number( iLabelF ))));
                            //qDebug() << "x - labelItem->text() " << labelItem->text() << headerLabelsCount;
                        }
                        // No need to call labelItem->setParentWidget(), since we are using
                        // the layout item temporarily only.
                        if( labelStep <= 0 ) {
                            const PainterSaver p( ptr );
                            //const QSize size( labelItem->sizeHint() );
                            QPoint topLeft, topRight, bottomRight, bottomLeft;
                            const QSize size(
                                    labelItem->sizeHintAndRotatedCorners(
                                            topLeft, topRight, bottomRight, bottomLeft) );
                            const QSize sizeUnrotated( labelItem->sizeHintUnrotated() );
                            const int rotation = labelTA.rotation();
                            const bool rotPositive = (rotation > 0 && rotation < 180);
                            QPoint midOfSide(0,0);
                            int dX = 0;
                            int dY = 0;
                            if( rotation ){
                                if( rotPositive ){
                                    midOfSide = (topLeft + bottomLeft)  / 2;
                                    dX = topLeft.x() - midOfSide.x();
                                    dY = bottomLeft.y() - midOfSide.y();
                                }else{
                                    midOfSide = (topRight + bottomRight) / 2;
                                    dX = midOfSide.x() - topLeft.x();
                                    dY = midOfSide.y() - topRight.y();
                                }
                            }
/*
if( i == 2 ){
    qDebug()<<"------"<<size<<topPoint<<topLeft<<topRight<<bottomRight<<bottomLeft<<"   m:"<<midOfSide<<" dx"<<dX<<" dy"<<dY;
    ptr->setPen( Qt::black );
    QRectF rect(topPoint, QSizeF(sizeUnrotated));
    ptr->drawRect( rect );
    ptr->drawRect( QRectF(topPoint, QSizeF(2,2)) );
    ptr->drawRect( QRectF(topPoint+topLeft, QSizeF(2,2)) );
    ptr->drawRect( QRectF(topPoint+bottomLeft, QSizeF(2,2)) );
    ptr->drawRect( QRectF(topPoint+bottomRight, QSizeF(2,2)) );
    ptr->drawRect( QRectF(topPoint+topRight, QSizeF(2,2)) );
    ptr->drawRect( QRectF(topPoint+midOfSide, QSizeF(2,2)) );
    ptr->setPen( Qt::green );
    rect = QRectF(topPoint, QSizeF(size));
    ptr->drawRect( rect );
    ptr->drawRect( QRectF(QPointF((rect.topLeft()  + rect.bottomLeft())  / 2.0 - QPointF(2.0,2.0)), QSizeF(3.0,3.0)) );
    //ptr->drawRect( QRectF(QPointF((rect.topRight() + rect.bottomRight()) / 2.0 - QPointF(2.0,2.0)), QSizeF(3.0,3.0)) );
}
*/
                            QPoint topLeftPt;
                            if( diagramIsVertical ){
                                if( rotation ){
                                    topLeftPt = QPoint(
                                        static_cast<int>( topPoint.x() ) - dX,
                                        static_cast<int>( topPoint.y()   - dY +
                                                        ( position() == Bottom
                                                            ? halfFontHeight
                                                            : ((halfFontHeight + size.height()) * -1.0) ) ) );
                                }else{
                                    topLeftPt = QPoint(
                                        static_cast<int>( topPoint.x() - size.width() / 2.0 ),
                                        static_cast<int>( topPoint.y() +
                                                        ( position() == Bottom
                                                            ? halfFontHeight
                                                            : ((halfFontHeight + size.height()) * -1.0) ) ) );
                                }
                            }else{
                                if( rotation ){
                                    topLeftPt = QPoint(
                                        static_cast<int>( topPoint.x() ) + dX,
                                        static_cast<int>( topPoint.y()   - dY +
                                                        ( position() == Bottom
                                                            ? halfFontHeight
                                                            : ((halfFontHeight + size.height()) * -1.0) ) ) );
                                }else{
                                    topLeftPt = QPoint(
                                        static_cast<int>( bottomPoint.x() +
                                                            ( position() == Right
                                                                ? halfFontWidth
                                                                : (-halfFontWidth - size.width()) ) ),
                                        static_cast<int>( topPoint.y() - ( size.height() ) * 0.5 ) );
                                }
                            }
                            labelItem->setGeometry( QRect(topLeftPt, size) );

                            QRect labelGeo = labelItem->geometry();
                            //ptr->drawRect(labelGeo);
                            // if our item would only half fit, we disable clipping for that one
                            if( labelGeo.left() < geoRect.left() && labelGeo.right() > geoRect.left() )
                                ptr->setClipping( false );
                            else if( labelGeo.left() < geoRect.right() && labelGeo.right() > geoRect.right() )
                                ptr->setClipping( false );


                            if( !isLogarithmicX )
                                labelStep = labelDiff - dimX.stepWidth;

                            labelItem->paint( ptr );

                            // do not call customizedLabel() again:
                            labelItem2->setText( labelItem->text() );

                        } else {
                            labelStep -= dimX.stepWidth;
                        }
                    }

                    if( hardLabelsCount ) {
                        if( useShortLabels && idxLabel >= shortLabelsCount - 1 )
                            idxLabel = 0;
                        else if( !useShortLabels && idxLabel >= hardLabelsCount - 1 )
                            idxLabel = 0;
                        else{
                            idxLabel += static_cast<int>(dimX.stepWidth);
                            //qDebug() << "dimX.stepWidth:" << dimX.stepWidth << "  idxLabel:" << idxLabel;
                        }
                    } else if( headerLabelsCount ) {
                        if( ++idxLabel > headerLabelsCount - 1 ) {
                            idxLabel = 0;
                        }
                    } else {
                        iLabelF += dimX.stepWidth;
                    }
                }
                if ( isLogarithmicX )
                {
                    i *= 10.0;
                    if( i == 0.0 )
                    {
                        const qreal j = dimensions.first().start;
                        i = j == 0.0 ? 1.0 : pow( 10.0, floor( log10( j ) ) );
                    }
                }
                else
                {
                    i += dimX.stepWidth;
                }
            }
        } else {
            const PainterSaver p( ptr );
            const double maxLimit = maxValueY;
            const double steg = dimY.stepWidth;
            int maxLabelsWidth = 0;
            qreal labelValue;
            if( drawLabels && position() == Right ){
                // Find the widest label, so we to know how much we need to right-shift
                // our labels, to get them drawn right aligned:
                labelValue = minValueY;
                while ( labelValue <= maxLimit ) {
                    const QString labelText = diagram()->unitPrefix( static_cast< int >( labelValue ), diagramOrientation, true ) +
                                              QString::number( labelValue ) +
                                              diagram()->unitSuffix( static_cast< int >( labelValue ), diagramOrientation, true );
                    labelItem->setText( customizedLabel( labelText ) );
                    maxLabelsWidth = qMax( maxLabelsWidth, diagramIsVertical ? labelItem->sizeHint().width() : labelItem->sizeHint().height() );

                    calculateNextLabel( labelValue, steg, isLogarithmicY, dimensions.last().start );
                }
            }

            labelValue = minValueY;
            qreal step = steg;
            bool nextLabel = false;
            //qDebug("minValueY: %f   maxLimit: %f   steg: %f", minValueY, maxLimit, steg);

            if( drawLabels )
            {
                // first calculate the steps depending on labels colision
                while( labelValue <= maxLimit ) {
                    QPointF leftPoint = plane->translate( diagramIsVertical ? QPointF( 0, labelValue ) : QPointF( labelValue, 0 ) );
                    const qreal translatedValue = diagramIsVertical ? leftPoint.y() : leftPoint.x();
                    //qDebug() << "geoRect:" << geoRect << "   geoRect.top()" << geoRect.top()
                    //<< "geoRect.bottom()" << geoRect.bottom() << "  translatedValue:" << translatedValue;
                    const bool bTranslatedValueIsWithinRange = diagramIsVertical ? translatedValue > geoRect.top() && translatedValue <= geoRect.bottom()
                    														     : translatedValue > geoRect.left() && translatedValue <= geoRect.right();
                    if( bTranslatedValueIsWithinRange ){
                        const QString labelText = diagram()->unitPrefix( static_cast< int >( labelValue ), diagramOrientation, true ) +
                                                  QString::number( labelValue ) +
                                                  diagram()->unitSuffix( static_cast< int >( labelValue ), diagramOrientation, true );
                        const QString label2Text = diagram()->unitPrefix( static_cast< int >( labelValue + step ), diagramOrientation, true ) +
                                                   QString::number( labelValue + step ) +
                                                   diagram()->unitSuffix( static_cast< int >( labelValue + step ), diagramOrientation, true );
                        labelItem->setText(  customizedLabel( labelText ) );
                        labelItem2->setText( customizedLabel( QString::number( labelValue + step ) ) );
                        QPointF nextPoint = plane->translate(  diagramIsVertical ? QPointF( 0,  labelValue + step ) : QPointF( labelValue + step, 0 ) );
                        if ( labelItem->intersects( *labelItem2, leftPoint, nextPoint ) )
                        {
                            step += steg;
                            nextLabel = false;
                        }else{
                            nextLabel = true;
                        }
                    }else{
                        nextLabel = true;
                    }

                    if ( nextLabel || isLogarithmicY )
                        calculateNextLabel( labelValue, step, isLogarithmicY, dimensions.last().start );
                    else
                        labelValue = minValueY;
                }

                // Second - Paint the labels
                labelValue = minValueY;
                //qDebug() << "axis labels starting at" << labelValue << "step width" << step;
                if( !d->annotations.isEmpty() )
                {
                    const QList< double > annotations = d->annotations.keys();
                    KDAB_FOREACH( const double annotation, annotations )
                    {
                    	QPointF annoPoint = (diagramIsVertical ? QPointF( 0.0, annotation ) : QPointF( annotation, 0.0 ));
                        QPointF leftPoint = plane->translate( annoPoint );
                        QPointF rightPoint = plane->translate( annoPoint );

                        if ( diagramIsVertical ) {
                        	leftPoint.setX( rulerRef.x() + tickLength() );
                        	rightPoint.setX( rulerRef.x() );
                        } else {
                        	leftPoint.setY( rulerRef.y() + ((position() == Bottom) ? tickLength() : -tickLength()) );
                        	rightPoint.setY( rulerRef.y() );
                        }

                        const qreal translatedValue = diagramIsVertical ? rightPoint.y() : rightPoint.x();
                        const bool bIsVisibleLabel = diagramIsVertical ?
                                ( translatedValue >= geoRect.top() && translatedValue <= geoRect.bottom() && !isLogarithmicY || labelValue != 0.0 )
                              : ( translatedValue >= geoRect.left() && translatedValue <= geoRect.right() && !isLogarithmicY || labelValue != 0.0 );

                        if( bIsVisibleLabel )
                        {
                        	ptr->save();
                            if ( rulerAttr.hasTickMarkPenAt( annotation ) )
                            	ptr->setPen( rulerAttr.tickMarkPen( annotation ) );
                            else
                            	ptr->setPen( rulerAttr.majorTickMarkPen() );
                            ptr->drawLine( leftPoint, rightPoint );
                            ptr->restore();

                            labelItem->setText( d->annotations[ annotation ] );
                            const QSize labelSize( labelItem->sizeHint() );
                            int x, y;
                            if ( diagramIsVertical ) {
                                x = static_cast<int>( leftPoint.x() + met.height() * ( position() == Left ? -0.5 : 0.5)
                                    - ( position() == Left ? labelSize.width() : 0.0 ) );
    	                        y = static_cast<int>( leftPoint.y() - ( met.ascent() + met.descent() ) * 0.6 );
                            } else {
                            	const qreal halfFontHeight = met.height() * 0.5;
    	                        x = static_cast<int>( leftPoint.x() - labelSize.width() * 0.5 );
    	                        y = static_cast<int>( (position() == Bottom ? leftPoint.y() : rightPoint.y()) +
    	                                                        + ( position() == Bottom ? halfFontHeight : -(halfFontHeight + labelSize.height()) ) );
                            }
                            labelItem->setGeometry( QRect( QPoint( x, y ), labelSize ) );
                            labelItem->paint( ptr );
                        }
                    }
                }
                else
                {
                while( labelValue <= maxLimit ) {
                    //qDebug() << "value now" << labelValue;
                    const QString labelText = diagram()->unitPrefix( static_cast< int >( labelValue ), diagramOrientation, true ) +
                                              QString::number( labelValue ) +
                                              diagram()->unitSuffix( static_cast< int >( labelValue ), diagramOrientation, true );
                    labelItem->setText( customizedLabel( labelText ) );
                    QPointF leftPoint = plane->translate( diagramIsVertical ? QPointF( 0, labelValue ) : QPointF( labelValue, 0 ) );
                    QPointF rightPoint = leftPoint;

                    if ( diagramIsVertical ) {
                    	leftPoint.setX( rulerRef.x() + tickLength() );
                    	rightPoint.setX( rulerRef.x() );
                    } else {
                    	leftPoint.setY( rulerRef.y() + ((position() == Bottom) ? tickLength() : -tickLength()) );
                    	rightPoint.setY( rulerRef.y() );
                    }

                    bool bIsVisibleLabel;
                    const qreal translatedValue = diagramIsVertical ? rightPoint.y() : rightPoint.x();
                    if ( diagramIsVertical)
                    	bIsVisibleLabel = ( translatedValue >= geoRect.top() && translatedValue <= geoRect.bottom() && !isLogarithmicY || labelValue != 0.0 );
                    else
                    	bIsVisibleLabel = ( translatedValue >= geoRect.left() && translatedValue <= geoRect.right() && !isLogarithmicY || labelValue != 0.0 );

                    if( bIsVisibleLabel ){
                    	ptr->save();
                        if ( rulerAttr.hasTickMarkPenAt( labelValue ) )
                        	ptr->setPen( rulerAttr.tickMarkPen( labelValue ) );
                        else
                        	ptr->setPen( rulerAttr.majorTickMarkPen() );
                        ptr->drawLine( leftPoint, rightPoint );
                        ptr->restore();

                        drawnYTicks.append( static_cast<int>( diagramIsVertical ? leftPoint.y() : leftPoint.x() ) );
                        const QSize labelSize( labelItem->sizeHint() );

                        int x, y;
                        if ( diagramIsVertical ) {
	                        x = static_cast<int>( leftPoint.x() + met.height() * ( position() == Left ? -0.5 : 0.5) )
	                            - ( position() == Left ? labelSize.width() : (labelSize.width() - maxLabelsWidth) );
	                        y = static_cast<int>( leftPoint.y() - ( met.ascent() + met.descent() ) * 0.6 );
                        } else {
                        	const qreal halfFontHeight = met.height() * 0.5;
	                        x = static_cast<int>( leftPoint.x() - labelSize.width() * 0.5 );
	                        y = static_cast<int>( (position() == Bottom ? leftPoint.y() : rightPoint.y()) +
	                                                        + ( position() == Bottom ? halfFontHeight : -(halfFontHeight + labelSize.height()) ) );
                        }

                        labelItem->setGeometry( QRect( QPoint( x, y ), labelSize ) );
                        const QRect labelGeo = labelItem->geometry();
                        const bool hadClipping = ptr->hasClipping();
                        if( labelGeo.top() < geoRect.top() && labelGeo.bottom() > geoRect.top() )
                           ptr->setClipping( false );
                        else if( labelGeo.top() < geoRect.bottom() && labelGeo.bottom() > geoRect.bottom() )
                           ptr->setClipping( false );

                        labelItem->paint( ptr );
                        ptr->setClipping( hadClipping );
                    }

                    //qDebug() << step;
                    calculateNextLabel( labelValue, step, isLogarithmicY, dimensions.last().start );
                }
                }
            }
        }
        delete labelItem;
        delete labelItem2;
    }

    // this draws the subunit rulers
    if ( drawSubUnitRulers && d->annotations.isEmpty() ) {
    	ptr->save();

        d->drawSubUnitRulers( ptr, plane, dim, rulerRef, isAbscissa() ? drawnAbscissaTicks : drawnYTicks, diagramIsVertical, rulerAttr );

        ptr->restore();
    }

    if( ! titleText().isEmpty() ){
        d->drawTitleText( ptr, plane, areaGeoRect );
    }

    //qDebug() << "KDChart::CartesianAxis::paintCtx() done.";
}


/* pure virtual in QLayoutItem */
bool CartesianAxis::isEmpty() const
{
    return false; // if the axis exists, it has some (perhaps default) content
}
/* pure virtual in QLayoutItem */
Qt::Orientations CartesianAxis::expandingDirections() const
{
    Qt::Orientations ret;
    switch ( position() )
    {
    case Bottom:
    case Top:
        ret = Qt::Horizontal;
        break;
    case Left:
    case Right:
        ret = Qt::Vertical;
        break;
    default:
        Q_ASSERT( false ); // all positions need to be handeld
        break;
    };
    return ret;
}


static void calculateOverlap( int i, int first, int last,
                              int measure,
                              bool centerAbscissaTicks,
                              int& firstOverlap, int& lastOverlap )
{
    if( i == first ){
        if( centerAbscissaTicks ){
            //TODO(khz): Calculate the amount of left overlap
            //           for bar diagrams.
        }else{
            firstOverlap = measure / 2;
        }
    }
    // we test both bounds in on go: first and last might be equal
    if( i == last ){
        if( centerAbscissaTicks ){
            //TODO(khz): Calculate the amount of right overlap
            //           for bar diagrams.
        }else{
            lastOverlap = measure / 2;
        }
    }
}


void CartesianAxis::setCachedSizeDirty() const
{
    d->cachedMaximumSize = QSize();
}

/* pure virtual in QLayoutItem */
QSize CartesianAxis::maximumSize() const
{
    if( ! d->cachedMaximumSize.isValid() )
        d->cachedMaximumSize = d->calculateMaximumSize();
    return d->cachedMaximumSize;
}

QSize CartesianAxis::Private::calculateMaximumSize() const
{
    QSize result;
    if ( !diagram() )
        return result;

    const AbstractCartesianDiagram * dia = qobject_cast< const AbstractCartesianDiagram * >( diagram() );
    if( dia && dia->referenceDiagram() )
        dia = dia->referenceDiagram();
	const BarDiagram *barDiagram = qobject_cast< const BarDiagram* >( dia );
	const Qt::Orientation diagramOrientation = barDiagram != 0 ? barDiagram->orientation() : Qt::Vertical;
    const bool diagramIsVertical = diagramOrientation == Qt::Vertical;

    const TextAttributes labelTA = mAxis->textAttributes();
    const bool drawLabels = labelTA.isVisible();

    const TextAttributes titleTA( titleTextAttributesWithAdjustedRotation() );
    const bool drawTitle = titleTA.isVisible() && ! axis()->titleText().isEmpty();

    AbstractCoordinatePlane* plane = diagram()->coordinatePlane();
    //qDebug() << this<<"::maximumSize() uses plane geometry" << plane->geometry();
    QObject* refArea = plane->parent();
    TextLayoutItem labelItem( QString::null, labelTA, refArea,
                              KDChartEnums::MeasureOrientationMinimum, Qt::AlignLeft );
    TextLayoutItem titleItem( axis()->titleText(), titleTA, refArea,
                              KDChartEnums::MeasureOrientationMinimum, Qt::AlignHCenter | Qt::AlignVCenter );

    const QFontMetrics fm( labelItem.realFont(), GlobalMeasureScaling::paintDevice() );

    const qreal labelGap =
        drawLabels
        ? ( (diagramIsVertical ? fm.height() : fm.averageCharWidth()) / 3.0)
        : 0.0;
    const QFontMetricsF titleFM = QFontMetricsF( titleItem.realFont(), GlobalMeasureScaling::paintDevice() );
    const qreal titleGap =
        drawTitle
        ? ( (diagramIsVertical ? titleFM.height() : titleFM.averageCharWidth()) / 3.0)
        : 0.0;

    if ( axis()->isAbscissa() ) {
        const bool centerAbscissaTicks = referenceDiagramNeedsCenteredAbscissaTicks(diagram());
        int leftOverlap = 0;
        int rightOverlap = 0;

        qreal w = diagramIsVertical ? 10.0 : 0.0;
        qreal h = diagramIsVertical ? 0.0 : 10.0;
        if( drawLabels ){
            // if there're no label strings, we take the biggest needed number as height
            if( !annotations.isEmpty() )
            {
                const QStringList strings = annotations.values();
                KDAB_FOREACH( const QString& string, strings )
                {
                    labelItem.setText( string );
                    const QSize siz = labelItem.sizeHint();
                    if ( diagramIsVertical )
                    	h = qMax( h, static_cast< qreal >( siz.height() ) );
                    else
                    	w = qMax( w, static_cast< qreal >( siz.width() ) );
                }
            }
            else if ( !axis()->labels().isEmpty() )
            {
                // find the longest label text:
                const int first=0;
                const int last=axis()->labels().count()-1;
                const QStringList labelsList( axis()->labels() );
                for ( int i = first; i <= last; ++i )
                {
                    labelItem.setText( axis()->customizedLabel(labelsList[ i ]) );
                    const QSize siz = labelItem.sizeHint();
                    //qDebug()<<siz;
                    if ( diagramIsVertical )
                    	h = qMax( h, static_cast<qreal>(siz.height()) );
                    else
                    	w = qMax( w, static_cast<qreal>(siz.width()) );
                    calculateOverlap( i, first, last, diagramIsVertical ? siz.width() : siz.height(), centerAbscissaTicks,
                                      leftOverlap, rightOverlap );

                }
            }
            else
            {
                QStringList headerLabels = diagram()->itemRowLabels();
                const int headerLabelsCount = headerLabels.count();
                if( headerLabelsCount ){
                    if( cachedHeaderLabels == headerLabels && ( diagramIsVertical ? cachedFontHeight == fm.height() : cachedFontWidth == fm.averageCharWidth() )) {
                    	if ( diagramIsVertical )
                    		h = cachedLabelHeight;
                    	else
                    		w = cachedLabelWidth;
                    } else {
                        cachedHeaderLabels = headerLabels;
                        if ( diagramIsVertical )
                        	cachedFontWidth = fm.averageCharWidth();
                        else
                        	cachedFontHeight = fm.height();
                        const bool useFastCalcAlgorithm
                            = (strcmp( axis()->metaObject()->className(), "KDChart::CartesianAxis" ) == 0);
                        const int first=0;
                        const int last=headerLabelsCount-1;
                        for ( int i = first;
                            i <= last;
                            i = (useFastCalcAlgorithm && i < last) ? last : (i+1) )
                        {
                            labelItem.setText( axis()->customizedLabel(headerLabels[ i ]) );
                            const QSize siz = labelItem.sizeHint();
                            if ( diagramIsVertical ) {
                            	h = qMax( h, static_cast<qreal>(siz.height()) );
                                cachedLabelHeight = h;
                            } else {
                                cachedLabelWidth = w;
                            	w = qMax( w, static_cast<qreal>(siz.width()) );
                            }
                            calculateOverlap( i, first, last, diagramIsVertical ? siz.width() : siz.height(), centerAbscissaTicks,
                                            leftOverlap, rightOverlap );
                        }
                    }
                }else{
                    labelItem.setText(
                            axis()->customizedLabel(
                                    QString::number( diagramIsVertical ? plane->gridDimensionsList().first().end
                                    								   : plane->gridDimensionsList().last().end, 'f', 0 )));
                    const QSize siz = labelItem.sizeHint();
                    if ( diagramIsVertical )
                    	h = siz.height();
                    else
                    	w = siz.width();
                    calculateOverlap( 0, 0, 0, siz.width(), centerAbscissaTicks,
                                      leftOverlap, rightOverlap );
                }
            }
            // we leave a little gap between axis labels and bottom (or top, resp.) side of axis
            h += labelGap;
        }
        // space for a possible title:
        if ( drawTitle ) {
            // we add the title height and leave a little gap between axis labels and axis title
        	if ( diagramIsVertical ) {
        		h += titleItem.sizeHint().height() + titleGap;
        		w = titleItem.sizeHint().width() + 2.0;
        	} else {
        		h = titleItem.sizeHint().height() + 2.0;
        		w += titleItem.sizeHint().width() + titleGap;
        	}
        }
        // space for the ticks
        if ( diagramIsVertical )
        	h += qAbs( axis()->tickLength() ) * 3.0;
        else
        	w += qAbs( axis()->tickLength() ) * 3.0;
        result = QSize ( static_cast<int>( w ), static_cast<int>( h ) );

        //qDebug()<<"calculated size of x axis:"<<result;

        // If necessary adjust the widths
        // of the left (or right, resp.) side neighboring columns:
        amountOfLeftOverlap = leftOverlap;
        amountOfRightOverlap = rightOverlap;
        /* Unused code for a push-model:
        if( leftOverlap || rightOverlap ){
            QTimer::singleShot(200, const_cast<CartesianAxis*>(this),
                               SLOT(adjustLeftRightGridColumnWidths()));
        }
        */
    } else {
        int topOverlap = 0;
        int bottomOverlap = 0;

        qreal w = diagramIsVertical ? 0.0 : 10.0;
        qreal h = diagramIsVertical ? 10.0 : 0.0;
        if( drawLabels ){
            // if there're no label strings, we loop through the values
            // taking the longest (not largest) number - e.g. 0.00001 is longer than 100
            if( !annotations.isEmpty() )
            {
                const QStringList strings = annotations.values();
                KDAB_FOREACH( const QString& string, strings )
                {
                    labelItem.setText( string );
                    const QSize siz = labelItem.sizeHint();
                    if ( diagramIsVertical )
                    	w = qMax( w, static_cast< qreal >( siz.width() ) );
                    else
                    	h = qMax( h, static_cast< qreal >( siz.height() ) );
                }
            }
            else if( axis()->labels().isEmpty() )
            {
                const DataDimension dimY = AbstractGrid::adjustedLowerUpperRange(
                		diagramIsVertical ? plane->gridDimensionsList().last()
                				          : plane->gridDimensionsList().first(), true, true );
                const double step = dimY.stepWidth;
                const qreal minValue = dimY.start;
                const qreal maxValue = dimY.end;
                const bool isLogarithmicY = (dimY.calcMode == AbstractCoordinatePlane::Logarithmic );
                qreal labelValue = minValue;

                while( labelValue <= maxValue ) {
                    const QString labelText = diagram()->unitPrefix( static_cast< int >( labelValue ), diagramOrientation, true ) +
                                            QString::number( labelValue ) +
                                            diagram()->unitSuffix( static_cast< int >( labelValue ), diagramOrientation, true );
                    labelItem.setText( axis()->customizedLabel( labelText ) );

                    const QSize siz = labelItem.sizeHint();
                    if ( diagramIsVertical )
                    	w = qMax( w, (qreal)siz.width() );
                    else
                    	h = qMax( h, (qreal)siz.height() );
                    calculateOverlap( 0, 0, 0, diagramIsVertical ? siz.height() : siz.width(), false,// bar diagram flag is ignored for Ordinates
                                    topOverlap, bottomOverlap );
                    calculateNextLabel( labelValue, step, isLogarithmicY, plane->gridDimensionsList().last().start );
                }
            }else{
                // find the longest label text:
                const int first=0;
                const int last=axis()->labels().count()-1;
                const QStringList labelsList( axis()->labels() );
                for ( int i = first; i <= last; ++i )
                {
                    labelItem.setText( axis()->customizedLabel(labelsList[ i ]) );
                    const QSize siz = labelItem.sizeHint();
                    if ( diagramIsVertical )
                                        	w = qMax( w, (qreal)siz.width() );
                                        else
                                        	h = qMax( h, (qreal)siz.height() );
                    calculateOverlap( 0, 0, 0, diagramIsVertical ? siz.height() : siz.width(), false,// bar diagram flag is ignored for Ordinates
                                      topOverlap, bottomOverlap );
                }
            }
            // we leave a little gap between axis labels and left (or right, resp.) side of axis
            w += labelGap;
        }
        // space for a possible title:
        if ( drawTitle ) {
            // we add the title height and leave a little gap between axis labels and axis title
        	if ( diagramIsVertical ) {
        		w += titleItem.sizeHint().width() + titleGap;
        		h = titleItem.sizeHint().height() + 2.0;
        	} else {
        		w = titleItem.sizeHint().width() + 2.0;
        		h += titleItem.sizeHint().height() + titleGap;
        	}
            //qDebug() << "left/right axis title item size-hint:" << titleItem.sizeHint();
        }
        // space for the ticks
        if ( diagramIsVertical )
        	w += qAbs( axis()->tickLength() ) * 3.0;
        else
        	h += qAbs( axis()->tickLength() ) * 3.0;

        result = QSize ( static_cast<int>( w ), static_cast<int>( h ) );
        //qDebug() << "left/right axis width:" << result << "   w:" << w;


        // If necessary adjust the heights
        // of the top (or bottom, resp.) side neighboring rows:
        amountOfTopOverlap = topOverlap;
        amountOfBottomOverlap = bottomOverlap;
        /* Unused code for a push-model:
        if( topOverlap || bottomOverlap ){
            QTimer::singleShot(200, const_cast<CartesianAxis*>(this),
                               SLOT(adjustTopBottomGridRowHeights()));
        }
        */
    }
//qDebug() << "*******************" << result;
    //result=QSize(0,0);
    return result;
}
/* pure virtual in QLayoutItem */
QSize CartesianAxis::minimumSize() const
{
    return maximumSize();
}
/* pure virtual in QLayoutItem */
QSize CartesianAxis::sizeHint() const
{
    return maximumSize();
}
/* pure virtual in QLayoutItem */
void CartesianAxis::setGeometry( const QRect& r )
{
//    qDebug() << "KDChart::CartesianAxis::setGeometry(" << r << ") called"
//             << (isAbscissa() ? "for Abscissa":"for Ordinate") << "axis";
    d->geometry = r;
    setCachedSizeDirty();
}
/* pure virtual in QLayoutItem */
QRect CartesianAxis::geometry() const
{
    return d->geometry;
}

int CartesianAxis::tickLength( bool subUnitTicks ) const
{
    int result = 0;

    if ( isAbscissa() ) {
        result = position() == Top ? -4 : 3;
    } else {
        result = position() == Left ? -4 : 3;
    }

    if ( subUnitTicks )
        result = result < 0 ? result + 1 : result - 1;

    return result;
}

QMap< double, QString > CartesianAxis::annotations() const
{
    return d->annotations;
}

void CartesianAxis::setAnnotations( const QMap< double, QString >& annotations )
{
    if( d->annotations == annotations )
        return;

    d->annotations = annotations;
    update();
}


/* unused code from KDChartCartesianAxis.h for using a push-model:
Q_SIGNALS:
    void needAdjustLeftRightColumnsForOverlappingLabels(
            CartesianAxis* axis, int left, int right );
    void needAdjustTopBottomRowsForOverlappingLabels(
            CartesianAxis* axis, int top, int bottom );
private Q_SLOTS:
    void adjustLeftRightGridColumnWidths();
    void adjustTopBottomGridRowHeights();
*/

/*
// Unused code trying to use a push-model: This did not work
// since we can not re-layout the planes each time when
// Qt layouting is calling sizeHint()
void CartesianAxis::adjustLeftRightGridColumnWidths()
{
    if( ! d->amountOfLeftOverlap && ! d->amountOfRightOverlap )
        return;
    const int leftOverlap = d->amountOfLeftOverlap;
    const int rightOverlap= d->amountOfRightOverlap;
    d->amountOfLeftOverlap = 0;
    d->amountOfRightOverlap = 0;
    emit needAdjustLeftRightColumnsForOverlappingLabels(
            this, leftOverlap, rightOverlap );
}

void CartesianAxis::adjustTopBottomGridRowHeights()
{
    if( ! d->amountOfTopOverlap && ! d->amountOfBottomOverlap )
        return;
    const int topOverlap = d->amountOfTopOverlap;
    const int bottomOverlap= d->amountOfBottomOverlap;
    d->amountOfTopOverlap = 0;
    d->amountOfBottomOverlap = 0;
    emit needAdjustTopBottomRowsForOverlappingLabels(
            this, topOverlap, bottomOverlap );
}
*/
