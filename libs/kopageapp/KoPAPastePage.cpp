/* This file is part of the KDE project
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPAPastePage.h"

#include <QBuffer>
#include <QString>
#include <KoOdfReadStore.h>
#include <KoXmlWriter.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoEmbeddedDocumentSaver.h>
#include "KoPALoadingContext.h"
#include "KoPADocument.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "commands/KoPAPageInsertCommand.h"

#include <kdebug.h>

KoPAPastePage::KoPAPastePage( KoPADocument * doc, KoPAPageBase * activePage )
: m_doc( doc )
, m_activePage( activePage )
{
}

bool KoPAPastePage::process( const KoXmlElement & body, KoOdfReadStore & odfStore )
{
    KoOdfLoadingContext loadingContext( odfStore.styles(), odfStore.store(), m_doc->componentData() );
    KoPALoadingContext paContext( loadingContext, m_doc->dataCenterMap() );

    QList<KoPAPageBase *> masterPages( m_doc->loadOdfMasterPages( odfStore.styles().masterPages(), paContext ) );
    QList<KoPAPageBase *> pages( m_doc->loadOdfPages( body, paContext ) );

    KoPAPageBase * insertAfterPage = 0;
    KoPAPageBase * insertAfterMasterPage = 0;
    if ( dynamic_cast<KoPAMasterPage *>( m_activePage ) || ( m_activePage == 0 && pages.empty() ) ) {
        insertAfterMasterPage = m_activePage;
        insertAfterPage = m_doc->pages( false ).last();
    }
    else {
        insertAfterPage = m_activePage;
        insertAfterMasterPage = m_doc->pages( true ).last();
    }

    if ( ! pages.empty() ) {
        KoGenStyles mainStyles;
        QBuffer buffer;
        buffer.open( QIODevice::WriteOnly );
        KoXmlWriter xmlWriter( &buffer );
        KoEmbeddedDocumentSaver embeddedSaver;
        KoPASavingContext savingContext(xmlWriter, mainStyles, embeddedSaver, 1);
        savingContext.addOption( KoShapeSavingContext::UniqueMasterPages );
        QList<KoPAPageBase*> emptyList;
        QList<KoPAPageBase*> existingMasterPages = m_doc->pages( true );
        savingContext.setClearDrawIds( true );
        m_doc->saveOdfPages( savingContext, emptyList, existingMasterPages );

        QMap<QString, KoPAMasterPage*> masterPageNames;

        foreach ( KoPAPageBase * page, existingMasterPages )
        {
            KoPAMasterPage * masterPage = dynamic_cast<KoPAMasterPage*>( page );
            Q_ASSERT( masterPage );
            if ( masterPage ) {
                QString masterPageName( savingContext.masterPageName( masterPage ) );
                if ( !masterPageNames.contains( masterPageName ) ) {
                    masterPageNames.insert( masterPageName, masterPage );
                }
            }

        }

        m_doc->saveOdfPages( savingContext, emptyList, masterPages );

        QMap<KoPAMasterPage*, KoPAMasterPage*> updateMasterPage;
        foreach ( KoPAPageBase * page, masterPages )
        {
            KoPAMasterPage * masterPage = dynamic_cast<KoPAMasterPage*>( page );
            Q_ASSERT( masterPage );
            if ( masterPage ) {
                QString masterPageName( savingContext.masterPageName( masterPage ) );
                QMap<QString, KoPAMasterPage*>::const_iterator existingMasterPage( masterPageNames.constFind( masterPageName ) );
                if ( existingMasterPage != masterPageNames.constEnd() ) {
                    updateMasterPage.insert( masterPage, existingMasterPage.value() );
                }
            }
        }

        // update pages which have a duplicate master page
        foreach ( KoPAPageBase * page, pages )
        {
            KoPAPage * p = dynamic_cast<KoPAPage*>( page );
            Q_ASSERT( p );
            if ( p ) {
                KoPAMasterPage * masterPage( p->masterPage() );
                QMap<KoPAMasterPage*, KoPAMasterPage*>::const_iterator pageIt( updateMasterPage.constFind( masterPage ) );
                if ( pageIt != updateMasterPage.constEnd() ) {
                    p->setMasterPage( pageIt.value() );
                }
            }
        }

        // delete dumplicate master pages;
        QMap<KoPAMasterPage*, KoPAMasterPage*>::const_iterator pageIt( updateMasterPage.constBegin() );
        for ( ; pageIt != updateMasterPage.constEnd(); ++pageIt )
        {
            masterPages.removeAll( pageIt.key() );
            delete pageIt.key();
        }
    }

    QUndoCommand * cmd = 0;
    if ( m_doc->pageType() == KoPageApp::Slide ) {
        cmd = new QUndoCommand( i18np( "Paste Slide", "Paste Slides", qMax( masterPages.size(), pages.size() ) ) );
    }
    else {
        cmd = new QUndoCommand( i18np( "Paste Page", "Paste Pages", qMax( masterPages.size(), pages.size() ) ) );
    }

    foreach( KoPAPageBase * masterPage, masterPages )
    {
        new KoPAPageInsertCommand( m_doc, masterPage, insertAfterMasterPage, cmd );
        insertAfterMasterPage = masterPage;
    }

    foreach( KoPAPageBase * page, pages )
    {
        new KoPAPageInsertCommand( m_doc, page, insertAfterPage, cmd );
        insertAfterPage = page;
    }

    m_doc->addCommand( cmd );

    return true;
}
