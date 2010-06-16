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

#include "KDChartTextArea.h"
#include "KDChartTextArea_p.h"

#include <qglobal.h>

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

