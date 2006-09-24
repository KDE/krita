/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOasisStore.h"

#include "KoDocument.h"
#include "KoXmlNS.h"
#include "KoDom.h"
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include <ktempfile.h>
#include <kdebug.h>
#include <klocale.h>

#include <QFile>
#include <QtXml>
#include <QBuffer>

KoOasisStore::KoOasisStore( KoStore* store )
    : m_store( store ),
      m_storeDevice( 0 ),
      m_contentWriter( 0 ),
      m_bodyWriter( 0 ),
      m_manifestWriter( 0 ),
      m_contentTmpFile( 0 )
{
}

KoOasisStore::~KoOasisStore()
{
    // If all the right close methods were called, nothing should remain,
    // so those deletes are really just in case.
    Q_ASSERT( !m_contentWriter );
    delete m_contentWriter;
    Q_ASSERT( !m_bodyWriter );
    delete m_bodyWriter;
    Q_ASSERT( !m_storeDevice );
    delete m_storeDevice;
    Q_ASSERT( !m_contentTmpFile );
    delete m_contentTmpFile;
    Q_ASSERT( !m_manifestWriter );
    delete m_manifestWriter;
}

KoXmlWriter* KoOasisStore::contentWriter()
{
    if ( !m_contentWriter )
    {
        if ( !m_store->open( "content.xml" ) )
            return 0;
        m_storeDevice = new KoStoreDevice( m_store );
        m_contentWriter = KoDocument::createOasisXmlWriter( m_storeDevice, "office:document-content" );
    }
    return m_contentWriter;
}

KoXmlWriter* KoOasisStore::bodyWriter()
{
    if ( !m_bodyWriter )
    {
        Q_ASSERT( !m_contentTmpFile );
        m_contentTmpFile = new KTempFile;
        m_contentTmpFile->setAutoDelete( true );
        m_bodyWriter = new KoXmlWriter( m_contentTmpFile->file(), 1 );
    }
    return m_bodyWriter;
}

bool KoOasisStore::closeContentWriter()
{
    Q_ASSERT( m_bodyWriter );
    Q_ASSERT( m_contentTmpFile );

    delete m_bodyWriter; m_bodyWriter = 0;
    // copy over the contents from the tempfile to the real one
    QFile* tmpFile = m_contentTmpFile->file();
    tmpFile->close();
    m_contentWriter->addCompleteElement( tmpFile );
    m_contentTmpFile->close();
    delete m_contentTmpFile; m_contentTmpFile = 0;

    Q_ASSERT( m_contentWriter );
    m_contentWriter->endElement(); // document-content
    m_contentWriter->endDocument();
    delete m_contentWriter; m_contentWriter = 0;
    delete m_storeDevice; m_storeDevice = 0;
    if ( !m_store->close() ) // done with content.xml
        return false;
    return true;
}

KoXmlWriter* KoOasisStore::manifestWriter( const char* mimeType )
{
    if ( !m_manifestWriter )
    {
        // the pointer to the buffer is already stored in the KoXmlWriter, no need to store it here as well
        QBuffer *manifestBuffer = new QBuffer;
        manifestBuffer->open( QIODevice::WriteOnly );
        m_manifestWriter = new KoXmlWriter( manifestBuffer );
        m_manifestWriter->startDocument( "manifest:manifest" );
        m_manifestWriter->startElement( "manifest:manifest" );
        m_manifestWriter->addAttribute( "xmlns:manifest", KoXmlNS::manifest );
        m_manifestWriter->addManifestEntry( "/", mimeType );
    }
    return m_manifestWriter;
}

bool KoOasisStore::closeManifestWriter()
{
    m_manifestWriter->endElement();
    m_manifestWriter->endDocument();
    QBuffer* buffer = static_cast<QBuffer *>( m_manifestWriter->device() );
    delete m_manifestWriter; m_manifestWriter = 0;
    bool ok = false;
    if ( m_store->open( "META-INF/manifest.xml" ) )
    {
        qint64 written = m_store->write( buffer->buffer() );
        ok = ( written == (qint64) buffer->buffer().size() && m_store->close() );
    }
    delete buffer;
    return ok;
}

bool KoOasisStore::loadAndParse( const QString& fileName, KoXmlDocument& doc, QString& errorMessage )
{
    //kDebug(30003) << "loadAndParse: Trying to open " << fileName << endl;

    if (!m_store->open(fileName))
    {
        kWarning(30003) << "Entry " << fileName << " not found!" << endl;
        errorMessage = i18n( "Could not find %1", fileName );
        return false;
    }
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;

    // We need to be able to see the space in <text:span> </text:span>, this is why
    // we activate the "report-whitespace-only-CharData" feature.
    // Unfortunately this leads to lots of whitespace text nodes in between real
    // elements in the rest of the document, watch out for that.
    QXmlInputSource source( m_store->device() );
    // Copied from QDomDocumentPrivate::setContent, to change the whitespace thing
    QXmlSimpleReader reader;
    KoDocument::setupXmlReader( reader, true /*namespaceProcessing*/ );

    bool ok = doc.setContent( &source, &reader, &errorMsg, &errorLine, &errorColumn );
    if ( !ok )
    {
        kError(30003) << "Parsing error in " << fileName << "! Aborting!" << endl
                       << " In line: " << errorLine << ", column: " << errorColumn << endl
                       << " Error message: " << errorMsg << endl;
        errorMessage = i18n( "Parsing error in the main document at line %1, column %2\nError message: %3" 
                       ,errorLine ,errorColumn ,i18n ( "QXml", errorMsg ) );
    }
    else
    {
        kDebug(30003) << "File " << fileName << " loaded and parsed" << endl;
    }
    m_store->close();
    return ok;
}

QString KoOasisStore::mimeForPath( const KoXmlDocument& doc, const QString& fullPath )
{
    KoXmlElement docElem = doc.documentElement();
    KoXmlElement elem;
    forEachElement( elem, docElem )
    {
        if ( elem.localName() == "file-entry" && elem.namespaceURI() == KoXmlNS::manifest )
        {
            if ( elem.attributeNS( KoXmlNS::manifest, "full-path", QString::null ) == fullPath )
                return elem.attributeNS( KoXmlNS::manifest, "media-type", QString::null );
        }
    }
    return QString();
}
