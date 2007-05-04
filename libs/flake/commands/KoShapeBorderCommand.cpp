/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeBorderCommand.h"
#include "KoShape.h"
#include "KoShapeBorderModel.h"

#include <klocale.h>

class KoShapeBorderCommand::Private {
public:
    Private() {}
    QList<KoShape*> shapes;                ///< the shapes to set border for
    QList<KoShapeBorderModel*> oldBorders; ///< the old borders, one for each shape
    QList<KoShapeBorderModel*> newBorders; ///< the new borders to set

};

KoShapeBorderCommand::KoShapeBorderCommand( const QList<KoShape*> &shapes, KoShapeBorderModel *border,
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, d(new Private())
{
    d->shapes = shapes;
    int shapeCount = shapes.count();
    for( int i = 0; i < shapeCount; ++i )
        d->newBorders.append( border );

    // save old borders
    foreach( KoShape *shape, d->shapes )
        d->oldBorders.append( shape->border() );

    // XXX this command forgets to delete the old border on a shape and assumes someone else is still
    // using it.  For such cases we should probably create a new border for each shape in the redo method.
    // and delete the old borders in the destructor

    setText( i18n( "Set border" ) );
}

KoShapeBorderCommand::KoShapeBorderCommand( const QList<KoShape*> &shapes, QList<KoShapeBorderModel*> borders,
                                            QUndoCommand *parent )
: QUndoCommand( parent )
, d(new Private())
{
    Q_ASSERT( shapes.count() == borders.count() );

    d->shapes = shapes;
    d->newBorders = borders;

    // save old borders
    foreach( KoShape *shape, d->shapes )
        d->oldBorders.append( shape->border() );

    // XXX this command forgets to delete the old border on a shape and assumes someone else is still
    // using it.  For such cases we should probably create a new border for each shape in the redo method.
    // and delete the old borders in the destructor

    setText( i18n( "Set border" ) );
}

KoShapeBorderCommand::~KoShapeBorderCommand() {
    delete d;
}

void KoShapeBorderCommand::redo () {
    QUndoCommand::redo();
    QList<KoShapeBorderModel*>::iterator borderIt = d->newBorders.begin();
    foreach( KoShape *shape, d->shapes ) {
        shape->setBorder( *borderIt );
        shape->repaint();
        borderIt++;
    }
}

void KoShapeBorderCommand::undo () {
    QUndoCommand::undo();
    QList<KoShapeBorderModel*>::iterator borderIt = d->oldBorders.begin();
    foreach( KoShape *shape, d->shapes ) {
        shape->setBorder( *borderIt );
        shape->repaint();
        borderIt++;
    }
}
