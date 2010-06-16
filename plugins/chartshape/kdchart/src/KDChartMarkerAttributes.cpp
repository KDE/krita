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

#include "KDChartMarkerAttributes.h"
#include <QColor>
#include <QMap>
#include <QPen>
#include <QSizeF>
#include <QDebug>
#include <qglobal.h>

#include <KDABLibFakes>

using namespace KDChart;

class MarkerAttributes::Private
{
    friend class ::KDChart::MarkerAttributes;
public:
    Private();
private:
    bool visible;
    QMap<uint,MarkerStyle> markerStylesMap;
    MarkerStyle markerStyle;
    QSizeF markerSize;
    QColor markerColor;
    QPen markerPen;
};

MarkerAttributes::Private::Private()
    : visible( false ),
      markerStyle( MarkerSquare ),
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

