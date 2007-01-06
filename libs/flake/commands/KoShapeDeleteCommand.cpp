/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeDeleteCommand.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerBase.h"

#include <klocale.h>

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, KoShape *shape, QUndoCommand *parent )
: QUndoCommand( parent )
, m_controller( controller )
, m_deleteShapes( false )
{
    m_shapes.append( shape );
    m_oldParents.append( shape->parent() );

    setText( i18n( "Delete shape" ) );
}

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, const QList<KoShape*> &shapes,
                                            QUndoCommand *parent)
: QUndoCommand( parent )
, m_controller( controller )
, m_shapes(shapes)
, m_deleteShapes( false )
{
    foreach( KoShape *shape, m_shapes ) {
        m_oldParents.append( shape->parent() );
    }

    setText( i18n( "Delete shapes" ) );
}

KoShapeDeleteCommand::~KoShapeDeleteCommand() {
    if( ! m_deleteShapes )
        return;

    foreach (KoShape *shape, m_shapes ) {
        delete shape;
    }
}

void KoShapeDeleteCommand::redo () {
    if( ! m_controller )
        return;

    for(int i=0; i < m_shapes.count(); i++) {
        // the parent has to be there when it is removed from the KoShapeControllerBase
        m_controller->removeShape( m_shapes[i] );
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->removeChild( m_shapes[i] );
    }
    m_deleteShapes = true;
}

void KoShapeDeleteCommand::undo () {
    if( ! m_controller )
        return;

    for(int i=0; i < m_shapes.count(); i++) {
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->addChild( m_shapes[i] );
        // the parent has to be there when it is added to the KoShapeControllerBase
        m_controller->addShape( m_shapes[i] );
    }
    m_deleteShapes = false;
}
