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

#include "KDChartPaintContext.h"
#include "KDChartAbstractCoordinatePlane.h"

#include <QRectF>
#include <QPainter>

#include <KDABLibFakes>

using namespace KDChart;

#define d (d_func())

class PaintContext::Private {

public:
    QPainter* painter;
    QRectF rect;
    AbstractCoordinatePlane* plane;

    Private()
        : painter( 0 )
        , plane ( 0 )
    {}
};

PaintContext::PaintContext()
    : _d ( new Private() )
{
}

PaintContext::~PaintContext()
{
    delete _d;
}

const QRectF PaintContext::rectangle() const
{
    return d->rect;
}

void PaintContext::setRectangle ( const QRectF& rect )
{
    d->rect = rect;
}

QPainter* PaintContext::painter() const
{
    return d->painter;
}

void PaintContext::setPainter( QPainter* painter )
{
    d->painter = painter;
}

AbstractCoordinatePlane* PaintContext::coordinatePlane() const
{
    return d->plane;
}

void PaintContext::setCoordinatePlane( AbstractCoordinatePlane* plane)
{
    d->plane = plane;
}
