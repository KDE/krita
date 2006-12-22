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

#include "KoShapeCreateCommand.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerBase.h"

#include <klocale.h>

KoShapeCreateCommand::KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape, QUndoCommand *parent )
: QUndoCommand( parent )
, m_controller( controller )
, m_shape( shape )
, m_deleteShape( true )
{
    setText( i18n( "Create shape" ) );
}

KoShapeCreateCommand::~KoShapeCreateCommand() {
    if( m_shape && m_deleteShape )
        delete m_shape;
}

void KoShapeCreateCommand::redo () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    recurse(m_shape, Add);
    m_deleteShape = false;
}

void KoShapeCreateCommand::undo () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    recurse(m_shape, Remove);
    m_deleteShape = true;
}

void KoShapeCreateCommand::recurse(KoShape *shape, const AddRemove ar) {
    if(ar == Remove)
        m_controller->removeShape( m_shape );
    else
        m_controller->addShape( m_shape );

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*> (shape);
    if(container) {
        foreach(KoShape *child, container->iterator())
            recurse(child, ar);
    }
}
