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

#include "KDChartPieAttributes.h"
#include "KDChartPieAttributes_p.h"

#include <QDebug>

#include <KDABLibFakes>

#define d d_func()


using namespace KDChart;


PieAttributes::Private::Private()
    : explodeFactor( 0.0 )
    , tangentialGapFactor( 0.0 )
    , radialGapFactor( 0.0 )
{
}


PieAttributes::PieAttributes()
    : _d( new Private() )
{
}

PieAttributes::PieAttributes( const PieAttributes& r )
    : _d( new Private( *r.d ) )
{
}

PieAttributes& PieAttributes::operator= ( const PieAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

PieAttributes::~PieAttributes()
{
    delete _d; _d = 0;
}


bool PieAttributes::operator==( const PieAttributes& r ) const
{
    return 
        explodeFactor()   == r.explodeFactor() &&
        gapFactor( true ) == r.gapFactor( true ) &&
        gapFactor( false) == r.gapFactor( false);
}


void PieAttributes::init( )
{

}

void PieAttributes::setExplode( bool enabled )
{
    d->explodeFactor = (enabled ? 0.1 : 0.0);
}

bool PieAttributes::explode() const
{
    return (d->explodeFactor != 0.0);
}

void PieAttributes::setExplodeFactor( qreal factor )
{
    d->explodeFactor = factor;
}

qreal PieAttributes::explodeFactor() const
{
    return d->explodeFactor;
}

void PieAttributes::setGapFactor( bool circular, qreal factor )
{
	if ( circular )
		d->tangentialGapFactor = factor;
	else
		d->radialGapFactor = factor;
}

qreal PieAttributes::gapFactor( bool circular ) const
{
	return circular ? d->tangentialGapFactor : d->radialGapFactor;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::PieAttributes& a)
{
    dbg << "KDChart::PieAttributes(";
    dbg << "explodeFactor="<< a.explodeFactor() << ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */

