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

#include "KDChartLeveyJenningsDiagram.h"
#include "KDChartLeveyJenningsDiagram_p.h"

#include <QDateTime>
#include <QFontMetrics>
#include <QPainter>
#include <QSvgRenderer>
#include <QVector>

#include "KDChartChart.h"
#include "KDChartTextAttributes.h"
#include "KDChartAbstractGrid.h"

#include <KDABLibFakes>

using namespace KDChart;
using namespace std;

LeveyJenningsDiagram::Private::Private()
{
}

LeveyJenningsDiagram::Private::~Private() {}


#define d d_func()


LeveyJenningsDiagram::LeveyJenningsDiagram( QWidget* parent, LeveyJenningsCoordinatePlane* plane )
    : LineDiagram( new Private(), parent, plane )
{
    init();
}

void LeveyJenningsDiagram::init()
{
    d->lotChangedPosition = Qt::AlignTop;
    d->fluidicsPackChangedPosition = Qt::AlignBottom;
    d->sensorChangedPosition = Qt::AlignBottom;

    d->scanLinePen = QPen( Qt::blue );
    setPen( d->scanLinePen );

    d->expectedMeanValue = 0.0;
    d->expectedStandardDeviation = 0.0;

    d->diagram = this;

    d->icons[ LotChanged ] = QString::fromLatin1( ":/KDAB/kdchart/LeveyJennings/karo_black.svg" );
    d->icons[ SensorChanged ] = QString::fromLatin1( ":/KDAB/kdchart/LeveyJennings/karo_red.svg" );
    d->icons[ FluidicsPackChanged ] = QString::fromLatin1( ":/KDAB/kdchart/LeveyJennings/karo_blue.svg" );
    d->icons[ OkDataPoint ] = QString::fromLatin1( ":/KDAB/kdchart/LeveyJennings/circle_blue.svg" );
    d->icons[ NotOkDataPoint ] = QString::fromLatin1( ":/KDAB/kdchart/LeveyJennings/circle_blue_red.svg" );

    setSelectionMode( QAbstractItemView::SingleSelection );
}

LeveyJenningsDiagram::~LeveyJenningsDiagram()
{
}

/**
  * Creates an exact copy of this diagram.
  */
LineDiagram * LeveyJenningsDiagram::clone() const
{
    LeveyJenningsDiagram* newDiagram = new LeveyJenningsDiagram( new Private( *d ) );
    return newDiagram;
}

bool LeveyJenningsDiagram::compare( const LeveyJenningsDiagram* other )const
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
            ( static_cast<const LineDiagram*>(this)->compare( other ) );
}

/**
 * Sets the position of the lot change symbol to \a pos.
 * Valid values are: Qt::AlignTop (default), Qt::AlignBottom.
 */
void LeveyJenningsDiagram::setLotChangedSymbolPosition( Qt::Alignment pos )
{
    if( d->lotChangedPosition == pos )
        return;

    d->lotChangedPosition = pos;
    update();
}

/**
 * Returns the position of the lot change symbol.
 */
Qt::Alignment LeveyJenningsDiagram::lotChangedSymbolPosition() const
{
    return d->lotChangedPosition;
}

/**
 * Sets the position of the fluidics pack changed symbol to \a pos.
 * Valid values are: Qt::AlignBottom (default), Qt::AlignTop.
 */
void LeveyJenningsDiagram::setFluidicsPackChangedSymbolPosition( Qt::Alignment pos )
{
    if( d->fluidicsPackChangedPosition == pos )
        return;

    d->fluidicsPackChangedPosition = pos;
    update();
}

/**
 * Returns the position of the fluidics pack changed symbol.
 */
Qt::Alignment LeveyJenningsDiagram::fluidicsPackChangedSymbolPosition() const
{
    return d->fluidicsPackChangedPosition;
}

/**
 * Sets the position of the sensor changed symbol to \a pos.
 * Valid values are: Qt::AlignBottom (default), Qt::AlignTop.
 */
void LeveyJenningsDiagram::setSensorChangedSymbolPosition( Qt::Alignment pos )
{
    if( d->sensorChangedPosition == pos )
        return;

    d->sensorChangedPosition = pos;
    update();
}

/**
 * Returns the position of the sensor changed symbol.
 */
Qt::Alignment LeveyJenningsDiagram::sensorChangedSymbolPosition() const
{
    return d->sensorChangedPosition;
}

/**
 * Sets the date/time of all fluidics pack changes to \a changes.
 */
