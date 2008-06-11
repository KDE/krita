/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPatternBackground.h"
#include "KoShapeStyleWriter.h"
#include <KoStyleStack.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfGraphicStyles.h>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KoPatternBackground::Private
{
public:
    Private()
    {
    }
    QImage pattern;
    QMatrix matrix;
};

KoPatternBackground::KoPatternBackground()
    : d( new Private() )
{
}

KoPatternBackground::KoPatternBackground( const QImage &pattern )
    : d( new Private() )
{
    d->pattern = pattern;
}

KoPatternBackground::~KoPatternBackground()
{
    delete d;
}

void KoPatternBackground::setMatrix( const QMatrix &matrix )
{
    d->matrix = matrix;
}

QMatrix KoPatternBackground::matrix() const
{
    return d->matrix;
}

void KoPatternBackground::setPattern( const QImage &pattern )
{
    d->pattern = pattern;
}

QImage KoPatternBackground::pattern()
{
    return d->pattern;
}

KoPatternBackground& KoPatternBackground::operator = ( const KoPatternBackground &rhs )
{
    if( this == &rhs )
        return *this;

    d->matrix = rhs.d->matrix;
    d->pattern = rhs.d->pattern;

    return *this;
}

void KoPatternBackground::paint( QPainter &painter, const QPainterPath &fillPath ) const
{
    QBrush brush( d->pattern );
    brush.setMatrix( d->matrix );

    painter.setBrush( brush );
    painter.drawPath( fillPath );
}

void KoPatternBackground::fillStyle( KoGenStyle &style, KoShapeSavingContext &context )
{
    QBrush brush( d->pattern );
    brush.setMatrix( d->matrix );

    KoShapeStyleWriter styleWriter( context );
    styleWriter.addFillStyle( style, brush );
}

bool KoPatternBackground::loadStyle( KoOdfLoadingContext & context, const QSizeF &shapeSize )
{
    KoStyleStack &styleStack = context.styleStack();
    if( ! styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) 
        return false;

    QString fillStyle = styleStack.property( KoXmlNS::draw, "fill" );
    if( fillStyle == "bitmap" )
    {
        QBrush brush = KoOdfGraphicStyles::loadOasisPatternStyle( styleStack, context, shapeSize );
        d->matrix = brush.matrix();
        d->pattern = brush.textureImage();
        return true;
    }

    return false;
}
