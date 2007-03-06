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

#include "KoPathSeparateCommand.h"
#include "KoShapeControllerBase.h"
#include "KoPathShape.h"
#include <klocale.h>

class KoPathSeparateCommand::Private {
public:
    Private(KoShapeControllerBase *c, const QList<KoPathShape*> &p)
        : controller(c),
        paths(p),
        isSeparated( false )
    {
    }

    ~Private() {
        if( isSeparated && controller )
        {
            foreach( KoPathShape* p, paths )
                delete p;
        }
        else
        {
            foreach( KoPathShape* p, separatedPaths )
                delete p;
        }
    }

    KoShapeControllerBase *controller;
    QList<KoPathShape*> paths;
    QList<KoPathShape*> separatedPaths;
    bool isSeparated;
};


KoPathSeparateCommand::KoPathSeparateCommand( KoShapeControllerBase *controller, const QList<KoPathShape*> &paths, QUndoCommand *parent )
    : QUndoCommand( parent ),
    d(new Private(controller, paths))
{
    setText( i18n( "Separate paths" ) );
}

KoPathSeparateCommand::~KoPathSeparateCommand()
{
    delete d;
}

void KoPathSeparateCommand::redo()
{
    if( d->separatedPaths.isEmpty() )
    {
        foreach( KoPathShape* p, d->paths )
        {
            QList<KoPathShape*> separatedPaths;
            if( p->separate( separatedPaths ) )
                d->separatedPaths << separatedPaths;
        }
    }

    d->isSeparated = true;

    if( d->controller )
    {
        foreach( KoPathShape* p, d->paths )
            d->controller->removeShape( p );
        foreach( KoPathShape *p, d->separatedPaths )
            d->controller->addShape( p );
    }
    foreach( KoPathShape* p, d->paths )
        p->repaint();
}

void KoPathSeparateCommand::undo()
{
    if( d->controller )
    {
        foreach( KoPathShape *p, d->separatedPaths )
            d->controller->removeShape( p );
        foreach( KoPathShape* p, d->paths )
            d->controller->addShape( p );
    }

    d->isSeparated = false;

    foreach( KoPathShape* p, d->paths )
        p->repaint();
}
