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

#include "KDChartThreeDLineAttributes.h"
#include "KDChartThreeDLineAttributes_p.h"

#include <QDebug>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

ThreeDLineAttributes::Private::Private()
    : lineXRotation( 15 ),
      lineYRotation( 15 )
{
}


ThreeDLineAttributes::ThreeDLineAttributes()
    : AbstractThreeDAttributes( new Private() )
{

}

ThreeDLineAttributes::ThreeDLineAttributes( const ThreeDLineAttributes& r )
    : AbstractThreeDAttributes( new Private( *r.d) )
{
}

ThreeDLineAttributes& ThreeDLineAttributes::operator= ( const ThreeDLineAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

ThreeDLineAttributes::~ThreeDLineAttributes()
{
}

void ThreeDLineAttributes::init()
{
}


bool ThreeDLineAttributes::operator==( const ThreeDLineAttributes& r ) const
{
    return ( lineXRotation() == r.lineXRotation() &&
             lineYRotation() == r.lineYRotation() &&
             AbstractThreeDAttributes::operator==(r));
}



void ThreeDLineAttributes::setLineXRotation( uint degrees )
{
    d->lineXRotation = degrees;
}

uint ThreeDLineAttributes::lineXRotation() const
{
    return d->lineXRotation;
}

void ThreeDLineAttributes::setLineYRotation( uint degrees )
{
    d->lineYRotation = degrees;
}

uint ThreeDLineAttributes::lineYRotation() const
{
    return d->lineYRotation;
}


#if !defined(QT_NO_DEBUG_STREAM)

QDebug operator<<(QDebug dbg, const KDChart::ThreeDLineAttributes& a)
{
    dbg << "KDChart::ThreeDLineAttributes(";
    dbg = operator <<( dbg, static_cast<const AbstractThreeDAttributes&>(a) );
    dbg << " lineXRotation="<< a.lineXRotation()
        << " lineYRotation="<< a.lineYRotation()
        << ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */

