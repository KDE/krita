/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
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
#include "kdganttdatetimegrid.h"
#include "kdganttdatetimegrid_p.h"

#include "kdganttabstractrowcontroller.h"

#include <QApplication>
#include <QDateTime>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionHeader>
#include <QWidget>
#include <QDebug>

#include <cassert>

using namespace KDGantt;

/*!\class KDGantt::DateTimeGrid
 * \ingroup KDGantt
 *
 * This implementation of AbstractGrid works with QDateTime
 * and shows days and week numbers in the header
 *
 * \todo Extend to work with hours, minutes,... as units too.
 */

// TODO: I think maybe this class should be responsible
// for unit-transformation of the scene...

qreal DateTimeGrid::Private::dateTimeToChartX( const QDateTime& dt ) const
{
    assert( startDateTime.isValid() );
    qreal result = startDateTime.date().daysTo(dt.date())*24.*60.*60.;
    result += startDateTime.time().msecsTo(dt.time())/1000.;
    result *= dayWidth/( 24.*60.*60. );

    return result;
}

QDateTime DateTimeGrid::Private::chartXtoDateTime( qreal x ) const
{
    assert( startDateTime.isValid() );
    int days = static_cast<int>( x/dayWidth );
    qreal secs = x*( 24.*60.*60. )/dayWidth;
    QDateTime dt = startDateTime;
    QDateTime result = dt.addDays( days )
                       .addSecs( static_cast<int>(secs-(days*24.*60.*60.) ) )
                       .addMSecs( qRound( ( secs-static_cast<int>( secs ) )*1000. ) );
    return result;
}

#define d d_func()

DateTimeGrid::DateTimeGrid() : AbstractGrid( new Private )
{
}

DateTimeGrid::~DateTimeGrid()
{
}

/*! \returns The QDateTime used as start date for the grid.
 *
 * The default is three days before the current date.
 */
QDateTime DateTimeGrid::startDateTime() const
{
    return d->startDateTime;
}

/*! \param dt The start date of the grid. It is used as the beginning of the
 * horizontal scrollbar in the view.
 *
 * Emits gridChanged() after the start date has changed.
 */
void DateTimeGrid::setStartDateTime( const QDateTime& dt )
{
    d->startDateTime = dt;
    emit gridChanged();
}

/*! \returns The width in pixels for each day in the grid.
 *
 * The default is 100 pixels.
 */
qreal DateTimeGrid::dayWidth() const
{
    return d->dayWidth;
}

/*! \param w The width in pixels for each day in the grid.
 * Day width is limited to minimum 1.0.
 *
 * The signal gridChanged() is emitted after the day width is changed.
 */
void DateTimeGrid::setDayWidth( qreal w )
{
    qDebug()<<"DateTimeGrid::setDayWidth"<<w;
    d->dayWidth = qMax( w, qreal(0.1) );
    emit gridChanged();
}

/*! \param s The scale to be used to paint the grid.
 *
 * The signal gridChanged() is emitted after the scale has changed.
 * \sa Scale
 */
void DateTimeGrid::setScale( Scale s )
{
        d->scale = s;
        emit gridChanged();
}

/*! \returns The scale used to paint the grid.
 *
 * The default is ScaleAuto, which means the day scale will be used
 * as long as the day width is less or equal to 500.
 * \sa Scale
 */
DateTimeGrid::Scale DateTimeGrid::scale() const
{
        return d->scale;
}

/*! \returns The format used to paint the hours.
 *
 * The default is "hh".
 */
QString DateTimeGrid::hourFormat() const
{
    return d->hourFormat;
}

/*! Set the format used to paint the hours.
 *
 * If @p format is empty, it is not set.
 */
void DateTimeGrid::setHourFormat( const QString &format )
{
    if ( ! format.isEmpty() ) {
        d->hourFormat = format;
    }
}

/*! \param factor The zoom factor
 *
 */
void DateTimeGrid::zoomIn( qreal factor )
{
    setDayWidth( d->dayWidth * factor );
}

/*! \param factor The zoom factor
 *
 */
void DateTimeGrid::zoomOut( qreal factor )
{
    setDayWidth( d->dayWidth * factor );
}

/*! \param ws The start day of the week.
 *
 * A solid line is drawn on the grid to mark the beginning of a new week.
 * Emits gridChanged() after the start day has changed.
 */
