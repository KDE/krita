/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoOdfWriteStore.h"

#include <QBuffer>

#include <ktemporaryfile.h>
#include <kdebug.h>
#include <klocale.h>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include "KoDocument.h"
#include "KoXmlNS.h"

struct KoOdfWriteStore::Private
{
    Private( KoStore * store )
    : store( store )
    , storeDevice( 0 )
    , contentWriter( 0 )
    , bodyWriter( 0 )
    , manifestWriter( 0 )
    , contentTmpFile( 0 )
    {}

    ~Private()
    {
        // If all the right close methods were called, nothing should remain,
        // so those deletes are really just in case.
        Q_ASSERT( !contentWriter );
        delete contentWriter;
        Q_ASSERT( !bodyWriter );
        delete bodyWriter;
        Q_ASSERT( !storeDevice );
        delete storeDevice;
        Q_ASSERT( !contentTmpFile );
        delete contentTmpFile;
        Q_ASSERT( !manifestWriter );
        delete manifestWriter;
    }

    KoStore * store;
    KoStoreDevice * storeDevice;
    KoXmlWriter * contentWriter;
    KoXmlWriter * bodyWriter;
    KoXmlWriter * manifestWriter;
    KTemporaryFile * contentTmpFile;
};

KoOdfWriteStore::KoOdfWriteStore( KoStore* store )
: d( new Private( store ) )
{
}

KoOdfWriteStore::~KoOdfWriteStore()
{
    delete d;
}

KoStore* KoOdfWriteStore::store() const
{
    return d->store;
}

KoXmlWriter* KoOdfWriteStore::contentWriter()
{
    if ( !d->contentWriter ) {
        if ( !d->store->open( "content.xml" ) ) {
            return 0;
        }
        d->storeDevice = new KoStoreDevice( d->store );
        d->contentWriter = KoDocument::createOasisXmlWriter( d->storeDevice, "office:document-content" );
    }
    return d->contentWriter;
}

KoXmlWriter* KoOdfWriteStore::bodyWriter()
{
    if ( !d->bodyWriter ) {
        Q_ASSERT( !d->contentTmpFile );
        d->contentTmpFile = new KTemporaryFile;
        d->contentTmpFile->open();
        d->bodyWriter = new KoXmlWriter( d->contentTmpFile, 1 );
    }
    return d->bodyWriter;
}

bool KoOdfWriteStore::closeContentWriter()
{
    Q_ASSERT( d->bodyWriter );
    Q_ASSERT( d->contentTmpFile );

    delete d->bodyWriter; d->bodyWriter = 0;
    // copy over the contents from the tempfile to the real one
    d->contentTmpFile->close();
    d->contentWriter->addCompleteElement( d->contentTmpFile );
    d->contentTmpFile->close();
    delete d->contentTmpFile; d->contentTmpFile = 0;

    Q_ASSERT( d->contentWriter );
    d->contentWriter->endElement(); // document-content
    d->contentWriter->endDocument();
    delete d->contentWriter; d->contentWriter = 0;
    delete d->storeDevice; d->storeDevice = 0;
    if ( !d->store->close() ) { // done with content.xml
        return false;
    }
    return true;
}

KoXmlWriter* KoOdfWriteStore::manifestWriter( const char* mimeType )
{
    if ( !d->manifestWriter ) {
        // the pointer to the buffer is already stored in the KoXmlWriter, no need to store it here as well
        QBuffer *manifestBuffer = new QBuffer;
        manifestBuffer->open( QIODevice::WriteOnly );
        d->manifestWriter = new KoXmlWriter( manifestBuffer );
        d->manifestWriter->startDocument( "manifest:manifest" );
        d->manifestWriter->startElement( "manifest:manifest" );
        d->manifestWriter->addAttribute( "xmlns:manifest", KoXmlNS::manifest );
        d->manifestWriter->addManifestEntry( "/", mimeType );
    }
    return d->manifestWriter;
}

bool KoOdfWriteStore::closeManifestWriter()
{
    Q_ASSERT(d->manifestWriter);
    d->manifestWriter->endElement();
    d->manifestWriter->endDocument();
    QBuffer* buffer = static_cast<QBuffer *>( d->manifestWriter->device() );
    delete d->manifestWriter; d->manifestWriter = 0;
    bool ok = false;
    if ( d->store->open( "META-INF/manifest.xml" ) ) {
        qint64 written = d->store->write( buffer->buffer() );
        ok = ( written == (qint64) buffer->buffer().size() && d->store->close() );
    }
    delete buffer;
    return ok;
}
