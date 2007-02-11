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

#include "KoShapeGroupCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"

#include <klocale.h>
//#include <kdebug.h>

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped,
                                            QUndoCommand *parent)
: QUndoCommand( parent )
, m_shapes(shapes)
, m_clipped(clipped)
, m_container(container)
{
    Q_ASSERT(m_clipped.count() == m_shapes.count());
    foreach( KoShape* shape, m_shapes )
        m_oldParents.append( shape->parent() );

    setText( i18n( "Group shapes" ) );
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroup *container, QList<KoShape *> shapes, QUndoCommand *parent)
: QUndoCommand(parent)
, m_shapes(shapes)
, m_container(container)
{
    foreach( KoShape* shape, m_shapes )
    {
        m_clipped.append(false);
        m_oldParents.append( shape->parent() );
    }

    setText( i18n( "Group shapes" ) );
}

KoShapeGroupCommand::KoShapeGroupCommand(QUndoCommand *parent)
: QUndoCommand(parent)
{
}

void KoShapeGroupCommand::redo () {
    QList <QPointF> positions;
    bool boundingRectInitialized=true;
    QRectF bound;
    if(m_container->childCount() > 0)
        bound = m_container->boundingRect();
    else
        boundingRectInitialized = false;
    foreach(KoShape *shape, m_shapes) {
        positions.append(shape->absolutePosition());
        if(boundingRectInitialized)
            bound = bound.unite(shape->boundingRect());
        else {
            bound = shape->boundingRect();
            boundingRectInitialized = true;
        }
        m_container->addChild(shape);
    }
    m_container->setPosition( bound.topLeft() );
    m_container->resize( bound.size() );
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes[i]->setAbsolutePosition( positions[i] );
    }

//kDebug() << "after group: " << m_container->position().x() << ", " << m_container->position().y() << endl;
}

void KoShapeGroupCommand::undo () {
    QList <QPointF> positions;
    foreach(KoShape *shape, m_shapes)
        positions.append(shape->absolutePosition());

    for(int i=0; i < m_shapes.count(); i++) {
        m_container->removeChild(m_shapes[i]);
        m_shapes[i]->setAbsolutePosition( positions[i] );
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->addChild( m_shapes[i] );
    }
}