void DateTimeGrid::setWeekStart( Qt::DayOfWeek ws )
{
    d->weekStart = ws;
    emit gridChanged();
}

/*! \returns The start day of the week */
Qt::DayOfWeek DateTimeGrid::weekStart() const
{
    return d->weekStart;
}

/*! \param fd A set of days to mark as free in the grid.
 *
 * Free days are filled with the alternate base brush of the
 * palette used by the view.
 * The signal gridChanged() is emitted after the free days are changed.
 */
void DateTimeGrid::setFreeDays( const QSet<Qt::DayOfWeek>& fd )
{
    d->freeDays = fd;
    emit gridChanged();
}

/*! \returns true if row separators are used. */
bool DateTimeGrid::rowSeparators() const
{
    return d->rowSeparators;
}
/*! \param enable Whether to use row separators or not. */
void DateTimeGrid::setRowSeparators( bool enable )
{
    d->rowSeparators = enable;
}

/*! \returns The days marked as free in the grid. */
QSet<Qt::DayOfWeek> DateTimeGrid::freeDays() const
{
    return d->freeDays;
}

/*!
 * \param value The datetime to get the x value for.
 * \returns The x value corresponding to \a value or -1.0 if \a value is not a datetime variant.
 */
qreal DateTimeGrid::mapToChart( const QVariant& value ) const
{
    if ( ! qVariantCanConvert<QDateTime>( value ) ||
         ( value.type() == QVariant::String && qVariantValue<QString>(value).isEmpty() ) )
    {
        return -1.0;
    }
    return d->dateTimeToChartX( value.toDateTime() );
}

/*!
 * \param x The x value get the datetime for.
 * \returns The datetime corresponding to \a x or an invalid datetime if x cannot be mapped.
 */
QVariant DateTimeGrid::mapFromChart( qreal x ) const
{
    return d->chartXtoDateTime( x );
}

/*! \param idx The index to get the Span for.
 * \returns The start and end pixels, in a Span, of the specified index.
 */
Span DateTimeGrid::mapToChart( const QModelIndex& idx ) const
{
    assert( model() );
    if ( !idx.isValid() ) return Span();
    assert( idx.model()==model() );
    const QVariant sv = model()->data( idx, StartTimeRole );
    const QVariant ev = model()->data( idx, EndTimeRole );
    if( qVariantCanConvert<QDateTime>(sv) &&
        qVariantCanConvert<QDateTime>(ev) &&
        !(sv.type() == QVariant::String && qVariantValue<QString>(sv).isEmpty()) &&
        !(ev.type() == QVariant::String && qVariantValue<QString>(ev).isEmpty())
        ) {
      QDateTime st = sv.toDateTime();
      QDateTime et = ev.toDateTime();
      if ( et.isValid() && st.isValid() ) {
        qreal sx = d->dateTimeToChartX( st );
        qreal ex = d->dateTimeToChartX( et )-sx;
        //qDebug() << "DateTimeGrid::mapToChart("<<st<<et<<") => "<< Span( sx, ex );
        return Span( sx, ex);
      }
    }
    // Special case for Events with only a start date
    if( qVariantCanConvert<QDateTime>(sv) && !(sv.type() == QVariant::String && qVariantValue<QString>(sv).isEmpty()) ) {
      QDateTime st = sv.toDateTime();
      if ( st.isValid() ) {
        qreal sx = d->dateTimeToChartX( st );
        return Span( sx, 0 );
      }
    }
    return Span();
}

#if 0
static void debug_print_idx( const QModelIndex& idx )
{
    if ( !idx.isValid() ) {
        qDebug() << "[Invalid]";
        return;
    }
    QDateTime st = idx.data( StartTimeRole ).toDateTime();
    QDateTime et = idx.data( StartTimeRole ).toDateTime();
    qDebug() << idx << "["<<st<<et<<"]";
}
#endif

/*! Maps the supplied Span to QDateTimes, and puts them as start time and
 * end time for the supplied index.
 *
 * \param span The span used to map from.
 * \param idx The index used for setting the start time and end time in the model.
 * \param constraints A list of hard constraints to match against the start time and
 * end time mapped from the span.
 *
 * \returns true if the start time and time was successfully added to the model, or false
 * if unsucessful.
 * Also returns false if any of the constraints isn't satisfied. That is, if the start time of
 * the constrained index is before the end time of the dependency index, or the end time of the
 * constrained index is before the start time of the dependency index.
 */
