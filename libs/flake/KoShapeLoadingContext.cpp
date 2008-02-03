/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeLoadingContext.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoSharedLoadingData.h"

#include <kdebug.h>

class KoShapeLoadingContext::Private {
public:
    Private( KoOdfLoadingContext &c, KoShapeControllerBase * sc )
    : context( c )
    , imageCollection( 0 )
    , zIndex( 0 )
    , shapeController( sc )
    {}
    ~Private()
    {
        foreach ( KoSharedLoadingData * data, sharedData )
        {
            delete data;
        }
    }
    KoOdfLoadingContext &context;
    QMap<QString, KoShapeLayer*> layers;
    QMap<QString, KoShape*> drawIds;
    KoImageCollection * imageCollection;
    QMap<QString, KoSharedLoadingData*> sharedData;
    QMap<KoShape*, int> zIndices;
    int zIndex;
    KoShapeControllerBase * shapeController;
};

KoShapeLoadingContext::KoShapeLoadingContext( KoOdfLoadingContext & context, KoShapeControllerBase * shapeController )
: d( new Private( context, shapeController ) )
{
}

KoShapeLoadingContext::~KoShapeLoadingContext()
{
    delete d;
}

KoOdfLoadingContext & KoShapeLoadingContext::odfLoadingContext()
{
    return d->context;
}

KoShapeLayer * KoShapeLoadingContext::layer( const QString & layerName )
{
   return d->layers.value( layerName, 0 );
}

void KoShapeLoadingContext::addLayer( KoShapeLayer * layer, const QString & layerName )
{
    d->layers[ layerName ] = layer;
}

void KoShapeLoadingContext::addShapeId( KoShape * shape, const QString & id )
{
    d->drawIds.insert( id, shape );
}

KoShape * KoShapeLoadingContext::shapeById( const QString & id )
{
   return d->drawIds.value( id, 0 );
}

void KoShapeLoadingContext::setImageCollection( KoImageCollection * imageCollection )
{
    d->imageCollection = imageCollection;
}

KoImageCollection * KoShapeLoadingContext::imageCollection()
{
    return d->imageCollection;
}

int KoShapeLoadingContext::zIndex()
{
    return d->zIndex++;
}

void KoShapeLoadingContext::setZIndex( int index )
{
    d->zIndex = index;
}

void KoShapeLoadingContext::addShapeZIndex( KoShape * shape, int index )
{
    d->zIndices.insert( shape, index );
}

const QMap<KoShape*, int> & KoShapeLoadingContext::shapeZIndices()
{
    return d->zIndices;
}

void KoShapeLoadingContext::addSharedData( const QString & id, KoSharedLoadingData * data )
{
    QMap<QString, KoSharedLoadingData*>::iterator it( d->sharedData.find( id ) );
    // data will not be overwritten
    if ( it == d->sharedData.end() ) {
        d->sharedData.insert( id, data );
    }
    else {
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT( it == d->sharedData.end() );
    }
}

KoSharedLoadingData * KoShapeLoadingContext::sharedData( const QString & id ) const
{
    KoSharedLoadingData * data = 0;
    QMap<QString, KoSharedLoadingData*>::const_iterator it( d->sharedData.find( id ) );
    if ( it != d->sharedData.end() ) {
        data = it.value();
    }
    return data;
}

KoShapeControllerBase * KoShapeLoadingContext::shapeController() const
{
    return d->shapeController;
}
