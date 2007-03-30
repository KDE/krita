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

#include "KoShapeMoveCommand.h"

#include <klocale.h>

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions, QUndoCommand *parent)
: QUndoCommand( parent )
, m_shapes(shapes)
, m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    Q_ASSERT(m_shapes.count() == m_previousPositions.count());
    Q_ASSERT(m_shapes.count() == m_newPositions.count());

    setText( i18n( "Move shapes" ) );
}

void KoShapeMoveCommand::redo() {
    QUndoCommand::redo();
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_newPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeMoveCommand::undo() {
    QUndoCommand::undo();
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_previousPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}