bool DateTimeGrid::mapFromChart( const Span& span, const QModelIndex& idx,
    const QList<Constraint>& constraints ) const
{
    assert( model() );
    if ( !idx.isValid() ) return false;
    assert( idx.model()==model() );

    QDateTime st = d->chartXtoDateTime(span.start());
    QDateTime et = d->chartXtoDateTime(span.start()+span.length());
    //qDebug() << "DateTimeGrid::mapFromChart("<<span<<") => "<< st << et;
    Q_FOREACH( const Constraint& c, constraints ) {
        if ( c.type() != Constraint::TypeHard || !isSatisfiedConstraint( c )) continue;
        if ( c.startIndex() == idx ) {
            QDateTime tmpst = model()->data( c.endIndex(), StartTimeRole ).toDateTime();
            //qDebug() << tmpst << "<" << et <<"?";
            if ( tmpst<et ) return false;
        } else if ( c.endIndex() == idx ) {
            QDateTime tmpet = model()->data( c.startIndex(), EndTimeRole ).toDateTime();
            //qDebug() << tmpet << ">" << st <<"?";
            if ( tmpet>st ) return false;
        }
    }
    return model()->setData( idx, qVariantFromValue(st), StartTimeRole )
        && model()->setData( idx, qVariantFromValue(et), EndTimeRole );
}

DateTimeGrid::Scale DateTimeGrid::autoScale() const
{
    Scale scale = ScaleDay;
    if ( dayWidth() > 450) {
        scale = ScaleHour;
    } else if (dayWidth() * 30 < 20) {
        scale = ScaleYear;
    } else if (dayWidth() * 7 < 20) {
        scale = ScaleMonth;
    } else if (dayWidth() < 12) {
        scale = ScaleWeek;
    }
    return scale;
}

void DateTimeGrid::paintGrid( QPainter* painter,
                              const QRectF& sceneRect,
                              const QRectF& exposedRect,
                              AbstractRowController* rowController,
                              QWidget* widget )
{
    //qDebug()<<"paintGrid()"<<scale()<<dayWidth();

    paintRowGrid(painter,sceneRect,exposedRect,rowController,widget);

    switch(scale()) {
        case ScaleHour:
            paintHourGrid(painter,sceneRect,exposedRect,rowController,widget);
            break;
        case ScaleDay:
            paintDayGrid(painter,sceneRect,exposedRect,rowController,widget);
            break;
        case ScaleWeek:
            paintWeekGrid(painter,sceneRect,exposedRect,rowController,widget);
            break;
        case ScaleMonth:
            paintMonthGrid(painter,sceneRect,exposedRect,rowController,widget);
            break;
        case ScaleYear:
            paintYearGrid(painter,sceneRect,exposedRect,rowController,widget);
            break;
        case ScaleAuto:
            switch(autoScale()) {
                case ScaleHour:
                    paintHourGrid(painter,sceneRect,exposedRect,rowController,widget);
                    break;
                case ScaleDay:
                    paintDayGrid(painter,sceneRect,exposedRect,rowController,widget);
                    break;
                case ScaleWeek:
                    paintWeekGrid(painter,sceneRect,exposedRect,rowController,widget);
                    break;
                case ScaleMonth:
                    paintMonthGrid(painter,sceneRect,exposedRect,rowController,widget);
                    break;
                case ScaleYear:
                    paintYearGrid(painter,sceneRect,exposedRect,rowController,widget);
                    break;
                case ScaleAuto: // for completeness and remove warning
                    break;
            }
            break;
    }
}

void DateTimeGrid::paintHourGrid( QPainter* painter,
                              const QRectF& /*sceneRect*/,
                              const QRectF& exposedRect,
                              AbstractRowController* /*rowController*/,
                              QWidget* /*widget*/ )
{
    //qDebug()<<"paintHourGrid()"<<scale()<<dayWidth();
    QDateTime dt = d->chartXtoDateTime( exposedRect.left() );
    dt.setTime( QTime( dt.time().hour(), 0, 0, 0 ) );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right(); dt = dt.addSecs( 60*60 ),x=d->dateTimeToChartX( dt ) ) {
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        if ( dt.time() == QTime( 23, 0, 0 ) ) {
            pen.setStyle( Qt::SolidLine );
        } else {
            pen.setStyle( Qt::DashLine );
        }
        painter->setPen( pen );
        x += ( dayWidth() / 24.0 ) - 1;
        painter->drawLine( QPointF( x, exposedRect.top() ), QPointF( x, exposedRect.bottom() ) );
    }
}

