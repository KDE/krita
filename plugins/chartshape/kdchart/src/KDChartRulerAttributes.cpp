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

#include "KDChartRulerAttributes.h"

#include <limits>

#include <QPen>
#include <QDebug>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

class RulerAttributes::Private
{
    friend class RulerAttributes;
public:
    Private();
private:
    QPen tickMarkPen;
    QPen majorTickMarkPen;
    QPen minorTickMarkPen;
    
    bool majorTickMarkPenIsSet;
    bool minorTickMarkPenIsSet;

    bool showMajorTickMarks;
    bool showMinorTickMarks;
    
    RulerAttributes::TickMarkerPensMap customTickMarkPens;
};

RulerAttributes::Private::Private()
    : tickMarkPen( QColor( 0x00, 0x00, 0x00 ) )
    , majorTickMarkPen( QColor( 0x00, 0x00, 0x00 ) )
    , minorTickMarkPen( QColor( 0x00, 0x00, 0x00 ) )
{
	tickMarkPen.setCapStyle( Qt::FlatCap );
	majorTickMarkPen.setCapStyle( Qt::FlatCap );
	minorTickMarkPen.setCapStyle( Qt::FlatCap );
	
	majorTickMarkPenIsSet = false;
	minorTickMarkPenIsSet = false;

        showMajorTickMarks = true;
        showMinorTickMarks = true;
}


RulerAttributes::RulerAttributes()
    : _d( new Private() )
{
    // this bloc left empty intentionally
}

RulerAttributes::RulerAttributes( const RulerAttributes& r )
    : _d( new Private( *r.d ) )
{
}

void RulerAttributes::setTickMarkPen( const QPen& pen )
{
	d->tickMarkPen = pen;
}

QPen RulerAttributes::tickMarkPen() const
{
	return d->tickMarkPen;
}

void RulerAttributes::setMajorTickMarkPen( const QPen& pen )
{
	d->majorTickMarkPen = pen;
	d->majorTickMarkPenIsSet = true;
}

bool RulerAttributes::majorTickMarkPenIsSet() const
{
    return d->majorTickMarkPenIsSet;
}

QPen RulerAttributes::majorTickMarkPen() const
{
	return d->majorTickMarkPenIsSet ? d->majorTickMarkPen : d->tickMarkPen;
}

void RulerAttributes::setMinorTickMarkPen( const QPen& pen )
{
	d->minorTickMarkPen = pen;
	d->minorTickMarkPenIsSet = true;
}

bool RulerAttributes::minorTickMarkPenIsSet() const
{
    return d->minorTickMarkPenIsSet;
}

QPen RulerAttributes::minorTickMarkPen() const
{
	return d->minorTickMarkPenIsSet ? d->minorTickMarkPen : d->tickMarkPen;
}

void RulerAttributes::setTickMarkPen( qreal value, const QPen& pen )
{
	if ( !d->customTickMarkPens.contains( value ) )
		d->customTickMarkPens.insert( value, pen );
}

QPen RulerAttributes::tickMarkPen( qreal value ) const
{
	QMapIterator<qreal, QPen> it( d->customTickMarkPens );
	while( it.hasNext() ) {
		it.next();
		if ( qAbs( value - it.key() ) < std::numeric_limits< float >::epsilon() )
			return it.value();
	}
	return d->tickMarkPen;
}

RulerAttributes::TickMarkerPensMap RulerAttributes::tickMarkPens() const
{
    return d->customTickMarkPens;
}

bool RulerAttributes::hasTickMarkPenAt( qreal value ) const
{
	QMapIterator<qreal, QPen> it( d->customTickMarkPens );
	while( it.hasNext() ) {
		it.next();
		if ( qAbs( value - it.key() ) < std::numeric_limits< float >::epsilon() )
			return true;
	}
	return false;
}

void RulerAttributes::setTickMarkColor( const QColor& color )
{
	d->tickMarkPen.setColor( color );
}

QColor RulerAttributes::tickMarkColor() const
{
	return d->tickMarkPen.color();
}

void RulerAttributes::setShowMajorTickMarks( bool show )
{
    d->showMajorTickMarks = show;
}

bool RulerAttributes::showMajorTickMarks() const
{
    return d->showMajorTickMarks;
}

void RulerAttributes::setShowMinorTickMarks( bool show )
{
    d->showMinorTickMarks = show;
}

bool RulerAttributes::showMinorTickMarks() const
{
    return d->showMinorTickMarks;
}

RulerAttributes & RulerAttributes::operator=( const RulerAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

RulerAttributes::~RulerAttributes()
{
    delete _d; _d = 0;
}


bool RulerAttributes::operator == ( const RulerAttributes& r ) const
{
    bool isEqual =
        tickMarkPen()      == r.tickMarkPen() &&
        majorTickMarkPen() == r.majorTickMarkPen() &&
        minorTickMarkPen() == r.minorTickMarkPen();
    if( isEqual ) {
        QMapIterator<qreal, QPen> it( d->customTickMarkPens );
        while( it.hasNext() ) {
            it.next();
            if ( it.value() != r.tickMarkPen(it.key()) )
                return false;
        }
    }
    return isEqual;
}

#if !defined( QT_NO_DEBUG_STREAM )
QDebug operator << ( QDebug dbg, const KDChart::RulerAttributes& a )
{
    dbg << "KDChart::RulerAttributes("
            << "tickMarkPen=" << a.tickMarkPen()
            << "majorTickMarkPen=" << a.majorTickMarkPen()
            << "minorTickMarkPen=" << a.minorTickMarkPen();
    const RulerAttributes::TickMarkerPensMap pens( a.tickMarkPens() );
    QMapIterator<qreal, QPen> it( pens );
    while( it.hasNext() ) {
        it.next();
        dbg << "customTickMarkPen=(" << it.value() << " : " << it.key() << ")";
    }
    dbg << ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */

