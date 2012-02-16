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

#include "KDChartMarkerAttributes.h"
#include <QColor>
#include <QMap>
#include <QPen>
#include <QSizeF>
#include <QDebug>
#include <QtGlobal>

#include <KDABLibFakes>

using namespace KDChart;

class MarkerAttributes::Private
{
    friend class ::KDChart::MarkerAttributes;
public:
    Private();
private:
    bool visible;
    bool threeD;
    QMap<uint,MarkerStyle> markerStylesMap;
    MarkerStyle markerStyle;
    MarkerSizeMode markerSizeMode;
    QSizeF markerSize;
    QColor markerColor;
    QPen markerPen;
};

MarkerAttributes::Private::Private()
    : visible( false ),
      threeD( false ),
      markerStyle( MarkerSquare ),
      markerSizeMode( AbsoluteSize ),
      markerSize( 10, 10 ),
      markerPen( Qt::black )
{
}


MarkerAttributes::MarkerAttributes()
    : _d( new Private )
{

}

MarkerAttributes::MarkerAttributes( const MarkerAttributes& r )
    : _d( new Private( *r._d ) )
{

}

MarkerAttributes & MarkerAttributes::operator=( const MarkerAttributes& r )
{
    MarkerAttributes copy( r );
    copy.swap( *this );
    return *this;
}

MarkerAttributes::~MarkerAttributes()
{
    delete _d; _d = 0;
}

#define d d_func()

bool MarkerAttributes::operator==( const MarkerAttributes& r ) const
{
    /*
    qDebug() << "MarkerAttributes::operator== finds"
            << "b" << (isVisible() == r.isVisible())
            << "c" << (markerStylesMap() == r.markerStylesMap())
            << "d" << (markerStyle() == r.markerStyle()) << markerStyle() <<r.markerStyle()
            << "e" << (markerSize() == r.markerSize())
            << "f" << (markerColor() == r.markerColor())
            << "g" << (pen() == r.pen())
            << "h" << (markerColor() == r.markerColor()) << markerColor() << r.markerColor();
    */
    return ( isVisible() == r.isVisible() &&
            markerStylesMap() == r.markerStylesMap() &&
            markerStyle() == r.markerStyle() &&
            markerSize() == r.markerSize() &&
            markerColor() == r.markerColor() &&
            pen() == r.pen() );
}



void MarkerAttributes::setVisible( bool visible )
{
    d->visible = visible;
}

bool MarkerAttributes::isVisible() const
{
    return d->visible;
}

void MarkerAttributes::setThreeD( bool value )
{
    d->threeD = value;
}

bool MarkerAttributes::threeD() const
{
    return d->threeD;
}

void MarkerAttributes::setMarkerStylesMap( const MarkerStylesMap & map )
{
    d->markerStylesMap = map;
}

MarkerAttributes::MarkerStylesMap MarkerAttributes::markerStylesMap() const
{
    return d->markerStylesMap;
}

void MarkerAttributes::setMarkerStyle( MarkerStyle style )
{
    d->markerStyle = style;
}

MarkerAttributes::MarkerStyle MarkerAttributes::markerStyle() const
{
    return d->markerStyle;
}

void MarkerAttributes::setMarkerSize( const QSizeF& size )
{
    d->markerSize = size;
}

QSizeF MarkerAttributes::markerSize() const
{
    return d->markerSize;
}

void MarkerAttributes::setMarkerSizeMode( MarkerSizeMode mode )
{
    d->markerSizeMode = mode;
}

MarkerAttributes::MarkerSizeMode MarkerAttributes::markerSizeMode() const
{
    return d->markerSizeMode;
}

void MarkerAttributes::setMarkerColor( const QColor& color )
{
    d->markerColor = color;
}

QColor MarkerAttributes::markerColor() const
{
    return d->markerColor;
}

void MarkerAttributes::setPen( const QPen& pen )
{
    d->markerPen = pen;
}

QPen MarkerAttributes::pen() const
{
    return d->markerPen;
}

#undef d

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<( QDebug dbg, const MarkerAttributes & ma ) {
    return dbg << "KDChart::MarkerAttributes("
               << "visible=" << ma.isVisible()
               << "markerStylesMap=" << ma.markerStylesMap()
               << "markerStyle=" << ma.markerStyle()
               << "markerColor=" << ma.markerColor()
               << "pen=" << ma.pen()
               << ")";
}
#endif