void DateTimeGrid::paintDayGrid( QPainter* painter,
                                  const QRectF& /*sceneRect*/,
                                  const QRectF& exposedRect,
                                  AbstractRowController* /*rowController*/,
                                  QWidget* widget )
{
    //qDebug()<<"paintDayGrid()"<<scale()<<dayWidth();
    QDateTime dt = d->chartXtoDateTime( exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right(); dt = dt.addDays( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        if ( dt.date().addDays( 1 ).dayOfWeek() == d->weekStart ) {
            pen.setStyle( Qt::SolidLine );
        } else {
            pen.setStyle( Qt::DashLine );
        }
        painter->setPen( pen );
        paintFreeDay( painter, x, exposedRect, dt.date(), widget );
        x += dayWidth() - 1;
        painter->drawLine( QPointF( x, exposedRect.top() ), QPointF( x, exposedRect.bottom() ) );
    }
}

void DateTimeGrid::paintWeekGrid( QPainter* painter,
                                  const QRectF& /*sceneRect*/,
                                  const QRectF& exposedRect,
                                  AbstractRowController* /*rowController*/,
                                  QWidget* widget )
{
    //qDebug()<<"paintWeekGrid()"<<scale()<<dayWidth();
    QDateTime dt = d->chartXtoDateTime( exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    while ( dt.date().dayOfWeek() != d->weekStart ) dt = dt.addDays( -1 );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right(); dt = dt.addDays( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        if ( dt.date().addDays( 1 ).day() == 1 ) {
            pen.setStyle( Qt::SolidLine );
        } else if ( dt.date().addDays( 1 ).dayOfWeek() == d->weekStart ) {
            pen.setStyle( Qt::DashLine );
        } else {
            pen.setStyle( Qt::NoPen );
        }
        painter->setPen( pen );
        paintFreeDay( painter, x, exposedRect, dt.date(), widget );
        if ( pen.style() != Qt::NoPen ) {
            //qDebug()<<"paintWeekGrid()"<<dt;
            x += dayWidth() - 1;
            painter->drawLine( QPointF( x, exposedRect.top() ), QPointF( x, exposedRect.bottom() ) );
        }
    }
}

void DateTimeGrid::paintMonthGrid( QPainter* painter,
                                  const QRectF& /*sceneRect*/,
                                  const QRectF& exposedRect,
                                  AbstractRowController* /*rowController*/,
                                  QWidget* widget )
{
    //qDebug()<<"paintMonthGrid()"<<scale()<<dayWidth();

    // Paint a dashed line between each month and a solid line between the years.
    QDateTime dt = d->chartXtoDateTime( exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    dt = dt.addDays( 1 - dt.date().day() );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right(); dt = dt.addDays( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        if ( dt.date().addMonths( 1 ).month() == 1 && dt.date().addDays( 1 ).day() == 1 ) {
            pen.setStyle( Qt::SolidLine );
        } else if ( dt.date().addDays( 1 ).day() == 1 ) {
            pen.setStyle( Qt::DashLine );
        } else {
            pen.setStyle( Qt::NoPen );
        }
        painter->setPen( pen );
        paintFreeDay( painter, x, exposedRect, dt.date(), widget );
        if ( pen.style() != Qt::NoPen ) {
            //qDebug()<<"paintMonthGrid()"<<dt;
            x += dayWidth() - 1;
            painter->drawLine( QPointF( x, exposedRect.top() ), QPointF( x, exposedRect.bottom() ) );
        }
    }
}

void DateTimeGrid::paintYearGrid( QPainter* painter,
                                  const QRectF& sceneRect,
                                  const QRectF& exposedRect,
                                  AbstractRowController* rowController,
                                  QWidget* widget )
{
    //qDebug()<<"paintYearGrid()"<<scale()<<dayWidth();

    Q_UNUSED(sceneRect);
    Q_UNUSED(rowController)

    // Paint a dashed line between each quarter and a solid line between the years.
    QDateTime dt = d->chartXtoDateTime( exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    dt = dt.addDays( 1 - dt.date().day() );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right(); dt = dt.addDays( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        if ( dt.date().addMonths( 1 ).month() == 1 && dt.date().addDays( 1 ).day() == 1 ) {
            // Solid line at day 1 of each year.
            pen.setStyle( Qt::SolidLine );
        } else if ( dt.date().addMonths( 1 ).month() % 3 == 1 && dt.date().addDays( 1 ).day() == 1 ) {
            // Dashed line between the quarters
            pen.setStyle( Qt::DashLine );
        } else {
            pen.setStyle( Qt::NoPen );
        }
        painter->setPen( pen );
        paintFreeDay( painter, x, exposedRect, dt.date(), widget );
        if ( pen.style() != Qt::NoPen ) {
            //qDebug()<<"paintYearGrid()"<<dt;
            x += dayWidth() - 1;
            painter->drawLine( QPointF( x, exposedRect.top() ), QPointF( x, exposedRect.bottom() ) );
        }
    }
}

void DateTimeGrid::paintFreeDay( QPainter* painter, qreal x, const QRectF& exposedRect, const QDate &dt, QWidget* widget )
{
    if ( d->freeDays.contains( static_cast<Qt::DayOfWeek>( dt.dayOfWeek() ) ) ) {
        //FIXME We now use same color for alternating rows and free days
        painter->setBrush( widget ? widget->palette().alternateBase() : QApplication::palette().alternateBase() );
        painter->fillRect( QRectF( x, exposedRect.top(), dayWidth(), exposedRect.height() ), painter->brush() );
    }
}

void DateTimeGrid::paintRowGrid( QPainter* painter,
                                  const QRectF& /*sceneRect*/,
                                  const QRectF& exposedRect,
                                  AbstractRowController* rowController,
                                  QWidget* /*widget*/ )
{
    if ( rowController && rowSeparators() ) {
        // First draw the rows
        QPen pen = painter->pen();
        pen.setBrush( QApplication::palette().dark() );
        pen.setStyle( Qt::DashLine );
        painter->setPen( pen );
        QModelIndex idx = rowController->indexAt( qRound( exposedRect.top() ) );
        qreal y = 0;
        while ( y < exposedRect.bottom() && idx.isValid() ) {
            const Span s = rowController->rowGeometry( idx );
            y = s.start()+s.length();
            //painter->drawLine( QPointF( sceneRect.left(), y ), QPointF( sceneRect.right(), y ) );
            // Is alternating background better?
            if ( idx.row()%2 ) painter->fillRect( QRectF( exposedRect.x(), s.start(), exposedRect.width(), s.length() ), QApplication::palette().alternateBase() );
            idx =  rowController->indexBelow( idx );
        }
    }
}

void DateTimeGrid::render( QPainter* painter,  const QRectF &target, const QRectF& headerRect, const QRectF& exposedRect, QWidget *widget, Qt::AspectRatioMode aspectRatioMode )
{
    painter->save();

    qreal xratio = target.width() / exposedRect.width();
    qreal yratio = target.height() / exposedRect.height();
    //qDebug()<<"QGraphicsScene::render()"<<xratio<<yratio;
    // Scale according to the aspect ratio mode.
    switch (aspectRatioMode) {
        case Qt::KeepAspectRatio:
            xratio = yratio = qMin(xratio, yratio);
            break;
        case Qt::KeepAspectRatioByExpanding:
            xratio = yratio = qMax(xratio, yratio);
            break;
        case Qt::IgnoreAspectRatio:
            break;
    }

    //qDebug()<<"DateTimeGrid::render()"<<"target="<<target<<"exposedRect="<<exposedRect<<"xr="<<xratio<<"yr="<<yratio;

    painter->setClipRect( target );
    QTransform painterTransform;
    painterTransform *= QTransform()
            .translate(target.left(), target.top())
            .scale(xratio, yratio)
            .translate(-exposedRect.left(), -exposedRect.top());
    painter->setWorldTransform(painterTransform, true);

    paintHeader( painter, headerRect, exposedRect, 0.0, widget );
    painter->restore();
}

void DateTimeGrid::paintHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                qreal offset, QWidget* widget )
{
        switch(scale()) {
        case ScaleHour: paintHourScaleHeader(painter,headerRect,exposedRect,offset,widget); break;
        case ScaleDay: paintDayScaleHeader(painter,headerRect,exposedRect,offset,widget); break;
        case ScaleWeek: paintWeekScaleHeader(painter,headerRect,exposedRect,offset,widget); break;
        case ScaleMonth: 
        case ScaleYear: paintMonthScaleHeader(painter,scale(),headerRect,exposedRect,offset,widget); break;
        case ScaleAuto:
            Scale autoResult = autoScale();
            switch(autoResult) {
                case ScaleHour:
                    paintHourScaleHeader(painter,headerRect,exposedRect,offset,widget);
                    break;
                case ScaleDay:
                    paintDayScaleHeader(painter,headerRect,exposedRect,offset,widget);
                    break;
                case ScaleWeek:
                    paintWeekScaleHeader(painter,headerRect,exposedRect,offset,widget);
                    break;
                case ScaleMonth:
                case ScaleYear:
                    paintMonthScaleHeader(painter,autoResult,headerRect,exposedRect,offset,widget);
                    break;
                case ScaleAuto: // For completeness and remove warning
                    break;
            }
            break;
        }
}

/*! Paints the hour scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintHourScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                qreal offset, QWidget* widget )
{
    QStyle* style = widget?widget->style():QApplication::style();

    // Paint a section for each hour
    QDateTime dt = d->chartXtoDateTime( offset+exposedRect.left() );
    dt.setTime( QTime( dt.time().hour(), 0, 0, 0 ) );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right()+offset;
          dt = dt.addSecs( 60*60 /*1 hour*/ ),x=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x-offset, headerRect.top()+headerRect.height()/2., dayWidth()/24., headerRect.height()/2. ).toRect();
        opt.text = dt.time().toString( d->hourFormat );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }

    dt = d->chartXtoDateTime( offset+exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    // Paint a section for each day
    for ( qreal x2 = d->dateTimeToChartX( dt ); x2 < exposedRect.right()+offset;
          dt = dt.addDays( 1 ),x2=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x2-offset, headerRect.top(), dayWidth(), headerRect.height()/2. ).toRect();
        opt.text = QDate::longDayName( dt.date().dayOfWeek() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }
}

