/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoPAPageMoveCommand.h"

#include <klocalizedstring.h>

#include "KoPADocument.h"
#include "KoPAPageBase.h"

KoPAPageMoveCommand::KoPAPageMoveCommand( KoPADocument *document, KoPAPageBase *page, KoPAPageBase *after, KUndo2Command *parent )
: KUndo2Command( parent )
, m_document( document )
, m_after( after )
{
    Q_ASSERT( document );
    init( QList<KoPAPageBase *>() << page );
}

KoPAPageMoveCommand::KoPAPageMoveCommand( KoPADocument *document, const QList<KoPAPageBase *> &pages, KoPAPageBase *after, KUndo2Command *parent )
: KUndo2Command( parent )
, m_document( document )
, m_after( after )
{
    Q_ASSERT( document );

    init( pages );
}

// Since C++ constructor can't call another constructor ...
void KoPAPageMoveCommand::init( const QList<KoPAPageBase *> &pages )
{
    Q_ASSERT( pages.size() > 0 );

    foreach ( KoPAPageBase *page, pages ) {
        int index = m_document->pageIndex( page );
        m_pageIndices.insert( index, page );
    }

    if ( pages.at(0)->pageType() == KoPageApp::Slide ) {
        setText( kundo2_i18np( "Move slide", "Move slides", pages.size() ) );
    }
    else {
        setText( kundo2_i18np( "Move page", "Move pages", pages.size() ) );
    }
}

KoPAPageMoveCommand::~KoPAPageMoveCommand()
{
}

void KoPAPageMoveCommand::redo()
{
    KoPAPageBase *after = m_after;
    QList<KoPAPageBase *> pages = m_pageIndices.values();

    foreach ( KoPAPageBase *page, pages ) {
        m_document->takePage( page );
        m_document->insertPage( page, after );
        after = page;
    }

}

void KoPAPageMoveCommand::undo()
{
    QList<KoPAPageBase *> pages = m_pageIndices.values();
    foreach ( KoPAPageBase *page, pages ) {
        m_document->takePage( page );
    }

    QMap<int, KoPAPageBase *>::const_iterator it;
    for ( it = m_pageIndices.constBegin(); it != m_pageIndices.constEnd(); ++it ) {
        m_document->insertPage( it.value(), it.key() );
    }
}