void LeveyJenningsDiagram::setFluidicsPackChanges( const QVector< QDateTime >& changes )
{
    if( d->fluidicsPackChanges == changes )
        return;

    d->fluidicsPackChanges = changes;
    update();
}

/**
 * Returns the list of all fluidics pack changes.
 */
QVector< QDateTime > LeveyJenningsDiagram::fluidicsPackChanges() const
{
    return d->fluidicsPackChanges;
}

/**
 * Sets the date/time of all sensor changes to \a changes.
 */
void LeveyJenningsDiagram::setSensorChanges( const QVector< QDateTime >& changes )
{
    if( d->sensorChanges == changes )
        return;

    d->sensorChanges = changes;
    update();
}

/**
 * Sets the pen used for drawing the scan line to \a pen
 */
void LeveyJenningsDiagram::setScanLinePen( const QPen& pen )
{
    if( d->scanLinePen == pen )
        return;

    d->scanLinePen = pen;
    update();
}

/**
 * Returns the pen being used for drawing the scan line.
 */
QPen LeveyJenningsDiagram::scanLinePen() const
{
    return d->scanLinePen;
}

/**
 * Returns the SVG file name usef for \a symbol
 */
QString LeveyJenningsDiagram::symbol( Symbol symbol ) const
{
    return d->icons[ symbol ];
}

/**
 * Sets the symbol being used for \a symbol to a SVG file \a filename.
 */
void LeveyJenningsDiagram::setSymbol( Symbol symbol, const QString& filename )
{
    if( d->icons[ symbol ] == filename )
        return;

    delete d->iconRenderer[ symbol ];
    d->iconRenderer[ symbol ] = 0;

    d->icons[ symbol ] = filename;

    update();
}

/**
 * Returns the list of all sensor changes.
 */
QVector< QDateTime > LeveyJenningsDiagram::sensorChanges() const
{
    return d->sensorChanges;
}

/**
 * Sets the expected mean value over all QC values to \a meanValue.
 */
void LeveyJenningsDiagram::setExpectedMeanValue( float meanValue )
{
    if( d->expectedMeanValue == meanValue )
        return;

    d->expectedMeanValue = meanValue;
    d->setYAxisRange();
    update();
}

/**
 * Returns the expected mean values over all QC values.
 */
float LeveyJenningsDiagram::expectedMeanValue() const
{
    return d->expectedMeanValue;
}

/**
 * Sets the expected standard deviaction over all QC values to \a sd.
 */
void LeveyJenningsDiagram::setExpectedStandardDeviation( float sd )
{
    if( d->expectedStandardDeviation == sd )
        return;

    d->expectedStandardDeviation = sd;
    d->setYAxisRange();
    update();
}

/**
 * Returns the expected standard deviation over all QC values.
 */
float LeveyJenningsDiagram::expectedStandardDeviation() const
{
    return d->expectedStandardDeviation;
}

/**
 * Returns the calculated mean values over all QC values.
 */
float LeveyJenningsDiagram::calculatedMeanValue() const
{
    return d->calculatedMeanValue;
}

/**
 * Returns the calculated standard deviation over all QC values.
 */
float LeveyJenningsDiagram::calculatedStandardDeviation() const
{
    return d->calculatedStandardDeviation;
}

