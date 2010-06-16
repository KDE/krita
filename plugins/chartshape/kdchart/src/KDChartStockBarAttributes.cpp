/****************************************************************************
 ** Copyright (C) 2008 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#include "KDChartStockBarAttributes.h"

#define d d_func()

using namespace KDChart;

class StockBarAttributes::Private {
public:
    Private();

    qreal candlestickWidth;
    qreal tickLength;
};

StockBarAttributes::Private::Private()
    : candlestickWidth( 0.3 )
    , tickLength( 0.15 )
{
}

StockBarAttributes::StockBarAttributes()
    : _d( new Private )
{
}

StockBarAttributes::StockBarAttributes( const StockBarAttributes& r )
    : _d( new Private( *r.d ) )
{
}

StockBarAttributes &StockBarAttributes::operator= ( const StockBarAttributes& r )
{
    if ( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

StockBarAttributes::~StockBarAttributes()
{
    delete _d;
}

/**
  * Sets the width of a candlestick
  *
  * @param width The width of a candlestick
  */
void StockBarAttributes::setCandlestickWidth( qreal width )
{
    d->candlestickWidth = width;
}
/**
  * @return the width of a candlestick
  */
qreal StockBarAttributes::candlestickWidth() const
{
    return d->candlestickWidth;
}

/**
  * Sets the tick length of both the open and close marker
  *
  * @param length the tick length
  */
void StockBarAttributes::setTickLength( qreal length )
{
    d->tickLength = length;
}

/**
  * @return the tick length used for both the open and close marker
  */
qreal StockBarAttributes::tickLength() const
{
    return d->tickLength;
}

bool StockBarAttributes::operator==( const StockBarAttributes& r ) const
{
    return candlestickWidth() == r.candlestickWidth() &&
           tickLength() == r.tickLength();
}
