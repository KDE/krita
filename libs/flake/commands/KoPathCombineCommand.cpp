/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathCombineCommand.h"
#include "KoShapeControllerBase.h"
#include "KoShapeContainer.h"
#include <klocale.h>

class KoPathCombineCommand::Private {
public:
    Private(KoShapeControllerBase *c, const QList<KoPathShape*> &p)
        : controller(c),
        paths(p),
        combinedPath( 0 ),
        isCombined( false )
    {
    }
    ~Private() {
        if( isCombined && controller )
        {
            foreach( KoPathShape* path, paths )
                delete path;
        }
        else
            delete combinedPath;
    }

    KoShapeControllerBase *controller;
    QList<KoPathShape*> paths;
    KoPathShape *combinedPath;
    bool isCombined;
};

KoPathCombineCommand::KoPathCombineCommand( KoShapeControllerBase *controller,
        const QList<KoPathShape*> &paths, QUndoCommand *parent )
    : QUndoCommand( parent ),
    d(new Private(controller, paths))
{
    setText( i18n( "Combine paths" ) );
}

KoPathCombineCommand::~KoPathCombineCommand()
{
    delete d;
}

void KoPathCombineCommand::redo()
{
    if( ! d->paths.size() )
        return;

    if( ! d->combinedPath )
    {
        d->combinedPath = new KoPathShape();
        KoShapeContainer * parent = d->paths.first()->parent();
        if(parent)
            parent->addChild(d->combinedPath);
        d->combinedPath->setBorder( d->paths.first()->border() );
        d->combinedPath->setShapeId( d->paths.first()->shapeId() );
        // combine the paths
        foreach( KoPathShape* path, d->paths )
            d->combinedPath->combine( path );
    }

    d->isCombined = true;

    if( d->controller )
    {
        foreach( KoPathShape* p, d->paths )
            d->controller->removeShape( p );

        d->controller->addShape( d->combinedPath );
    }
}

void KoPathCombineCommand::undo()
{
    if( ! d->paths.size() )
        return;

    d->isCombined = false;

    if( d->controller )
    {
        d->controller->removeShape( d->combinedPath );
        foreach( KoPathShape* p, d->paths )
        {
            d->controller->addShape( p );
        }
    }
}

