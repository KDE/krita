/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

class KoShapeLoadingContext::Private {
public:
    Private(KoOasisLoadingContext &c) : context(c) {}
    KoOasisLoadingContext &context;
    QMap<QString, KoShapeLayer*> layers;
    QMap<QString, KoShape*> drawIds;
};

KoShapeLoadingContext::KoShapeLoadingContext( KoOasisLoadingContext & context )
: d( new Private(context))
{
}

KoOasisLoadingContext & KoShapeLoadingContext::koLoadingContext()
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

