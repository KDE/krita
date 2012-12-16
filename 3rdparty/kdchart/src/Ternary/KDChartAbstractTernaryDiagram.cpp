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

#include "KDChartAbstractTernaryDiagram.h"
#include "KDChartAbstractTernaryDiagram_p.h"

#include "KDChartTernaryCoordinatePlane.h"

using namespace KDChart;

AbstractTernaryDiagram::Private::Private()
    : AbstractDiagram::Private()
{
}

void AbstractTernaryDiagram::init()
{
}

#define d d_func()

AbstractTernaryDiagram::AbstractTernaryDiagram( QWidget* parent,
                                                TernaryCoordinatePlane* plane )
    : AbstractDiagram( parent, plane )
{
}

AbstractTernaryDiagram::~AbstractTernaryDiagram()
{
    while ( ! d->axesList.isEmpty() ) {
        TernaryAxis* axis = d->axesList.takeFirst();
        delete axis;
    }
}

void AbstractTernaryDiagram::addAxis( TernaryAxis* axis )
{
    d->axesList.append( axis );
    // FIXME update
}

void AbstractTernaryDiagram::takeAxis( TernaryAxis* axis )
{

    int index = d->axesList.indexOf( axis );
    if ( index != -1 )
        d->axesList.removeAt( index );
    // FIXME update
}

TernaryAxisList AbstractTernaryDiagram::axes() const
{
    return d->axesList;
}

void AbstractTernaryDiagram::paint (PaintContext *paintContext)
{
    d->paint( paintContext );
}

