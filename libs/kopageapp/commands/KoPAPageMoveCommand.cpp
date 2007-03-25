/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
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

#include "KoPAPageMoveCommand.h"

#include <klocale.h>

#include "KoPADocument.h"
#include "KoPAPageBase.h"

KoPAPageMoveCommand::KoPAPageMoveCommand( KoPADocument *document, KoPAPageBase *page, KoPAPageBase *after, QUndoCommand *parent )
: QUndoCommand( parent )
, m_document( document )    
, m_page( page )
, m_after( after )
, m_index( -1 )
{
    Q_ASSERT( document );
    Q_ASSERT( page );
    setText( i18n( "Move page" ) );
}

KoPAPageMoveCommand::~KoPAPageMoveCommand()
{
}

void KoPAPageMoveCommand::redo()
{
    m_index = m_document->takePage( m_page );
    Q_ASSERT( m_index != -1 );
    m_document->insertPage( m_page, m_after );
}

void KoPAPageMoveCommand::undo()
{
    Q_ASSERT( m_index != -1 );

    int pos = m_document->takePage( m_page );
    Q_ASSERT( pos != -1 );

    m_document->insertPage( m_page, m_index );
}

