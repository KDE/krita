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

#include "SelectionTransformCommand.h"
#include <KoSelection.h>

SelectionTransformCommand::SelectionTransformCommand( KoSelection * selection, const QTransform &oldTransformation, const QTransform &newTransformation, QUndoCommand * parent )
: QUndoCommand( parent )
, m_selection( selection )
, m_oldTransformation( oldTransformation )
, m_newTransformation( newTransformation )
{
    Q_ASSERT( m_selection );
    m_selectedShapes = m_selection->selectedShapes();
}

void SelectionTransformCommand::redo()
{
    QUndoCommand::redo();

    m_selection->blockSignals( true );
    
    m_selection->deselectAll();
    foreach( KoShape * shape, m_selectedShapes )
        m_selection->select( shape, false );

    m_selection->setTransformation( m_newTransformation );
    
    m_selection->blockSignals( false );
}

void SelectionTransformCommand::undo()
{
    m_selection->blockSignals( true );

    m_selection->deselectAll();
    foreach( KoShape * shape, m_selectedShapes )
        m_selection->select( shape, false );
    
    m_selection->setTransformation( m_oldTransformation );

    m_selection->blockSignals( false );

    QUndoCommand::undo();
}
