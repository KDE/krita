/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeTransformCommand.h"
#include "KoShape.h"

KoShapeTransformCommand::KoShapeTransformCommand( const QList<KoShape*> &shapes, const QMatrix &transformation, QUndoCommand *parent )
    : QUndoCommand(parent), m_shapes( shapes )
{
    uint shapeCount = shapes.count();
    for( uint i = 0; i < shapeCount; ++i )
        m_transformations.append( transformation );
}
KoShapeTransformCommand::KoShapeTransformCommand( const QList<KoShape*> &shapes, const QList<QMatrix> &transformations, QUndoCommand * parent )
    : QUndoCommand(parent), m_shapes( shapes ), m_transformations( transformations )
{
    Q_ASSERT( shapes.count() == transformations.count() );
}

void KoShapeTransformCommand::redo()
{
    QUndoCommand::redo();
    uint shapeCount = m_shapes.count();
    for( uint i = 0; i < shapeCount; ++i )
    {
        KoShape * shape = m_shapes[i];
        shape->repaint();
        shape->applyTransformation( m_transformations[i] );
        shape->repaint();
    }
}

void KoShapeTransformCommand::undo()
{
    QUndoCommand::undo();

    uint shapeCount = m_shapes.count();
    for( uint i = 0; i < shapeCount; ++i )
    {
        KoShape * shape = m_shapes[i];
        shape->repaint();
        shape->applyTransformation( m_transformations[i].inverted() );
        shape->repaint();
    }
}
