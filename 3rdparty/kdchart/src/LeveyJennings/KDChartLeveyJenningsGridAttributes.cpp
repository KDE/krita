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

#include "KDChartLeveyJenningsGridAttributes.h"

#include <QBrush>
#include <QMap>
#include <QPen>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

class LeveyJenningsGridAttributes::Private
{
    friend class LeveyJenningsGridAttributes;
public:
    Private();
private:
    QMap< GridType, bool > visible;
    QMap< GridType, QPen > pens;
    QMap< LeveyJenningsGridAttributes::Range, QBrush > rangeBrushes;
};

LeveyJenningsGridAttributes::Private::Private()
{
    pens[ Calculated ].setCapStyle( Qt::FlatCap );
    pens[ Calculated ].setColor( Qt::blue );
    pens[ Expected ].setCapStyle( Qt::FlatCap );
    pens[ Expected ].setColor( Qt::black );
    
    visible[ Calculated ] = true;
    visible[ Expected ] = true;
    
    rangeBrushes[ LeveyJenningsGridAttributes::CriticalRange ] = QBrush( QColor( 255, 255, 192 ) );
    rangeBrushes[ LeveyJenningsGridAttributes::OutOfRange ]    = QBrush( QColor( 255, 128, 128 ) );
}


LeveyJenningsGridAttributes::LeveyJenningsGridAttributes()
    : _d( new Private() )
{
    // this bloc left empty intentionally
}

LeveyJenningsGridAttributes::LeveyJenningsGridAttributes( const LeveyJenningsGridAttributes& r )
    : _d( new Private( *r.d ) )
{
}

LeveyJenningsGridAttributes & LeveyJenningsGridAttributes::operator=( const LeveyJenningsGridAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

LeveyJenningsGridAttributes::~LeveyJenningsGridAttributes()
{
    delete _d; _d = 0;
}


bool LeveyJenningsGridAttributes::operator==( const LeveyJenningsGridAttributes& r ) const
{
    return  isGridVisible( Expected ) == r.isGridVisible( Expected ) &&
            isGridVisible( Calculated ) == r.isGridVisible( Calculated ) &&
            gridPen( Expected ) == r.gridPen( Expected ) &&
            gridPen( Calculated ) == r.gridPen( Calculated );
}

void LeveyJenningsGridAttributes::setRangeBrush( Range range, const QBrush& brush )
{
    d->rangeBrushes[ range ] = brush;
}

QBrush LeveyJenningsGridAttributes::rangeBrush( Range range ) const
{
    return d->rangeBrushes[ range ];
}


void LeveyJenningsGridAttributes::setGridVisible( GridType type, bool visible )
{
    d->visible[ type ] = visible;
}

bool LeveyJenningsGridAttributes::isGridVisible( GridType type ) const
{
    return d->visible[ type ];
}

void LeveyJenningsGridAttributes::setGridPen( GridType type, const QPen& pen )
{
    d->pens[ type ] = pen;
    d->pens[ type ].setCapStyle( Qt::FlatCap );
}

QPen LeveyJenningsGridAttributes::gridPen( GridType type ) const
{
    return d->pens[ type ];
}
