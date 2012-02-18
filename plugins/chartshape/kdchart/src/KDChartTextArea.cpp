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

#include "KDChartTextArea.h"
#include "KDChartTextArea_p.h"

#include <QtGlobal>

#include <QPainter>
#include <QRect>

#include <KDABLibFakes>


using namespace KDChart;

TextArea::Private::Private() :
    AbstractAreaBase::Private()
{
    // this bloc left empty intentionally
}


TextArea::Private::~Private()
{
    // this bloc left empty intentionally
}


TextArea::TextArea()
    : QObject()
    , KDChart::AbstractAreaBase()
    , KDChart::TextLayoutItem()
{
    // this bloc left empty intentionally
}

TextArea::~TextArea()
{
    // this bloc left empty intentionally
}


void TextArea::init()
{
    // this bloc left empty intentionally
}

void TextArea::paintIntoRect( QPainter& painter, const QRect& rect )
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

void TextArea::paintAll( QPainter& painter )
{
    // Paint the background and frame
    paintBackground( painter, geometry() );
    paintFrame(      painter, geometry() );

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
    //qDebug() << "TextAreaWidget::paintAll() done.";
}

QRect TextArea::areaGeometry() const
{
    return geometry();
}

void TextArea::positionHasChanged()
{
    emit positionChanged( this );
}

