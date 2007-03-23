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

#include "KoPAPageDeleteCommand.h"

#include <klocale.h>

#include "KoPADocument.h"
#include "KoPAPageBase.h"

KoPAPageDeleteCommand::KoPAPageDeleteCommand( KoPADocument *document, KoPAPageBase *page, QUndoCommand *parent )
: QUndoCommand( parent )
, m_document( document )    
, m_page( page )
, m_index( -1 )                
, m_deletePage( false )
{
    Q_ASSERT( document );
    Q_ASSERT( page );
    setText( i18n( "Delete page" ) );
}

KoPAPageDeleteCommand::~KoPAPageDeleteCommand()
{
    if ( m_deletePage ) {
        delete m_page;
    }
}

void KoPAPageDeleteCommand::redo()
{
    m_index = m_document->takePage( m_page );
    m_deletePage = true;
}

void KoPAPageDeleteCommand::undo()
{
    m_document->insertPage( m_page, m_index );
    m_deletePage = false;
}
