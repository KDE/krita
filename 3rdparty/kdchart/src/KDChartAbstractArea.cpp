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

#include "KDChartAbstractArea.h"
#include "KDChartAbstractArea_p.h"

#include <QtGlobal>

#include <QPainter>
#include <QRect>

#include <KDABLibFakes>


using namespace KDChart;

#define d (d_func())

AbstractArea::Private::Private() :
    AbstractAreaBase::Private()
{
    // this bloc left empty intentionally
}


AbstractArea::Private::~Private()
{
    // this bloc left empty intentionally
}


AbstractArea::AbstractArea()
    : QObject()
    , KDChart::AbstractAreaBase()
    , KDChart::AbstractLayoutItem()
{
    init();
}

AbstractArea::~AbstractArea()
{
    // this bloc left empty intentionally
}


void AbstractArea::init()
{
    d->amountOfLeftOverlap = 0;
    d->amountOfRightOverlap = 0;
    d->amountOfTopOverlap = 0;
    d->amountOfBottomOverlap = 0;
}


int AbstractArea::leftOverlap( bool doNotRecalculate ) const
{
    // Re-calculate the sizes,
    // so we also get the amountOf..Overlap members set newly:
    if( ! doNotRecalculate )
        sizeHint();
    return d->amountOfLeftOverlap;
}
int AbstractArea::rightOverlap( bool doNotRecalculate ) const
{
    // Re-calculate the sizes,
    // so we also get the amountOf..Overlap members set newly:
    if( ! doNotRecalculate )
        sizeHint();
    return d->amountOfRightOverlap;
}
int AbstractArea::topOverlap( bool doNotRecalculate ) const
{
    // Re-calculate the sizes,
    // so we also get the amountOf..Overlap members set newly:
    if( ! doNotRecalculate )
        sizeHint();
    return d->amountOfTopOverlap;
}
int AbstractArea::bottomOverlap( bool doNotRecalculate ) const
{
    // Re-calculate the sizes,
    // so we also get the amountOf..Overlap members set newly:
    if( ! doNotRecalculate )
        sizeHint();
    return d->amountOfBottomOverlap;
}


void AbstractArea::paintIntoRect( QPainter& painter, const QRect& rect )
{
    const QRect oldGeometry( geometry() );
    if( oldGeometry != rect )
        setGeometry( rect );
    painter.translate( rect.left(), rect.top() );
    paintAll( painter );
    painter.translate( -rect.left(), -rect.top() );
    if( oldGeometry != rect )
        setGeometry( oldGeometry );
}

void AbstractArea::paintAll( QPainter& painter )
{
    // Paint the background and frame
    const QRect overlappingArea( geometry().adjusted(
            -d->amountOfLeftOverlap,
            -d->amountOfTopOverlap,
            d->amountOfRightOverlap,
            d->amountOfBottomOverlap ) );
    paintBackground( painter, overlappingArea );
    paintFrame(      painter, overlappingArea );

    // temporarily adjust the widget size, to be sure all content gets calculated
    // to fit into the inner rectangle
    const QRect oldGeometry( areaGeometry()  );
    QRect inner( innerRect() );
    inner.moveTo(
        oldGeometry.left() + inner.left(),
        oldGeometry.top()  + inner.top() );
    const bool needAdjustGeometry = oldGeometry != inner;
    if( needAdjustGeometry )
        setGeometry( inner );
    paint( &painter );
    if( needAdjustGeometry )
        setGeometry( oldGeometry );
    //qDebug() << "AbstractAreaWidget::paintAll() done.";
}

QRect AbstractArea::areaGeometry() const
{
    return geometry();
}

void AbstractArea::positionHasChanged()
{
    emit positionChanged( this );
}

