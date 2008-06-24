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

#include "KoGradientBackground.h"
#include <KoStyleStack.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfGraphicStyles.h>
#include <KoShapeSavingContext.h>

#include <KDebug>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KoGradientBackground::Private
{
public:
    Private() : gradient(0) {};
    ~Private()
    {
        delete gradient;
    }
    QGradient * cloneGradient( const QGradient * gradient ) const
    {
        if( ! gradient )
            return 0;

        QGradient * clone = 0;

        switch( gradient->type() )
        {
            case QGradient::LinearGradient:
            {
                const QLinearGradient * lg = static_cast<const QLinearGradient*>( gradient );
                clone = new QLinearGradient( lg->start(), lg->finalStop() );
                break;
            }
            case QGradient::RadialGradient:
            {
                const QRadialGradient * rg = static_cast<const QRadialGradient*>( gradient );
                clone = new QRadialGradient( rg->center(), rg->radius(), rg->focalPoint() );
                break;
            }
            case QGradient::ConicalGradient:
            {
                const QConicalGradient * cg = static_cast<const QConicalGradient*>( gradient );
                clone = new QConicalGradient( cg->center(), cg->angle() );
                break;
            }
            default:
                return 0;
        }

        clone->setSpread( gradient->spread() );
        clone->setStops( gradient->stops() );

        return clone;
    }

    QGradient * gradient;
    QMatrix matrix;
};

KoGradientBackground::KoGradientBackground( QGradient * gradient, const QMatrix &matrix )
    : d( new Private() )
{
    d->gradient = d->cloneGradient( gradient );
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::KoGradientBackground( const QGradient & gradient, const QMatrix &matrix )
    : d( new Private() )
{
    d->gradient = d->cloneGradient( &gradient );
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::~KoGradientBackground()
{
    delete d;
}

void KoGradientBackground::setMatrix( const QMatrix &matrix )
{
    d->matrix = matrix;
}

QMatrix KoGradientBackground::matrix() const
{
    return d->matrix;
}

void KoGradientBackground::setGradient( QGradient * gradient )
{
    if( d->gradient )
        delete d->gradient;

    d->gradient = gradient;
    Q_ASSERT(d->gradient);
}

void KoGradientBackground::setGradient( const QGradient & gradient )
{
    if( d->gradient )
        delete d->gradient;

    d->gradient = d->cloneGradient( &gradient );
    Q_ASSERT(d->gradient);
}

const QGradient * KoGradientBackground::gradient() const
{
    return d->gradient;
}

KoGradientBackground& KoGradientBackground::operator = ( const KoGradientBackground &rhs )
{
    if( this == &rhs )
        return *this;

    d->matrix = rhs.d->matrix;
    delete d->gradient;
    d->gradient = d->cloneGradient( rhs.d->gradient );
    Q_ASSERT( d->gradient );

    return *this;
}

void KoGradientBackground::paint( QPainter &painter, const QPainterPath &fillPath ) const
{
    QBrush brush( *d->gradient );
    brush.setMatrix( d->matrix );

    painter.setBrush( brush );
    painter.drawPath( fillPath );
}

void KoGradientBackground::fillStyle( KoGenStyle &style, KoShapeSavingContext &context )
{
    QBrush brush( *d->gradient );
    brush.setMatrix( d->matrix );
    KoOdfGraphicStyles::saveOasisFillStyle( style, context.mainStyles(), brush );
}

bool KoGradientBackground::loadStyle( KoOdfLoadingContext & context, const QSizeF &shapeSize )
{
    KoStyleStack &styleStack = context.styleStack();
    if( ! styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) 
        return false;

    QString fillStyle = styleStack.property( KoXmlNS::draw, "fill" );
    if ( fillStyle == "gradient" )
    {
        QBrush brush = KoOdfGraphicStyles::loadOasisGradientStyle( styleStack, context.stylesReader(), shapeSize );
        const QGradient * gradient = brush.gradient();
        if ( gradient ) {
            d->gradient = d->cloneGradient( gradient );
            d->matrix = brush.matrix();
            return true;
        }
    }
    return false;
}
