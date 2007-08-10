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
#include "KoShapeControllerBase.h"
#include "KoShape.h"
#include "KoShapeContainer.h"

#include <kdebug.h>

class KoShapeLoadingContext::Private {
public:
    Private(KoOasisLoadingContext &c) : context(c) {}
    ~Private()
    {
        if( shapesForDocument.count() )
            kWarning(30006) << "KoShapeLoadingContext: there are loaded shapes not added to the document";
        // the collected shapes were not added to a document
        // so we remove them from their parents and delete them
        // to not leak any memory
        foreach( KoShape * shape, shapesForDocument )
        {
            if( shape->parent() )
                shape->parent()->removeChild( shape );
            delete shape;
        }
        shapesForDocument.clear();
    }
    KoOasisLoadingContext &context;
    QMap<QString, KoShapeLayer*> layers;
    QMap<QString, KoShape*> drawIds;
    QList<KoShape*> shapesForDocument;
};

KoShapeLoadingContext::KoShapeLoadingContext( KoOasisLoadingContext & context )
: d( new Private(context))
{
}

KoShapeLoadingContext::~KoShapeLoadingContext()
{
    delete d;
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

void KoShapeLoadingContext::addShapeToDocument( KoShape * shape )
{
    d->shapesForDocument.append( shape );
}

void KoShapeLoadingContext::transferShapesToDocument( KoShapeControllerBase * controller )
{
    if( ! controller )
        return;

    // add all the shapes collected during loading to the shape controller
    foreach( KoShape * shape, d->shapesForDocument )
        controller->addShape( shape );

    // the shape controller now owns the shapes, so we can clear the list
    d->shapesForDocument.clear();
}
