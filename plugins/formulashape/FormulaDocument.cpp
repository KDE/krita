/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2010 Inge Wallin <inge@lysator.liu.se>

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
#include "FormulaDocument.h"

// Qt
#include <QWidget>
#include <QIODevice>
#include <QDebug>
#include <QPainter>

// Calligra
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

// KFormula
#include "KoFormulaShape.h"
#include "FormulaPart.h"

class FormulaDocument::Private
{
public:
    Private();
    ~Private();

    KoFormulaShape *parent;
};

FormulaDocument::Private::Private()
{
}

FormulaDocument::Private::~Private()
{
}

FormulaDocument::FormulaDocument( KoFormulaShape *parent )
    : KoDocument(new FormulaPart(0))
    , d ( new Private )
{
    d->parent = parent;
}

FormulaDocument::~FormulaDocument()
{
    delete d;
}


bool FormulaDocument::loadOdf( KoOdfReadStore &odfStore )
{
    KoXmlDocument doc = odfStore.contentDoc();
    KoXmlElement  bodyElement = doc.documentElement();

    kDebug(31000) << bodyElement.nodeName();

    if (bodyElement.localName() != "math" || bodyElement.namespaceURI() != KoXmlNS::math) {
        kError(35001) << "No <math:math> element found.";
        return false;
    }

    // When the formula is stored in an embedded document, it seems to
    // always have a <math:semantics> element that surrounds the
    // actual formula.  I have to check with the MathML spec what this
    // actually means and if it is obligatory.  /iw
    KoXmlNode semanticsNode = bodyElement.namedItemNS( KoXmlNS::math, "semantics" );
    if ( !semanticsNode.isNull() ) {
        bodyElement = semanticsNode.toElement();
    }

    KoOdfLoadingContext   odfLoadingContext( odfStore.styles(), odfStore.store() );
    KoShapeLoadingContext context(odfLoadingContext, d->parent->resourceManager());

    return d->parent->loadOdfEmbedded( bodyElement, context );
}

bool FormulaDocument::loadXML( const KoXmlDocument &doc, KoStore *)
{
    Q_UNUSED( doc );

    // We don't support the old XML format any more.
    return false;
}

bool FormulaDocument::saveOdf( SavingContext &context )
{
    // FIXME: This code is copied from ChartDocument, so it needs to
    // be adapted to the needs of the KoFormulaShape.

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
    bodyWriter->startElement( "office:formula" );

    d->parent->saveOdf( savingContext );

    bodyWriter->endElement(); // office:formula
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

void FormulaDocument::paintContent( QPainter &painter, const QRect &rect )
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}


