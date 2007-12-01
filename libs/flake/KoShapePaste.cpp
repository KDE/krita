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

#include "KoShapePaste.h"

#include <kdebug.h>
#include <klocale.h>

#include <KoOasisLoadingContext.h>
#include <KoOdfReadStore.h>

#include "KoCanvasBase.h"
#include "KoShapeController.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "commands/KoShapeCreateCommand.h"

struct KoShapePaste::Private
{
    Private( KoDocument * doc, KoCanvasBase * canvas, int zIndex, KoShapeContainer * parent )
    : doc( doc )
    , canvas( canvas )
    , zIndex( zIndex )
    , parent( parent )
    {}

    KoDocument * doc;
    KoCanvasBase * canvas;
    int zIndex;
    KoShapeContainer * parent;
};

KoShapePaste::KoShapePaste( KoDocument * doc, KoCanvasBase * canvas, int zIndex, KoShapeContainer * parent )
: d( new Private( doc, canvas, zIndex, parent ) )
{
}

KoShapePaste::~KoShapePaste()
{
    delete d;
}

bool KoShapePaste::process( const KoXmlElement & body, KoOdfReadStore & odfStore )
{
    KoOasisLoadingContext loadingContext( d->doc, odfStore.styles(), odfStore.store() );
    KoShapeLoadingContext context( loadingContext );

    context.setZIndex( d->zIndex );

    QUndoCommand * cmd = new QUndoCommand( i18n( "Paste Shapes" ) );

    // TODO if this is a text create a text shape and load the text inside the new shape.
    KoXmlElement element;
    forEachElement( element, body )
    {
        kDebug(30006) <<"loading shape" << element.localName();

        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf( element, context );
        if ( shape ) {
            if ( ! shape->parent() ) {
                shape->setParent( d->parent );
            }
            d->canvas->shapeController()->addShapeDirect(shape, cmd);
        }
    }

    d->canvas->addCommand( cmd );

    return true;
}
