/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoEventActionRemoveCommand.h"
#include <klocale.h>

#include "KoShape.h"
#include "KoEventAction.h"

KoEventActionRemoveCommand::KoEventActionRemoveCommand( KoShape * shape, KoEventAction * eventAction, QUndoCommand *parent )
: QUndoCommand( parent )
, m_shape( shape )
, m_eventAction( eventAction )
, m_deleteEventAction( false )
{
}

KoEventActionRemoveCommand::~KoEventActionRemoveCommand()
{
    if ( m_deleteEventAction ) {
        delete m_eventAction;
    }
}

void KoEventActionRemoveCommand::redo()
{
    m_shape->removeEventAction( m_eventAction );
    m_deleteEventAction = true;
}

void KoEventActionRemoveCommand::undo()
{
    m_shape->addEventAction( m_eventAction );
    m_deleteEventAction = false;
}