/*! Paints the day scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintDayScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                qreal offset, QWidget* widget )
{
    // For starters, support only the regular tab-per-day look
    QStyle* style = widget?widget->style():QApplication::style();

    // Paint a section for each day
    QDateTime dt = d->chartXtoDateTime( offset+exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right()+offset;
          dt = dt.addDays( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x-offset, headerRect.top()+headerRect.height()/2., dayWidth(), headerRect.height()/2. ).toRect();
        opt.text = dt.toString( QString::fromAscii( "ddd" ) ).left( 1 );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }

    dt = d->chartXtoDateTime( offset+exposedRect.left() );
    dt.setTime( QTime( 0, 0, 0, 0 ) );
    // Go backwards until start of week
    while ( dt.date().dayOfWeek() != d->weekStart ) dt = dt.addDays( -1 );
    // Paint a section for each week
    for ( qreal x2 = d->dateTimeToChartX( dt ); x2 < exposedRect.right()+offset;
          dt = dt.addDays( 7 ),x2=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x2-offset, headerRect.top(), dayWidth()*7., headerRect.height()/2. ).toRect();
        opt.text = QString::number( dt.date().weekNumber() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }
}

/*! Paints the week scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintWeekScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                        qreal offset, QWidget* widget )
{
    QStyle* style = widget?widget->style():QApplication::style();

    // Paint a section for each week
    QDateTime sdt = d->chartXtoDateTime( offset+exposedRect.left() );
    sdt.setTime( QTime( 0, 0, 0, 0 ) );
    // Go backwards until start of week
    while ( sdt.date().dayOfWeek() != d->weekStart ) sdt = sdt.addDays( -1 );
    QDateTime dt = sdt;
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right()+offset;
            dt = dt.addDays( 7 ),x=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x-offset, headerRect.top()+headerRect.height()/2., dayWidth()*7, headerRect.height()/2. ).toRect();
        opt.text = QString::number( dt.date().weekNumber() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }

    // Paint a section for each month
    dt = sdt;
    for ( qreal x2 = d->dateTimeToChartX( dt ); x2 < exposedRect.right()+offset; x2=d->dateTimeToChartX( dt ) ) {
        //qDebug()<<"paintWeekScaleHeader()"<<dt;
        QDate next = dt.date().addMonths( 1 );
        next = next.addDays( 1 - next.day() );

        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x2-offset, headerRect.top(), dayWidth()*dt.date().daysTo( next ), headerRect.height()/2. ).toRect();
        opt.text = QDate::longMonthName( dt.date().month() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }

        dt.setDate( next );
    }
}

/*! Paints the month scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintMonthScaleHeader( QPainter* painter, Scale scale,
                                          const QRectF& headerRect, const QRectF& exposedRect,
                                          qreal offset, QWidget* widget )
{
    QStyle* style = widget?widget->style():QApplication::style();

    // Paint a section for each month
    QDateTime sdt = d->chartXtoDateTime( offset+exposedRect.left() );
    sdt.setTime( QTime( 0, 0, 0, 0 ) );
    sdt = sdt.addDays( 1 - sdt.date().day() );
    QDateTime dt = sdt;
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right()+offset;
            dt = dt.addMonths( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x-offset, headerRect.top()+headerRect.height()/2., dayWidth()*dt.date().daysInMonth(), headerRect.height()/2. ).toRect();
        QString monthName = QDate::shortMonthName( dt.date().month() );
        if (scale == ScaleYear)
            opt.text = monthName.left(1);
        else
            opt.text = monthName;
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }

    // Paint a section for each year
    dt = sdt;
    for ( qreal x2 = d->dateTimeToChartX( dt ); x2 < exposedRect.right()+offset; x2=d->dateTimeToChartX( dt ) ) {
        //qDebug()<<"paintMonthScaleHeader()"<<dt;
        QDate next = dt.date().addYears( 1 );
        next = next.addMonths( 1 - next.month() );

        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x2-offset, headerRect.top(), dayWidth()*dt.date().daysTo( next ), headerRect.height()/2. ).toRect();
        opt.text = QString::number( dt.date().year() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }

        dt.setDate( next );
    }
}

#if 0
/*! Paints the year scale header.
 * \sa paintHeader()
 */
void DateTimeGrid::paintYearScaleHeader( QPainter* painter,  const QRectF& headerRect, const QRectF& exposedRect,
                                        qreal offset, QWidget* widget )
{
#if 1
    // FIXME: Improve this with e.g. single letter months
    paintMonthScaleHeader( painter, headerRect, exposedRect, offset, widget );
#else
    QStyle* style = widget?widget->style():QApplication::style();

    // Paint a section for each month
    QDateTime sdt = d->chartXtoDateTime( offset+exposedRect.left() );
    sdt.setTime( QTime( 0, 0, 0, 0 ) );
    sdt = sdt.addDays( 1 - sdt.date().day() );
    QDateTime dt = sdt;
    for ( qreal x = d->dateTimeToChartX( dt ); x < exposedRect.right()+offset;
            dt = dt.addMonths( 1 ),x=d->dateTimeToChartX( dt ) ) {
        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x-offset, headerRect.top()+headerRect.height()/2., dayWidth()*dt.date().daysInMonth(), headerRect.height()/2. ).toRect();
        opt.text = QDate::shortMonthName( dt.date().month() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }
    }

    // Paint a section for each year
    dt = sdt;
    for ( qreal x2 = d->dateTimeToChartX( dt ); x2 < exposedRect.right()+offset; x2=d->dateTimeToChartX( dt ) ) {
        //qDebug()<<"paintMonthScaleHeader()"<<dt;
        QDate next = dt.date().addYears( 1 );
        next = next.addMonths( 1 - next.month() );

        QStyleOptionHeader opt;
        opt.init( widget );
        opt.rect = QRectF( x2-offset, headerRect.top(), dayWidth()*dt.date().daysTo( next ), headerRect.height()/2. ).toRect();
        opt.text = QString::number( dt.date().year() );
        opt.textAlignment = Qt::AlignCenter;
        // NOTE:CE_Header does not honor clipRegion(), so we do the CE_Header logic here
        style->drawControl( QStyle::CE_HeaderSection, &opt, painter, 0 ); //NOTE: using widget will loose background when printing
        QStyleOptionHeader subopt = opt;
        subopt.rect = style->subElementRect( QStyle::SE_HeaderLabel, &opt, widget );
        if ( subopt.rect.isValid() ) {
            style->drawControl( QStyle::CE_HeaderLabel, &subopt, painter, widget );
        }

        dt.setDate( next );
    }
