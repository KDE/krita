/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAPageInsertCommand.h"

#include <klocalizedstring.h>

#include "KoPADocument.h"
#include "KoPAPageBase.h"

KoPAPageInsertCommand::KoPAPageInsertCommand( KoPADocument *document, KoPAPageBase *page, KoPAPageBase *after, KUndo2Command *parent )
: KUndo2Command( parent )
, m_document( document )
, m_page( page )
, m_after( after )
, m_deletePage( true )
{
    Q_ASSERT( document );
    Q_ASSERT( page );
    if ( m_page->pageType() == KoPageApp::Slide ) {
        setText( kundo2_i18n( "Insert slide" ) );
    }
    else {
        setText( kundo2_i18n( "Insert page" ) );
    }
}

KoPAPageInsertCommand::~KoPAPageInsertCommand()
{
    if ( m_deletePage ) {
        delete m_page;
    }
}

void KoPAPageInsertCommand::redo()
{
    m_document->insertPage( m_page, m_after );
    m_deletePage = false;
}

void KoPAPageInsertCommand::undo()
{
    m_document->takePage( m_page );
    m_deletePage = true;
}
