/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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

// Own
#include "ChartDocument.h"

// Qt
#include <QWidget>
#include <QIODevice>
#include <QDebug>
#include <QPainter>

// KOffice
#include <KoDocument.h>
#include <KoXmlWriter.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoGenStyles.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoView.h>
#include <KComponentData>
#include <KDebug>

// KChart
#include "ChartShape.h"


namespace KChart {

class ChartDocument::Private
{
public:
    Private();
    ~Private();

    ChartShape *parent;
};

ChartDocument::Private::Private()
{
}

ChartDocument::Private::~Private()
{
}

ChartDocument::ChartDocument( ChartShape *parent )
    : KoDocument( 0, 0 )
    , d ( new Private )
{
    d->parent = parent;
    // Needed by KoDocument::nativeOasisMimeType().
    // KoEmbeddedDocumentSaver uses that method to
    // get the mimetype of the embedded document.
    setComponentData( KComponentData( "kchart" ) );
}

ChartDocument::~ChartDocument()
{
    delete d;
}


bool ChartDocument::loadOdf( KoOdfReadStore &odfStore )
{
    KoXmlDocument doc = odfStore.contentDoc();
    KoXmlNode bodyNode = doc.documentElement().namedItemNS( KoXmlNS::office, "body" );
    if ( bodyNode.isNull() ) {
        kError(35001) << "No <office:body> element found.";
        return false;
    }
    KoXmlNode chartElementParentNode = bodyNode.namedItemNS( KoXmlNS::office, "chart" );
    if ( chartElementParentNode.isNull() ) {
        kError(35001) << "No <office:chart> element found.";
        return false;
    }
    KoXmlElement chartElement = chartElementParentNode.namedItemNS( KoXmlNS::chart, "chart" ).toElement();
    if ( chartElement.isNull() ) {
        kError(35001) << "No <chart:chart> element found.";
        return false;
    }
    KoOdfLoadingContext odfLoadingContext( odfStore.styles(), odfStore.store() );
    KoShapeLoadingContext context(odfLoadingContext, d->parent->resourceManager());

    return d->parent->loadOdfEmbedded( chartElement, context );
}

bool ChartDocument::loadXML( const KoXmlDocument &doc, KoStore *)
{
    Q_UNUSED( doc );

    // We don't support the old XML format any more.
    return false;
}

bool ChartDocument::saveOdf( SavingContext &context )
{
    KoOdfWriteStore &odfStore = context.odfStore;
    KoStore *store = odfStore.store();
    KoXmlWriter *manifestWriter = odfStore.manifestWriter();
    KoXmlWriter *contentWriter  = odfStore.contentWriter();
    if ( !contentWriter )
        return false;

    KoGenStyles mainStyles;
    KoXmlWriter *bodyWriter = odfStore.bodyWriter();
    if ( !bodyWriter )
        return false;

    KoEmbeddedDocumentSaver& embeddedSaver = context.embeddedSaver;

    KoShapeSavingContext savingContext( *bodyWriter, mainStyles, embeddedSaver );

    bodyWriter->startElement( "office:body" );
    bodyWriter->startElement( "office:chart" );

    d->parent->saveOdf( savingContext );

    bodyWriter->endElement(); // office:chart
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles( KoGenStyles::DocumentAutomaticStyles, contentWriter );
    odfStore.closeContentWriter();

    // Add manifest line for content.xml and styles.xml
    manifestWriter->addManifestEntry( url().path() + "/content.xml", "text/xml" );
    manifestWriter->addManifestEntry( url().path() + "/styles.xml", "text/xml" );

    // save the styles.xml
    if ( !mainStyles.saveOdfStylesDotXml( store, manifestWriter ) )
        return false;

    if ( !savingContext.saveDataCenter( store, manifestWriter ) ) {
        return false;
    }

    return true;
}

KoView *ChartDocument::createViewInstance( QWidget *parent )
{
    Q_UNUSED( parent );

    return 0;
}

void ChartDocument::paintContent( QPainter &painter, const QRect &rect )
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}

} // namespace KChart