void LeveyJenningsDiagram::setModel( QAbstractItemModel* model )
{
    if( this->model() != 0 )
    {
        disconnect( this->model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( modelReset() ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
        disconnect( this->model(), SIGNAL( layoutChanged() ),
                                   this, SLOT( calculateMeanAndStandardDeviation() ) );
    }
    LineDiagram::setModel( model );
    if( this->model() != 0 )
    {
        connect( this->model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( modelReset() ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );
        connect( this->model(), SIGNAL( layoutChanged() ),
                                this, SLOT( calculateMeanAndStandardDeviation() ) );

        calculateMeanAndStandardDeviation();
    }
}

// TODO: This is the 'easy' solution
// evaluate whether this is enough or we need some better one or even boost here
void LeveyJenningsDiagram::calculateMeanAndStandardDeviation() const
{
    QVector< double > values;
    // first fetch all values
    const QAbstractItemModel& m = *model();
    const int rowCount = m.rowCount( rootIndex() );

    for( int row = 0; row < rowCount; ++row )
    {
        const QVariant var = m.data( m.index( row, 1, rootIndex() ) );
        if( !var.isValid() )
            continue;
        const double value = var.toDouble();
        if( ISNAN( value ) )
            continue;
        values << value;
    }

    double sum = 0.0;
    double sumSquares = 0.0;
    KDAB_FOREACH( double value, values )
    {
        sum += value;
        sumSquares += value * value;
    }

    const int N = values.count();

    d->calculatedMeanValue = sum / N;
    d->calculatedStandardDeviation = sqrt( ( static_cast< double >( N ) * sumSquares - sum * sum ) / ( N * ( N - 1 ) ) );
}

// calculates the largest QDate not greater than \a dt.
static QDate floorDay( const QDateTime& dt )
{
    return dt.date();
}

// calculates the smallest QDate not less than \a dt.
static QDate ceilDay( const QDateTime& dt )
{
    QDate result = dt.date();

    if( QDateTime( result, QTime() ) < dt )
        result = result.addDays( 1 );

    return result;
}

// calculates the largest QDateTime like xx:00 not greater than \a dt.
static QDateTime floorHour( const QDateTime& dt )
{
    return QDateTime( dt.date(), QTime( dt.time().hour(), 0 ) );
}

// calculates the smallest QDateTime like xx:00 not less than \a dt.
static QDateTime ceilHour( const QDateTime& dt )
{
    QDateTime result( dt.date(), QTime( dt.time().hour(), 0 ) );

    if( result < dt )
        result = result.addSecs( 3600 );

    return result;
}

/** \reimpl */
const QPair<QPointF, QPointF> LeveyJenningsDiagram::calculateDataBoundaries() const
{
    const double yMin = d->expectedMeanValue - 4 * d->expectedStandardDeviation;
    const double yMax = d->expectedMeanValue + 4 * d->expectedStandardDeviation;

    d->setYAxisRange();

    // rounded down/up to the prev/next midnight (at least that's the default)
    const QPair< QDateTime, QDateTime > range = timeRange();
    const unsigned int minTime = range.first.toTime_t();
    const unsigned int maxTime = range.second.toTime_t();

    const double xMin = minTime / static_cast< double >( 24 * 60 * 60 );
    const double xMax = maxTime / static_cast< double >( 24 * 60 * 60 ) - xMin;

    const QPointF bottomLeft( QPointF( 0, yMin ) );
    const QPointF topRight( QPointF( xMax, yMax ) );

    return QPair< QPointF, QPointF >( bottomLeft, topRight );
}

/**
 * Returns the timerange of the diagram's data.
 */
QPair< QDateTime, QDateTime > LeveyJenningsDiagram::timeRange() const
{
    if( d->timeRange != QPair< QDateTime, QDateTime >() )
        return d->timeRange;

    const QAbstractItemModel& m = *model();
    const int rowCount = m.rowCount( rootIndex() );

    const QDateTime begin = m.data( m.index( 0, 3, rootIndex() ) ).toDateTime();
    const QDateTime end = m.data( m.index( rowCount - 1, 3, rootIndex() ) ).toDateTime();

    if( begin.secsTo( end ) > 86400 )
    {
        // if begin to end is more than 24h
        // round down/up to the prev/next midnight
        const QDate min = floorDay( begin );
        const QDate max = ceilDay( end );
        return QPair< QDateTime, QDateTime >( QDateTime( min ), QDateTime( max ) );
    }
    else if( begin.secsTo( end ) > 3600 )
    {
        // more than 1h: rond down up to the prex/next hour
        // if begin to end is more than 24h
        const QDateTime min = floorHour( begin );
        const QDateTime max = ceilHour( end );
        return QPair< QDateTime, QDateTime >( min, max );
    }
    return QPair< QDateTime, QDateTime >( begin, end );
}

/**
 * Sets the \a timeRange visible on the x axis. Set it to QPair< QDateTime, QDateTime >()
 * to use the default auto calculation.
 */
void LeveyJenningsDiagram::setTimeRange( const QPair< QDateTime, QDateTime >& timeRange )
{
    if( d->timeRange == timeRange )
        return;

    d->timeRange = timeRange;
    update();
}

/**
 * Draws the fluidics pack and sensor changed symbols.
 */
void LeveyJenningsDiagram::drawChanges( PaintContext* ctx )
{
    const unsigned int minTime = timeRange().first.toTime_t();

    KDAB_FOREACH( const QDateTime& dt, d->fluidicsPackChanges )
    {
        const double xValue = ( dt.toTime_t() - minTime ) / static_cast< double >( 24 * 60 * 60 );
        const QPointF point( xValue, 0.0 );
        drawFluidicsPackChangedSymbol( ctx, point );
    }

    KDAB_FOREACH( const QDateTime& dt, d->sensorChanges )
    {
        const double xValue = ( dt.toTime_t() - minTime ) / static_cast< double >( 24 * 60 * 60 );
        const QPointF point( xValue, 0.0 );
        drawSensorChangedSymbol( ctx, point );
    }
}

/** \reimpl */
void LeveyJenningsDiagram::paint( PaintContext* ctx )
{
    d->reverseMapper.clear();

    // note: Not having any data model assigned is no bug
    //       but we can not draw a diagram then either.
    if ( !checkInvariants( true ) ) return;
    if ( !AbstractGrid::isBoundariesValid(dataBoundaries()) ) return;

    QPainter* const painter = ctx->painter();
    const PainterSaver p( painter );
    if( model()->rowCount( rootIndex() ) == 0 || model()->columnCount( rootIndex() ) < 4 )
        return; // nothing to paint for us

    AbstractCoordinatePlane* const plane = ctx->coordinatePlane();
    ctx->setCoordinatePlane( plane->sharedAxisMasterPlane( painter ) );

    const QAbstractItemModel& m = *model();
    const int rowCount = m.rowCount( rootIndex() );

    const unsigned int minTime = timeRange().first.toTime_t();

    painter->setRenderHint( QPainter::Antialiasing, true );

    int prevLot = -1;
    QPointF prevPoint;
    bool hadMissingValue = false;

    for( int row = 0; row < rowCount; ++row )
    {
        const QModelIndex lotIndex = m.index( row, 0, rootIndex() );
        const QModelIndex valueIndex = m.index( row, 1, rootIndex() );
        const QModelIndex okIndex = m.index( row, 2, rootIndex() );
        const QModelIndex timeIndex = m.index( row, 3, rootIndex() );
        const QModelIndex expectedMeanIndex = m.index( row, 4, rootIndex() );
        const QModelIndex expectedSDIndex = m.index( row, 5, rootIndex() );

        painter->setPen( pen( lotIndex ) );

        const int lot = m.data( lotIndex ).toInt();
        double value = m.data( valueIndex ).toDouble();
        const bool ok = m.data( okIndex ).toBool();
        const QDateTime time = m.data( timeIndex ).toDateTime();
        const double xValue = ( time.toTime_t() - minTime ) / static_cast< double >( 24 * 60 * 60 );

        const double expectedMean = m.data( expectedMeanIndex ).toDouble();
        const double expectedSD = m.data( expectedSDIndex ).toDouble();

        QPointF point = ctx->coordinatePlane()->translate( QPointF( xValue, value ) );

        if( static_cast< int >( value ) == 0 )
        {
            hadMissingValue = true;
        }
        else
        {
            if( static_cast< int >( expectedMean ) != 0 && static_cast< int >( expectedSD ) != 0 )
            {
                // this calculates the 'logical' value relative to the expected mean and SD of this point
                value -= expectedMean;
                value /= expectedSD;
                value *= d->expectedStandardDeviation;
                value += d->expectedMeanValue;
                point = ctx->coordinatePlane()->translate( QPointF( xValue, value ) );
            }

            if( prevLot == lot )
            {
                const QPen pen = painter->pen();
                QPen newPen = pen;

                if( hadMissingValue )
                {
                    newPen.setDashPattern( QVector< qreal >() << 4.0 << 4.0 );
                }

                painter->setPen( newPen );
                painter->drawLine( prevPoint, point );
                painter->setPen( pen );
                // d->reverseMapper.addLine( valueIndex.row(), valueIndex.column(), prevPoint, point );
            }
            else if( row > 0 )
            {
                drawLotChangeSymbol( ctx, QPointF( xValue, value ) );
            }

            if( value <= d->expectedMeanValue + 4 * d->expectedStandardDeviation &&
                value >= d->expectedMeanValue - 4 * d->expectedStandardDeviation )
            {
                const QPointF location( xValue, value );
                drawDataPointSymbol( ctx, location, ok );
                d->reverseMapper.addCircle( valueIndex.row(),
                                            valueIndex.column(),
                                            ctx->coordinatePlane()->translate( location ),
                                            iconRect().size() );
            }
            prevLot = lot;
            prevPoint = point;
            hadMissingValue = false;
        }

        const QModelIndex current = selectionModel()->currentIndex();
        if( selectionModel()->rowIntersectsSelection( lotIndex.row(), lotIndex.parent() ) || current.sibling( current.row(), 0 ) == lotIndex )
        {
            const QPen pen = ctx->painter()->pen();
            painter->setPen( d->scanLinePen );
            painter->drawLine( ctx->coordinatePlane()->translate( QPointF( xValue, d->expectedMeanValue - 4 *
                                                                                   d->expectedStandardDeviation ) ),
                               ctx->coordinatePlane()->translate( QPointF( xValue, d->expectedMeanValue + 4 *
                                                                                   d->expectedStandardDeviation ) ) );
            painter->setPen( pen );
        }
    }

    drawChanges( ctx );

    ctx->setCoordinatePlane( plane );
}

/**
 * Draws a data point symbol for the data point at \a pos.
 * @param ok True, when the data point is ok, false otherwise (different symbol)
 * @param ctx The PaintContext being used
 */
void LeveyJenningsDiagram::drawDataPointSymbol( PaintContext* ctx, const QPointF& pos, bool ok )
{
    const Symbol type = ok ? OkDataPoint : NotOkDataPoint;

    QPainter* const painter = ctx->painter();
    const PainterSaver ps( painter );
    const QPointF transPos = ctx->coordinatePlane()->translate( pos ).toPoint();
    painter->translate( transPos );

    painter->setClipping( false );
    iconRenderer( type )->render( painter, iconRect() );
}

/**
 * Draws a lot changed symbol for the data point at \a pos.
 * @param ctx The PaintContext being used
 * \sa lotChangedSymbolPosition
 */
void LeveyJenningsDiagram::drawLotChangeSymbol( PaintContext* ctx, const QPointF& pos )
{
    const QPointF transPos = ctx->coordinatePlane()->translate(
        QPointF( pos.x(), d->lotChangedPosition & Qt::AlignTop ? d->expectedMeanValue +
                                                                 4 * d->expectedStandardDeviation
                                                               : d->expectedMeanValue -
                                                                 4 * d->expectedStandardDeviation ) );


    QPainter* const painter = ctx->painter();
    const PainterSaver ps( painter );
    painter->setClipping( false );
    painter->translate( transPos );
    iconRenderer( LotChanged )->render( painter, iconRect() );
}

/**
 * Draws a sensor changed symbol for the data point at \a pos.
 * @param ctx The PaintContext being used
 * \sa sensorChangedSymbolPosition
 */
void LeveyJenningsDiagram::drawSensorChangedSymbol( PaintContext* ctx, const QPointF& pos )
{
    const QPointF transPos = ctx->coordinatePlane()->translate(
        QPointF( pos.x(), d->sensorChangedPosition & Qt::AlignTop ? d->expectedMeanValue +
                                                                    4 * d->expectedStandardDeviation
                                                                  : d->expectedMeanValue -
                                                                    4 * d->expectedStandardDeviation ) );

    QPainter* const painter = ctx->painter();
    const PainterSaver ps( painter );
    painter->setClipping( false );
    painter->translate( transPos );
    iconRenderer( SensorChanged )->render( painter, iconRect() );
}

/**
 * Draws a fluidics pack changed symbol for the data point at \a pos.
 * @param ctx The PaintContext being used
 * \sa fluidicsPackChangedSymbolPosition
 */
void LeveyJenningsDiagram::drawFluidicsPackChangedSymbol( PaintContext* ctx, const QPointF& pos )
{
    const QPointF transPos = ctx->coordinatePlane()->translate(
        QPointF( pos.x(), d->fluidicsPackChangedPosition & Qt::AlignTop ? d->expectedMeanValue +
                                                                          4 * d->expectedStandardDeviation
                                                                        : d->expectedMeanValue -
                                                                          4 * d->expectedStandardDeviation ) );

    QPainter* const painter = ctx->painter();
    const PainterSaver ps( painter );
    painter->setClipping( false );
    painter->translate( transPos );
    iconRenderer( FluidicsPackChanged )->render( painter, iconRect() );
}

/**
 * Returns the rectangle being used for drawing the icons
 */
QRectF LeveyJenningsDiagram::iconRect() const
{
    const Measure m( 12.5, KDChartEnums::MeasureCalculationModeAuto, KDChartEnums::MeasureOrientationAuto );
    TextAttributes test;
    test.setFontSize( m );
    const QFontMetrics fm( test.calculatedFont( coordinatePlane()->parent(), KDChartEnums::MeasureOrientationAuto ) );
    const qreal height = fm.height() / 1.2;
    return QRectF( -height / 2.0, -height / 2.0, height, height );
}

/**
 * Returns the SVG icon renderer for \a symbol
 */
QSvgRenderer* LeveyJenningsDiagram::iconRenderer( Symbol symbol )
{
    if( d->iconRenderer[ symbol ] == 0 )
        d->iconRenderer[ symbol ] = new QSvgRenderer( d->icons[ symbol ], this );

    return d->iconRenderer[ symbol ];
}
