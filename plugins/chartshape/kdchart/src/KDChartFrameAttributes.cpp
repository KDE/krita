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

#include "KDChartFrameAttributes.h"

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

class FrameAttributes::Private
{
    friend class FrameAttributes;
public:
    Private();
private:
    bool visible;
    QPen pen;
    int padding;
};

FrameAttributes::Private::Private() :
    visible( false ),
    padding( 0 )
{
}


FrameAttributes::FrameAttributes()
    : _d( new Private() )
{
}

FrameAttributes::FrameAttributes( const FrameAttributes& r )
    : _d( new Private( *r.d ) )
{
}

FrameAttributes & FrameAttributes::operator=( const FrameAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

FrameAttributes::~FrameAttributes()
{
    delete _d; _d = 0;
}


bool FrameAttributes::operator==( const FrameAttributes& r ) const
{
    /*
    qDebug() << "FrameAttributes:" << (isVisible() == r.isVisible())
        << (pen() == r.pen())
        << (padding() == r.padding()) << "\n";
    */
    return ( isVisible() == r.isVisible() &&
            pen() == r.pen() &&
            padding() == r.padding() );
}




void FrameAttributes::setVisible( bool visible )
{
    d->visible = visible;
}

bool FrameAttributes::isVisible() const
{
    return d->visible;
}

void FrameAttributes::setPen( const QPen & pen )
{
    d->pen = pen;
}

QPen FrameAttributes::pen() const
{
    return d->pen;
}

void FrameAttributes::setPadding( int padding )
{
    d->padding = padding;
}

int FrameAttributes::padding() const
{
    return d->padding;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::FrameAttributes& fa)
{
    dbg << "KDChart::FrameAttributes("
	<< "visible="<<fa.isVisible()
	<< "pen="<<fa.pen()
	<< "padding="<<fa.padding()
	<< ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
