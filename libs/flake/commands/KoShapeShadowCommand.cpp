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

#include "KoShapeShadowCommand.h"
#include "KoShape.h"
#include "KoShapeShadow.h"

#include <klocale.h>

class KoShapeShadowCommand::Private 
{
public:
    Private() {}
    ~Private() 
    {
        foreach( KoShapeShadow* shadow, oldShadows )
        {
            if(shadow && shadow->useCount() <= 0)
                delete shadow;
        }
    }

    QList<KoShape*> shapes;           ///< the shapes to set shadow for
    QList<KoShapeShadow*> oldShadows; ///< the old shadows, one for each shape
    QList<KoShapeShadow*> newShadows; ///< the new shadows to set
};

KoShapeShadowCommand::KoShapeShadowCommand( const QList<KoShape*> &shapes, KoShapeShadow *shadow,
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, d(new Private())
{
    d->shapes = shapes;
    int shapeCount = shapes.count();
    for( int i = 0; i < shapeCount; ++i )
        d->newShadows.append( shadow );

    // save old shadows
    foreach( KoShape *shape, d->shapes )
        d->oldShadows.append( shape->shadow() );

    setText( i18n( "Set Shadow" ) );
}

KoShapeShadowCommand::KoShapeShadowCommand( const QList<KoShape*> &shapes,
                                            const QList<KoShapeShadow*> &shadows,
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, d(new Private())
{
    Q_ASSERT( shapes.count() == shadows.count() );

    d->shapes = shapes;
    d->newShadows = shadows;

    // save old shadows
    foreach( KoShape *shape, shapes )
        d->oldShadows.append( shape->shadow() );

    setText( i18n( "Set Shadow" ) );
}

KoShapeShadowCommand::KoShapeShadowCommand( KoShape* shape, KoShapeShadow *shadow, QUndoCommand *parent )
: QUndoCommand( parent )
, d(new Private())
{
    d->shapes.append( shape );
    d->newShadows.append( shadow );
    d->oldShadows.append( shape->shadow() );

    setText( i18n( "Set Shadow" ) );
}

KoShapeShadowCommand::~KoShapeShadowCommand()
{
    delete d;
}

void KoShapeShadowCommand::redo()
{
    QUndoCommand::redo();
    int shapeCount = d->shapes.count();
    for( int i = 0; i < shapeCount; ++i )
    {
        KoShape *shape = d->shapes[i];
        shape->update();
        shape->setShadow( d->newShadows[i] );
        shape->update();
    }
}

void KoShapeShadowCommand::undo()
{
    QUndoCommand::undo();
    int shapeCount = d->shapes.count();
    for( int i = 0; i < shapeCount; ++i )
    {
        KoShape *shape = d->shapes[i];
        shape->update();
        shape->setShadow( d->oldShadows[i] );
        shape->update();
    }
}
