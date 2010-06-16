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

#include "KDChartLeveyJenningsAxis.h"
#include "KDChartLeveyJenningsAxis_p.h"

#include <QDateTime>
#include <QPainter>

#include "KDChartPaintContext.h"
#include "KDChartChart.h"
#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartAbstractGrid.h"
#include "KDChartPainterSaver_p.h"
#include "KDChartLayoutItems.h"
#include "KDChartPrintingParameters.h"

#include <KDABLibFakes>

#include <limits>

using namespace KDChart;

#define d (d_func())

LeveyJenningsAxis::LeveyJenningsAxis ( LeveyJenningsDiagram* diagram )
    : CartesianAxis ( new Private( diagram, this ), diagram )
{
    init();
}

LeveyJenningsAxis::~LeveyJenningsAxis ()
{
    // when we remove the first axis it will unregister itself and
    // propagate the next one to the primary, thus the while loop
    while ( d->mDiagram ) {
        LeveyJenningsDiagram *cd = qobject_cast< LeveyJenningsDiagram* >( d->mDiagram );
        cd->takeAxis( this );
    }
    Q_FOREACH( AbstractDiagram *diagram, d->secondaryDiagrams ) {
        LeveyJenningsDiagram *cd = qobject_cast< LeveyJenningsDiagram* >( diagram );
        cd->takeAxis( this );
    }
}

void LeveyJenningsAxis::init ()
{
    setType( LeveyJenningsGridAttributes::Expected );
    setDateFormat( Qt::TextDate );
    const QStringList labels = QStringList() << tr( "-3sd" ) << tr( "-2sd" ) << tr( "mean" )
                                             << tr( "+2sd" ) << tr( "+3sd" );

    setLabels( labels );
}

/**
  * @return The axis' type.
  */
LeveyJenningsGridAttributes::GridType LeveyJenningsAxis::type() const
{
    return d->type;
}

/**
  * Sets the type of the axis to \a type.
  * This method colors the label to the default color of the
  * respective type.
  * Please make sure to re-set the colors after calling this,
  * if you want them different.
  * Setting the type is only valid for axes located right or left
  * from the diagram. An axis on the bottom always shows the timeline.
  */
void LeveyJenningsAxis::setType( LeveyJenningsGridAttributes::GridType type )
{
    if( type != d->type )
    {
        TextAttributes ta = textAttributes();
        QPen pen = ta.pen();
        QColor color = type == LeveyJenningsGridAttributes::Expected ? Qt::black : Qt::blue;
        if( qobject_cast< const LeveyJenningsDiagram* >( d->diagram() ) && 
            qobject_cast< const LeveyJenningsCoordinatePlane* >( d->diagram()->coordinatePlane() ) )
        {
            color = qobject_cast< const LeveyJenningsCoordinatePlane* >( d->diagram()->coordinatePlane() )->gridAttributes().gridPen( type ).color();
        }
        pen.setColor( color );
        ta.setPen( pen );
        setTextAttributes( ta );
    }
    d->type = type;
}

Qt::DateFormat LeveyJenningsAxis::dateFormat() const
{
    return d->format;
}

void LeveyJenningsAxis::setDateFormat(Qt::DateFormat format)
{
    d->format = format;
}

bool LeveyJenningsAxis::compare( const LeveyJenningsAxis* other )const
{
    if( other == this ) return true;
    if( ! other ){
        //qDebug() << "CartesianAxis::compare() cannot compare to Null pointer";
        return false;
    }
    return  ( static_cast<const CartesianAxis*>(this)->compare( other ) ) &&
            ( type() == other->type() );
}

void LeveyJenningsAxis::paintCtx( PaintContext* context )
{

    Q_ASSERT_X ( d->diagram(), "LeveyJenningsAxis::paint",
                 "Function call not allowed: The axis is not assigned to any diagram." );

    LeveyJenningsCoordinatePlane* plane = dynamic_cast<LeveyJenningsCoordinatePlane*>(context->coordinatePlane());
    Q_ASSERT_X ( plane, "LeveyJenningsAxis::paint",
                 "Bad function call: PaintContext::coodinatePlane() NOT a levey jennings plane." );

    // note: Not having any data model assigned is no bug
    //       but we can not draw an axis then either.
    if( ! d->diagram()->model() )
        return;

    if( isOrdinate() )
        paintAsOrdinate( context );
    else
        paintAsAbscissa( context );
}

