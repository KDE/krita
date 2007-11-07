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

#include "KoDrag.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include <QString>

#include <KoStore.h>
#include <KoGenStyles.h>
#include <KoOdfWriteStore.h>
#include <KoXmlWriter.h>
#include "KoShapeSavingContext.h"

#include "KoDragOdfSaveHelper.h"

KoDrag::KoDrag()
: m_mimeData( 0 )
{
}

KoDrag::~KoDrag()
{
    if ( m_mimeData == 0 ) {
        delete m_mimeData;
    }
}

bool KoDrag::setOdf( const char * mimeType, KoDragOdfSaveHelper &helper )
{
    QBuffer buffer;
    KoStore* store = KoStore::createStore( &buffer, KoStore::Write, mimeType );
    Q_ASSERT( store );
    Q_ASSERT( !store->bad() );

    KoOdfWriteStore oasisStore( store );

    KoXmlWriter* manifestWriter = oasisStore.manifestWriter( mimeType );
    KoXmlWriter* contentWriter = oasisStore.contentWriter();

    // TODO delete store on all exits
    if ( !contentWriter ) {
        return false;
    }

    KoGenStyles mainStyles;
    KoXmlWriter* bodyWriter = oasisStore.bodyWriter();
    KoShapeSavingContext * context = helper.context( bodyWriter, mainStyles );

    if ( !helper.writeBody() ) {
        return false;
    }

    mainStyles.saveOdfAutomaticStyles( contentWriter, false );

    oasisStore.closeContentWriter();

    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if ( !mainStyles.saveOdfStylesDotXml( store, manifestWriter ) ) {
        return false;
    }

    // Write out manifest file
    if ( !oasisStore.closeManifestWriter() ) {
        return false;
    }

    delete store;

    setData( mimeType, buffer.buffer() );

    return true;
}

void KoDrag::setData( const QString & mimeType, const QByteArray & data )
{
    if ( m_mimeData == 0 ) {
        m_mimeData = new QMimeData();
    }
    m_mimeData->setData( mimeType, data );
}

void KoDrag::addToClipboard()
{
    if ( m_mimeData ) {
        QApplication::clipboard()->setMimeData( m_mimeData );
        m_mimeData = 0;
    }
}
