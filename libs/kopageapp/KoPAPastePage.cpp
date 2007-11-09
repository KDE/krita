/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
#include <QByteArray>
#include <QMimeData>
#include <QString>
#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoXmlReader.h>
#include <KoDom.h>
#include <KoXmlNS.h>
#include <KoOasisLoadingContext.h>
#include <KoOasisStyles.h>
#include "KoPALoadingContext.h"
#include "KoPADocument.h"
#include "KoPAMasterPage.h"
#include "commands/KoPAPageInsertCommand.h"

#include <kdebug.h>

KoPAPastePage::KoPAPastePage( KoPADocument * doc, KoPAPageBase * activePage )
: m_doc( doc )
, m_activePage( activePage )
{
}

bool KoPAPastePage::paste( const QString & mimeType, const QMimeData * data )
{
    QByteArray arr = data->data( mimeType );
    bool retval = false;
    if ( !arr.isEmpty() ) {
        QBuffer buffer( &arr );
        KoStore * store = KoStore::createStore( &buffer, KoStore::Read );
        KoOdfReadStore odfStore( store );

        QString errorMessage;
        if ( ! odfStore.loadAndParse( errorMessage ) ) {
            kError() << "loading and parsing failed:" << errorMessage << endl;
            return false;
        }

        KoOasisLoadingContext loadingContext( m_doc, odfStore.styles(), odfStore.store() );
        KoPALoadingContext paContext( loadingContext );

        KoXmlElement content = odfStore.contentDoc().documentElement();
        KoXmlElement realBody( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );

        if ( realBody.isNull() ) {
            kError() << "No body tag found!" << endl;
            return false;
        }

        KoXmlElement body = KoDom::namedItemNS( realBody, KoXmlNS::office, "presentation" );

        if ( body.isNull() ) {
            body = KoDom::namedItemNS( realBody, KoXmlNS::office, "drawing" );
            if ( body.isNull() ) {
                kError() << "No office:presentation nor office:drawing tag found!" << endl;
                return false;
            }
        }

        QList<KoPAPageBase *> masterPages( m_doc->loadOdfMasterPages( odfStore.styles().masterPages(), paContext ) );
        QList<KoPAPageBase *> pages( m_doc->loadOdfPages( body, paContext ) );

        KoPAPageBase * insertAfterPage = 0;
        KoPAPageBase * insertAfterMasterPage = 0;
        if ( dynamic_cast<KoPAMasterPage *>( m_activePage ) ) {
            insertAfterMasterPage = m_activePage;
            insertAfterPage = m_doc->pages( false ).last();
        }
        else {
            insertAfterPage = m_activePage;
            insertAfterMasterPage = m_doc->pages( true ).last();
        }

        QUndoCommand * cmd = new QUndoCommand( i18n( "Paste Page" ) );

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

        retval = true;
    }

    return retval;
}
