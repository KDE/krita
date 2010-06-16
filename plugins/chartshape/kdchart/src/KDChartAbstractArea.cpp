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

#include "KDChartAbstractArea.h"
#include "KDChartAbstractArea_p.h"

#include <qglobal.h>

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

