/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Jarosław Staniek <staniek@kde.org>

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

#include "KoOdfExporter.h"

#include <QBuffer>
#include <QByteArray>

#include <KDebug>

#include <KoOdfWriteStore.h>
#include <KoStoreDevice.h>
#include <KoFilterChain.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>

#include <memory>

KoOdfWriters::KoOdfWriters()
 : content(0), body(0), meta(0), manifest(0), mainStyles(0)
{
}

//------------------------------------------

class KoOdfExporter::Private
{
public:
    Private() {}
    QByteArray bodyContentElement;
};

//------------------------------------------

KoOdfExporter::KoOdfExporter( const QString& bodyContentElement, QObject* parent )
 : KoFilter( parent )
 , d( new Private )
{
    d->bodyContentElement = QByteArray("office:") + bodyContentElement.toLatin1();
}

KoOdfExporter::~KoOdfExporter()
{
    delete d;
}

KoFilter::ConversionStatus KoOdfExporter::convert( const QByteArray& from, const QByteArray& to )
{
    // check for proper conversion
    if (!acceptsSourceMimeType(from)) {
        kWarning(30003) << "Invalid source mimetype" << from;
        return KoFilter::NotImplemented;
    }
    if (!acceptsDestinationMimeType(to)) {
        kWarning(30003) << "Invalid destination mimetype" << to;
        return KoFilter::NotImplemented;
    }

    //create output files
    std::auto_ptr<KoStore> outputStore(
        KoStore::createStore( m_chain->outputFile(), KoStore::Write, to, KoStore::Zip ) );
    if ( !outputStore.get() ) {
        kWarning(30003) << "Unable to open output file!";
        return KoFilter::FileNotFound;
    }
    outputStore->disallowNameExpansion();
    kDebug(30003) << "created outputStore.";
    KoOdfWriteStore oasisStore( outputStore.get() );

    kDebug(30003) << "created oasisStore.";

    // KoGenStyles for writing styles while we're parsing
    KoGenStyles mainStyles;

    KoOdfWriters writers;
    writers.mainStyles = &mainStyles;

    // create a writer for meta.xml
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    KoXmlWriter metaWriter(&buf);
    writers.meta = &metaWriter;

    // create a writer for manifest.xml
    QBuffer manifestBuf;
    manifestBuf.open(QIODevice::WriteOnly);
    KoXmlWriter manifestWriter(&manifestBuf);
    writers.manifest = &manifestWriter;

    //open contentWriter & bodyWriter *temp* writers
    //so we can write picture files while we parse
    QBuffer contentBuf;
    KoXmlWriter contentWriter(&contentBuf);
    writers.content = &contentWriter;
    QBuffer bodyBuf;
    KoXmlWriter bodyWriter(&bodyBuf);
    writers.body = &bodyWriter;

    // open main tags
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(d->bodyContentElement.constData());

    const KoFilter::ConversionStatus result 
        = createDocument(outputStore.get(), &writers);
    if ( result != KoFilter::OK )
        return result;

    //save the office:automatic-styles & and fonts in content.xml
    mainStyles.saveOdfFontFaceDecls(&contentWriter);
    mainStyles.saveOdfAutomaticStyles(&contentWriter, false);

    //close tags in body
    bodyWriter.endElement();//office:*
    bodyWriter.endElement();//office:body

    //now create real content/body writers & dump the information there
    KoXmlWriter* realContentWriter = oasisStore.contentWriter();
    realContentWriter->addCompleteElement(&contentBuf);
    KoXmlWriter* realBodyWriter = oasisStore.bodyWriter();
    realBodyWriter->addCompleteElement(&bodyBuf);

    //now close content & body writers
    if ( !oasisStore.closeContentWriter() ) {
        kWarning(30003) << "Error closing content.";
        return KoFilter::CreationError;
    }

    kDebug(30003) << "closed content & body writers.";

    //create the manifest file
    KoXmlWriter* realManifestWriter = oasisStore.manifestWriter( to );
    //create the styles.xml file
    mainStyles.saveOdfStylesDotXml( outputStore.get(), realManifestWriter );
    realManifestWriter->addManifestEntry( "content.xml", "text/xml" );
    realManifestWriter->addCompleteElement(&manifestBuf);

    kDebug(30003) << "created manifest and styles.xml";

    //create meta.xml
    if (!outputStore->open("meta.xml")) {
        return KoFilter::CreationError;
    }
    KoStoreDevice metaDev(outputStore.get());
    KoXmlWriter* meta = KoOdfWriteStore::createOasisXmlWriter(&metaDev, "office:document-meta");
    meta->startElement("office:meta");
    meta->addCompleteElement(&buf);
    meta->endElement(); //office:meta
    meta->endElement(); //office:document-meta
    meta->endDocument();
    if (!outputStore->close()) {
        return KoFilter::CreationError;
    }
    realManifestWriter->addManifestEntry("meta.xml", "text/xml");
    oasisStore.closeManifestWriter();

    return KoFilter::OK;
}
