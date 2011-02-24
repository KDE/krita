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

#include "KDChartAbstractThreeDAttributes.h"
#include "KDChartAbstractThreeDAttributes_p.h"

#include <QDebug>
#include <QBrush>
#include <KDABLibFakes>

#define d d_func()


using namespace KDChart;


AbstractThreeDAttributes::Private::Private()
    : enabled( false ),
      depth( 20 ),
      threeDBrushEnabled( false )
{
}


AbstractThreeDAttributes::AbstractThreeDAttributes()
    : _d( new Private() )
{
}

AbstractThreeDAttributes::AbstractThreeDAttributes( const AbstractThreeDAttributes& r )
    : _d( new Private( *r.d ) )
{
}

AbstractThreeDAttributes& AbstractThreeDAttributes::operator= ( const AbstractThreeDAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

AbstractThreeDAttributes::~AbstractThreeDAttributes()
{
    delete _d; _d = 0;
}


bool AbstractThreeDAttributes::operator==( const AbstractThreeDAttributes& r ) const
{
    return isEnabled() == r.isEnabled() &&
        depth() == r.depth() && 
        isThreeDBrushEnabled() == r.isThreeDBrushEnabled();
}


void AbstractThreeDAttributes::init( )
{

}

void AbstractThreeDAttributes::setEnabled( bool enabled )
{
    d->enabled = enabled;
}

bool AbstractThreeDAttributes::isEnabled() const
{
    return d->enabled;
}

void AbstractThreeDAttributes::setDepth( double depth )
{
    d->depth = depth;
}


double AbstractThreeDAttributes::depth() const
{
    return d->depth;
}


double AbstractThreeDAttributes::validDepth() const
{
    return isEnabled() ? d->depth : 0.0;
}

bool AbstractThreeDAttributes::isThreeDBrushEnabled() const
{
    return d->threeDBrushEnabled;
}

void AbstractThreeDAttributes::setThreeDBrushEnabled( bool enabled )
{
    d->threeDBrushEnabled = enabled;
}

QBrush AbstractThreeDAttributes::threeDBrush( const QBrush& brush, const QRectF& rect ) const
{
    if( isEnabled() && isThreeDBrushEnabled() && brush.style() == Qt::SolidPattern ) {
        QLinearGradient gr(rect.topLeft(), rect.bottomRight());
        gr.setColorAt(0.0, brush.color());
        gr.setColorAt(0.5, brush.color().lighter(180));
        gr.setColorAt(1.0, brush.color());
        return QBrush(gr);
    }
    return brush;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::AbstractThreeDAttributes& a)
{
    dbg << "enabled="<<a.isEnabled()
        << "depth="<<a.depth();
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
