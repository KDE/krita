/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeBackgroundCommand.h"

#include <klocale.h>

KoShapeBackgroundCommand::KoShapeBackgroundCommand( const KoSelectionSet &shapes, const QBrush &brush,
                                                    QUndoCommand *parent )
: QUndoCommand( parent )
, m_newBrush( brush )
{
    m_shapes = shapes.toList();

    setText( i18n( "Set background" ) );
}

KoShapeBackgroundCommand::~KoShapeBackgroundCommand() {
}

void KoShapeBackgroundCommand::redo () {
    foreach( KoShape *shape, m_shapes ) {
        m_oldBrushes.append( shape->background() );
        shape->setBackground( m_newBrush );
        shape->repaint();
    }
}

void KoShapeBackgroundCommand::undo () {
    QList<QBrush>::iterator brushIt = m_oldBrushes.begin();
    foreach( KoShape *shape, m_shapes ) {
        shape->setBackground( *brushIt );
        shape->repaint();
        brushIt++;
    }
}
