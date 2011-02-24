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

#include "KDChartThreeDPieAttributes.h"
#include "KDChartThreeDPieAttributes_p.h"

#include <QDebug>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

ThreeDPieAttributes::Private::Private()
    : useShadowColors( true )
{
}


ThreeDPieAttributes::ThreeDPieAttributes()
    : AbstractThreeDAttributes( new Private() )
{

}

ThreeDPieAttributes::ThreeDPieAttributes( const ThreeDPieAttributes& r )
    : AbstractThreeDAttributes( new Private( *r.d) )
{
}

ThreeDPieAttributes& ThreeDPieAttributes::operator= ( const ThreeDPieAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

ThreeDPieAttributes::~ThreeDPieAttributes()
{
}

void ThreeDPieAttributes::init()
{
}


bool ThreeDPieAttributes::operator==( const ThreeDPieAttributes& r ) const
{
    return ( useShadowColors() == r.useShadowColors() &&
             AbstractThreeDAttributes::operator==(r));
}

void ThreeDPieAttributes::setUseShadowColors( bool shadowColors )
{
    d->useShadowColors = shadowColors;
}

bool ThreeDPieAttributes::useShadowColors() const
{
    return d->useShadowColors;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::ThreeDPieAttributes& a)
{
    dbg << "KDChart::ThreeDPieAttributes(";
    dbg = operator <<( dbg, static_cast<const AbstractThreeDAttributes&>(a) );
    dbg << "useShadowColors="<< a.useShadowColors() << ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */

