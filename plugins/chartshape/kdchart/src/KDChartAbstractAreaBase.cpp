/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
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
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDChartAbstractAreaBase.h"
#include "KDChartAbstractAreaBase_p.h"
#include <KDChartBackgroundAttributes.h>
#include <KDChartFrameAttributes.h>
#include <KDChartTextAttributes.h>
#include "KDChartPainterSaver_p.h"
#include "KDChartPrintingParameters.h"
#include <QPainter>

#include <KDABLibFakes>


using namespace KDChart;

AbstractAreaBase::Private::Private() :
    visible( true )
    // PENDING(khz) dockingPointToPadding?, alignToDockingPoint?
{
    init();
}


AbstractAreaBase::Private::~Private() {}


void AbstractAreaBase::Private::init()
{
}




AbstractAreaBase::AbstractAreaBase() :
    _d( new Private() )
{
}

AbstractAreaBase::~AbstractAreaBase()
{
    delete _d; _d = 0;
}


void AbstractAreaBase::init()
{
}


#define d d_func()

bool AbstractAreaBase::compare( const AbstractAreaBase* other )const
{
    if( other == this ) return true;
    if( ! other ){
        //qDebug() << "CartesianAxis::compare() cannot compare to Null pointer";
        return false;
    }
    /*
    qDebug() << "AbstractAreaBase:" << (frameAttributes() == other->frameAttributes())
        << (backgroundAttributes() == other->backgroundAttributes()) << "\n";
    */
    return  (frameAttributes()      == other->frameAttributes()) &&
            (backgroundAttributes() == other->backgroundAttributes());
}

void AbstractAreaBase::alignToReferencePoint( const RelativePosition& position )
{
    Q_UNUSED( position );
    // PENDING(kalle) FIXME
    qWarning( "Sorry, not implemented: void AbstractAreaBase::alignToReferencePoint( const RelativePosition& position )" );
}

void AbstractAreaBase::setFrameAttributes( const FrameAttributes &a )
{
    if( d->frameAttributes == a )
        return;

    d->frameAttributes = a;
    positionHasChanged();
}

FrameAttributes AbstractAreaBase::frameAttributes() const
{
    return d->frameAttributes;
}

void AbstractAreaBase::setBackgroundAttributes( const BackgroundAttributes &a )
{
    if( d->backgroundAttributes == a )
        return;

    d->backgroundAttributes = a;
    positionHasChanged();
}

BackgroundAttributes AbstractAreaBase::backgroundAttributes() const
{
    return d->backgroundAttributes;
}


/* static */
void AbstractAreaBase::paintBackgroundAttributes( QPainter& painter, const QRect& rect,
    const KDChart::BackgroundAttributes& attributes )
{
    if( !attributes.isVisible() ) return;

    /* first draw the brush (may contain a pixmap)*/
    if( Qt::NoBrush != attributes.brush().style() ) {
        KDChart::PainterSaver painterSaver( &painter );
        painter.setPen( Qt::NoPen );
        const QPointF newTopLeft( painter.deviceMatrix().map( rect.topLeft() ) );
        painter.setBrushOrigin( newTopLeft );
        painter.setBrush( attributes.brush() );
        painter.drawRect( rect );
    }
    /* next draw the backPixmap over the brush */
    if( !attributes.pixmap().isNull() &&
        attributes.pixmapMode() != BackgroundAttributes::BackgroundPixmapModeNone ) {
        QPointF ol = rect.topLeft();
        if( BackgroundAttributes::BackgroundPixmapModeCentered == attributes.pixmapMode() )
        {
            ol.setX( rect.center().x() - attributes.pixmap().width() / 2 );
            ol.setY( rect.center().y() - attributes.pixmap().height()/ 2 );
            painter.drawPixmap( ol, attributes.pixmap() );
        } else {
            QMatrix m;
            double zW = (double)rect.width()  / (double)attributes.pixmap().width();
            double zH = (double)rect.height() / (double)attributes.pixmap().height();
            switch( attributes.pixmapMode() ) {
            case BackgroundAttributes::BackgroundPixmapModeScaled:
            {
                double z;
                z = qMin( zW, zH );
                m.scale( z, z );
            }
            break;
            case BackgroundAttributes::BackgroundPixmapModeStretched:
                m.scale( zW, zH );
                break;
            default:
                ; // Cannot happen, previously checked
            }
            QPixmap pm = attributes.pixmap().transformed( m );
            ol.setX( rect.center().x() - pm.width() / 2 );
            ol.setY( rect.center().y() - pm.height()/ 2 );
            painter.drawPixmap( ol, pm );
        }
    }
}

/* static */
void AbstractAreaBase::paintFrameAttributes( QPainter& painter, const QRect& rect,
    const KDChart::FrameAttributes& attributes )
{

    if( !attributes.isVisible() ) return;

    // Note: We set the brush to NoBrush explicitly here.
    //       Otherwise we might get a filled rectangle, so any
    //       previously drawn background would be overwritten by that area.

    const QPen   oldPen(   painter.pen() );
    const QBrush oldBrush( painter.brush() );
    painter.setPen(  PrintingParameters::scalePen( attributes.pen() ) );
    painter.setBrush( Qt::NoBrush );
    painter.drawRect( rect );
    painter.setBrush( oldBrush );
    painter.setPen( oldPen );
}

void AbstractAreaBase::paintBackground( QPainter& painter, const QRect& rect )
{
    Q_ASSERT_X ( d != 0, "AbstractAreaBase::paintBackground()",
                "Private class was not initialized!" );
    paintBackgroundAttributes( painter, rect, d->backgroundAttributes );
}


void AbstractAreaBase::paintFrame( QPainter& painter, const QRect& rect )
{
    Q_ASSERT_X ( d != 0, "AbstractAreaBase::paintFrame()",
                "Private class was not initialized!" );
    paintFrameAttributes( painter, rect, d->frameAttributes );
}


void AbstractAreaBase::getFrameLeadings(int& left, int& top, int& right, int& bottom ) const
{
    if( d && d->frameAttributes.isVisible() ){
        const int padding = qMax( d->frameAttributes.padding(), 0 );
        left   = padding;
        top    = padding;
        right  = padding;
        bottom = padding;
    }else{
        left   = 0;
        top    = 0;
        right  = 0;
        bottom = 0;
    }
}

QRect AbstractAreaBase::innerRect() const
{
    int left;
    int top;
    int right;
    int bottom;
    getFrameLeadings( left, top, right, bottom );
    return
        QRect( QPoint(0,0), areaGeometry().size() )
            .adjusted( left, top, -right, -bottom );
}

void AbstractAreaBase::positionHasChanged()
{
    // this bloc left empty intentionally
}

