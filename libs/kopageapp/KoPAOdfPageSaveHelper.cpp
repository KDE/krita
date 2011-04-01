/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAOdfPageSaveHelper.h"

#include <QSet>

#include <KoXmlWriter.h>
#include "KoPADocument.h"
#include "KoPAPage.h"
#include "KoPASavingContext.h"
#include "KoPAMasterPage.h"

KoPAOdfPageSaveHelper::KoPAOdfPageSaveHelper( KoPADocument * doc, QList<KoPAPageBase *> pages )
    : m_doc(doc),
    m_context(0)
{
    foreach( KoPAPageBase * page, pages ) {
        if ( dynamic_cast<KoPAPage *>( page ) ) {
            m_pages.append( page );
        }
        else {
            m_masterPages.append( page );
        }
    }

    if ( m_pages.size() > 0 ) {
        m_masterPages.clear();

        // this might result in a different order of master pages when copying to a different document
        QSet<KoPAPageBase *> masterPages;
        foreach( KoPAPageBase * page, m_pages ) {
            KoPAPage * p = dynamic_cast<KoPAPage *>( page );
            masterPages.insert( p->masterPage() );
        }
        m_masterPages = masterPages.toList();
    }
}

KoPAOdfPageSaveHelper::~KoPAOdfPageSaveHelper()
{
    delete m_context;
}

KoShapeSavingContext * KoPAOdfPageSaveHelper::context( KoXmlWriter * bodyWriter, KoGenStyles & mainStyles, KoEmbeddedDocumentSaver & embeddedSaver )
{
    m_context = new KoPASavingContext( *bodyWriter, mainStyles, embeddedSaver, 1 );
    return m_context;
}

bool KoPAOdfPageSaveHelper::writeBody()
{
    Q_ASSERT( m_context );
    if ( m_context ) {
        m_doc->saveOdfDocumentStyles( *( static_cast<KoPASavingContext*>( m_context ) ) );
        KoXmlWriter & bodyWriter = static_cast<KoPASavingContext*>( m_context )->xmlWriter();
        bodyWriter.startElement( "office:body" );
        bodyWriter.startElement( m_doc->odfTagName( true ) );

        if ( !m_doc->saveOdfPages( *( static_cast<KoPASavingContext*>( m_context ) ), m_pages, m_masterPages ) ) {
            return false;
        }

        bodyWriter.endElement(); // office:odfTagName()
        bodyWriter.endElement(); // office:body

        return true;
    }
    return false;
}
