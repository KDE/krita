/* This file is part of the KDE project
 * Copyright (C) 2009 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoPAChangePageLayoutCommand.h"

#include <klocale.h>
#include <KDebug>

#include "KoPADocument.h"
#include "KoPAMasterPage.h"

KoPAChangePageLayoutCommand::KoPAChangePageLayoutCommand( KoPADocument *document, KoPAMasterPage *masterPage, const KoPageLayout &newPageLayout, bool applyToDocument, QUndoCommand *parent )
: QUndoCommand( parent)
, m_document( document )
, m_newPageLayout( newPageLayout )
{
    setText( i18n( "Set Page Layout" ) );

    if ( !applyToDocument ) {
        m_oldLayouts.insert( masterPage, masterPage->pageLayout() );
    }
    else {
        QList<KoPAPageBase *> masterPages = m_document->pages( true );
        foreach( KoPAPageBase *page, masterPages ) {
            KoPAMasterPage *masterPage = static_cast<KoPAMasterPage *>( page );
            m_oldLayouts.insert( masterPage, masterPage->pageLayout() );
        }
    }
}

KoPAChangePageLayoutCommand::~KoPAChangePageLayoutCommand()
{
}

void KoPAChangePageLayoutCommand::redo()
{
    QMap<KoPAMasterPage *, KoPageLayout>::const_iterator it = m_oldLayouts.constBegin();
    while ( it != m_oldLayouts.constEnd() ) {
        it.key()->setPageLayout( m_newPageLayout );
        m_document->updateViews( it.key() );
        it++;
    }
}

void KoPAChangePageLayoutCommand::undo()
{
    QMap<KoPAMasterPage *, KoPageLayout>::const_iterator it = m_oldLayouts.constBegin();
    while ( it != m_oldLayouts.constEnd() ) {
        it.key()->setPageLayout( it.value() );
        m_document->updateViews( it.key() );
        it++;
    }
}