void LeveyJenningsAxis::paintAsOrdinate( PaintContext* context )
{
    const LeveyJenningsDiagram* const diag = dynamic_cast< const LeveyJenningsDiagram* >( d->diagram() );

    Q_ASSERT( isOrdinate() );
    LeveyJenningsCoordinatePlane* plane = dynamic_cast<LeveyJenningsCoordinatePlane*>(context->coordinatePlane());
    
    const qreal meanValue =         type() == LeveyJenningsGridAttributes::Expected ? diag->expectedMeanValue() 
                                                                                    : diag->calculatedMeanValue();
    const qreal standardDeviation = type() == LeveyJenningsGridAttributes::Expected ? diag->expectedStandardDeviation() 
                                                                                    : diag->calculatedStandardDeviation();
    const TextAttributes labelTA = textAttributes();
    const bool drawLabels = labelTA.isVisible();

    // nothing to draw, since we've no ticks
    if( !drawLabels )
        return;
    
    const QObject* referenceArea = plane->parent();

    const QVector< qreal > values = QVector< qreal >() << ( meanValue - 3 * standardDeviation )
                                                       << ( meanValue - 2 * standardDeviation )
                                                       << ( meanValue )
                                                       << ( meanValue + 2 * standardDeviation )
                                                       << ( meanValue + 3 * standardDeviation );

    Q_ASSERT_X( values.count() <= labels().count(), "LeveyJenningsAxis::paintAsOrdinate", "Need to have at least 5 labels" );

    TextLayoutItem labelItem( tr( "mean" ), 
                              labelTA,
                              referenceArea,
                              KDChartEnums::MeasureOrientationMinimum,
                              Qt::AlignLeft );

    QPainter* const painter = context->painter();
    const PainterSaver ps( painter );
    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->setClipping( false );
    
    painter->setPen ( PrintingParameters::scalePen( labelTA.pen() ) ); // perhaps we want to add a setter method later?

    for( int i = 0; i < values.count(); ++i )
    {
        const QPointF labelPos = plane->translate( QPointF( 0.0, values.at( i ) ) );
        const QString label = customizedLabel( labels().at( i ) );
        labelItem.setText( label );
        const QSize size = labelItem.sizeHint();
        const float xPos = position() == Left ? geometry().right() - size.width() : geometry().left();
        labelItem.setGeometry( QRectF( QPointF( xPos, labelPos.y() - size.height() / 2.0 ), size ).toRect() );

        // don't draw labels which aren't in the valid range (might happen for calculated SDs)
        if( values.at( i ) > diag->expectedMeanValue() + 4 * diag->expectedStandardDeviation() )
            continue;

        if( values.at( i ) < diag->expectedMeanValue() - 4 * diag->expectedStandardDeviation() )
            continue;

        labelItem.paint( painter );
    }    
}

void LeveyJenningsAxis::paintAsAbscissa( PaintContext* context )
{
    Q_ASSERT( isAbscissa() );

    // this triggers drawing of the ticks
    setLabels( QStringList() << QString::fromLatin1( " " ) );
    CartesianAxis::paintCtx( context );

    const LeveyJenningsDiagram* const diag = dynamic_cast< const LeveyJenningsDiagram* >( d->diagram() );
    LeveyJenningsCoordinatePlane* plane = dynamic_cast<LeveyJenningsCoordinatePlane*>(context->coordinatePlane());

    const QObject* referenceArea = plane->parent();
    const TextAttributes labelTA = textAttributes();
    
    const bool drawLabels = labelTA.isVisible();

    if( !drawLabels )
        return;


    const QPair< QDateTime, QDateTime > range = diag->timeRange();

    QPainter* const painter = context->painter();
    const PainterSaver ps( painter );
    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->setClipping( false );
     

    TextLayoutItem labelItem( range.first.date().toString( dateFormat() ), 
                              labelTA,
                              referenceArea,
                              KDChartEnums::MeasureOrientationMinimum,
                              Qt::AlignLeft );
    QSize origSize = labelItem.sizeHint();
    if( range.first.secsTo( range.second ) < 86400 )
        labelItem = TextLayoutItem( range.first.toString( dateFormat() ), 
                                  labelTA,
                                  referenceArea,
                                  KDChartEnums::MeasureOrientationMinimum,
                                  Qt::AlignLeft );
    QSize size = labelItem.sizeHint();

    float yPos = position() == Bottom ? geometry().bottom() - size.height() : geometry().top();
    labelItem.setGeometry( QRectF( QPointF( geometry().left() - origSize.width() / 2.0, yPos ), size ).toRect() );
    labelItem.paint( painter );

    
    TextLayoutItem labelItem2( range.second.date().toString( dateFormat() ), 
                              labelTA,
                              referenceArea,
                              KDChartEnums::MeasureOrientationMinimum,
                              Qt::AlignLeft );
    origSize = labelItem2.sizeHint();
    if( range.first.secsTo( range.second ) < 86400 )
        labelItem2 = TextLayoutItem( range.second.toString( dateFormat() ), 
                                     labelTA,
                                     referenceArea,
                                     KDChartEnums::MeasureOrientationMinimum,
                                     Qt::AlignLeft );
    size = labelItem2.sizeHint();
    yPos = position() == Bottom ? geometry().bottom() - size.height() : geometry().top();
    labelItem2.setGeometry( QRectF( QPointF( geometry().right() - size.width() + origSize.width() / 2.0, yPos ), size ).toRect() );
    labelItem2.paint( painter );
}