#endif
}
#endif


#undef d

#ifndef KDAB_NO_UNIT_TESTS

#include <QStandardItemModel>
#include "unittest/test.h"

namespace {
    std::ostream& operator<<( std::ostream& os, const QDateTime& dt )
    {
        os << dt.toString().toStdString();
        return os;
    }
}

KDAB_SCOPED_UNITTEST_SIMPLE( KDGantt, DateTimeGrid, "test" ) {
    QStandardItemModel model( 3, 2 );
    DateTimeGrid grid;
    QDateTime dt = QDateTime::currentDateTime();
    grid.setModel( &model );
    grid.setStartDateTime( dt.addDays( -10 ) );

    model.setData( model.index( 0, 0 ), dt,               StartTimeRole );
    model.setData( model.index( 0, 0 ), dt.addDays( 17 ), EndTimeRole );

    model.setData( model.index( 2, 0 ), dt.addDays( 18 ), StartTimeRole );
    model.setData( model.index( 2, 0 ), dt.addDays( 19 ), EndTimeRole );

    Span s = grid.mapToChart( model.index( 0, 0 ) );
    //qDebug() << "span="<<s;

    assertTrue( s.start()>0 );
    assertTrue( s.length()>0 );

    grid.mapFromChart( s, model.index( 1, 0 ) );

    QDateTime s1 = model.data( model.index( 0, 0 ), StartTimeRole ).toDateTime();
    QDateTime e1 = model.data( model.index( 0, 0 ), EndTimeRole ).toDateTime();
    QDateTime s2 = model.data( model.index( 1, 0 ), StartTimeRole ).toDateTime();
    QDateTime e2 = model.data( model.index( 1, 0 ), EndTimeRole ).toDateTime();

    assertTrue( s1.isValid() );
    assertTrue( e1.isValid() );
    assertTrue( s2.isValid() );
    assertTrue( e2.isValid() );

    assertEqual( s1, s2 );
    assertEqual( e1, e2 );

    assertTrue( grid.isSatisfiedConstraint( Constraint( model.index( 0, 0 ), model.index( 2, 0 ) ) ) );
    assertFalse( grid.isSatisfiedConstraint( Constraint( model.index( 2, 0 ), model.index( 0, 0 ) ) ) );

    s = grid.mapToChart( model.index( 0, 0 ) );
    s.setEnd( s.end()+100000. );
    bool rc = grid.mapFromChart( s, model.index( 0, 0 ) );
    assertTrue( rc );
    assertEqual( s1, model.data( model.index( 0, 0 ), StartTimeRole ).toDateTime() );
    Span newspan = grid.mapToChart( model.index( 0, 0 ) );
    assertEqual( newspan.start(), s.start() );
    assertEqual( newspan.length(), s.length() );

    {
        QDateTime startDateTime = QDateTime::currentDateTime();
        qreal dayWidth = 100;
        QDate currentDate = QDate::currentDate();
        QDateTime dt( QDate(currentDate.year(), 1, 1),  QTime( 0, 0, 0, 0 ) );
        assert( dt.isValid() );
        qreal result = startDateTime.date().daysTo(dt.date())*24.*60.*60.;
        result += startDateTime.time().msecsTo(dt.time())/1000.;
        result *= dayWidth/( 24.*60.*60. );

        int days = static_cast<int>( result/dayWidth );
        qreal secs = result*( 24.*60.*60. )/dayWidth;
        QDateTime dt2 = startDateTime;
        QDateTime result2 = dt2.addDays( days ).addSecs( static_cast<int>(secs-(days*24.*60.*60.) ) ).addMSecs( qRound( ( secs-static_cast<int>( secs ) )*1000. ) );

        assertEqual( dt, result2 );
    }
}

#endif /* KDAB_NO_UNIT_TESTS */

#include "moc_kdganttdatetimegrid.cpp"
