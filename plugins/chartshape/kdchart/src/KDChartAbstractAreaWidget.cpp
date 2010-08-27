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

#include "KDChartAbstractAreaWidget.h"
#include "KDChartAbstractAreaWidget_p.h"

#include <KDABLibFakes>


using namespace KDChart;


AbstractAreaWidget::Private::Private()
{
    // this block left empty intentionally
}

AbstractAreaWidget::Private::~Private()
{
    // this block left empty intentionally
}


void AbstractAreaWidget::Private::resizeLayout(
    AbstractAreaWidget* widget, const QSize& size )
{
    if( size == currentLayoutSize ) return;

    currentLayoutSize = size;

    // Now we call adjust the size, for the inner parts of the widget.
    int left;
    int top;
    int right;
    int bottom;
    widget->getFrameLeadings( left, top, right, bottom );
    const QSize innerSize( size.width() - left - right,
                           size.height() - top - bottom );
    // With this adjusted size we call the real resizeLayout method,
    // which normally will call resizeLayout( size ) in the derived class
    // - which in turn is the place to resize the layout member variable
    // of that class.
    widget->resizeLayout( innerSize );
}


AbstractAreaWidget::AbstractAreaWidget( QWidget* parent )
    : QWidget( parent )
    , AbstractAreaBase( new Private() )
{
    init();
}

AbstractAreaWidget::~AbstractAreaWidget()
{
    // this block left empty intentionally
}

void AbstractAreaWidget::init()
{
    // this block left empty intentionally
}

void AbstractAreaWidget::needSizeHint()
{
    // this block left empty intentionally
}

#define d d_func()

void AbstractAreaWidget::resizeLayout( const QSize& size )
{
    Q_UNUSED( size );
    // this block left empty intentionally
}

void AbstractAreaWidget::paintEvent( QPaintEvent* event )
{
    Q_UNUSED( event );
    QPainter painter( this );
    if( size() != d->currentLayoutSize ){
        d->resizeLayout( this, size() );
    }
    paintAll( painter );
}

void AbstractAreaWidget::paintIntoRect( QPainter& painter, const QRect& rect )
{
    //qDebug() << "AbstractAreaWidget::paintIntoRect() called rect=" << rect;

    if( rect.isEmpty() ) return;

    d->resizeLayout( this, rect.size() );

    const QPoint translation( rect.topLeft() );
    painter.translate( translation );
    paintAll( painter );
    painter.translate( -translation.x(), -translation.y() );

/*
    // make sure, the contents of the widget have been set up,
    // so we get a useful geometry:
    needSizeHint();

    const QRect oldGeometry( layout()->geometry() );
    const QRect newGeo( QPoint(0,0), rect.size() );
    const bool mustChangeGeo = layout() && oldGeometry != newGeo;
    if( mustChangeGeo )
        layout()->setGeometry( newGeo );
    painter.translate( rect.left(), rect.top() );
    paintAll( painter );
    painter.translate( -rect.left(), -rect.top() );
    if( mustChangeGeo )
        layout()->setGeometry( oldGeometry );
*/
}

void AbstractAreaWidget::forceRebuild()
{
    //bloc left empty intentionally
}

void AbstractAreaWidget::paintAll( QPainter& painter )
{
    //qDebug() << "AbstractAreaWidget::paintAll() called";

    // Paint the background and frame
    paintBackground( painter, QRect(QPoint(0, 0), size() ) );
    paintFrame(      painter, QRect(QPoint(0, 0), size() ) );

/*
    we do not call setContentsMargins() now,
    but we call resizeLayout() whenever the size or the frame has changed

    // adjust the widget's content margins,
    // to be sure all content gets calculated
    // to fit into the inner rectangle
    const QRect oldGeometry( areaGeometry()  );
    const QRect inner( innerRect() );
    //qDebug() << "areaGeometry():" << oldGeometry
    //         << "  contentsRect():" << contentsRect() << "  inner:" << inner;
    if( contentsRect() != inner ){
        //qDebug() << "old contentsRect():" << contentsRect() << "  new innerRect:" << inner;
        setContentsMargins(
            inner.left(),
            inner.top(),
            oldGeometry.width() -inner.width()-1,
            oldGeometry.height()-inner.height()-1 );
        //forceRebuild();
    }
*/
    int left;
    int top;
    int right;
    int bottom;
    getFrameLeadings( left, top, right, bottom );
    const QPoint translation( left, top );
    painter.translate( translation );
    paint( &painter );
    painter.translate( -translation.x(), -translation.y() );
     //qDebug() << "AbstractAreaWidget::paintAll() done.";
}

QRect AbstractAreaWidget::areaGeometry() const
{
    return geometry();
}

void AbstractAreaWidget::positionHasChanged()
{
    emit positionChanged( this );
}
/*
void AbstractAreaWidget::setGeometry( const QRect & rect )
{
    qDebug() << "AbstractAreaWidget::setGeometry("<< rect << ") called";
    const bool bChanged = rect != geometry();
    QWidget::setGeometry( rect );
    if( bChanged )
        forceRebuild();
}
*/
