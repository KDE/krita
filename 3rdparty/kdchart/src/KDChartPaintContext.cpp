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
