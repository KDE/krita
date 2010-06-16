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
