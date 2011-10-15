/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
                 2009 Jeremias Epperlein <jeeree@web.de>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoFormulaShape.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kguiitem.h>

#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoDocumentEntry.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoDocumentResourceManager.h>

#include "FormulaDocument.h"
#include "FormulaData.h"
#include "FormulaElement.h"
#include "FormulaRenderer.h"


KoFormulaShape::KoFormulaShape(KoDocumentResourceManager *documentResourceManager)
  : KoFrameShape( KoXmlNS::draw, "object" )
{
    FormulaElement* element= new FormulaElement();
    m_formulaData = new FormulaData(element);
    m_formulaRenderer = new FormulaRenderer();
    m_isInline = false;

    m_document = new FormulaDocument( this );
    m_resourceManager = documentResourceManager;
}

KoFormulaShape::~KoFormulaShape()
{
    delete m_formulaData;
    delete m_formulaRenderer;
}

void KoFormulaShape::paint( QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
    painter.save();
    applyConversion( painter, converter );   // apply zooming and coordinate translation
    m_formulaRenderer->layoutElement(  m_formulaData->formulaElement() );
    m_formulaRenderer->paintElement( painter,  m_formulaData->formulaElement() );  // paint the formula
    painter.restore();
}

void KoFormulaShape::updateLayout() {
    m_formulaRenderer->layoutElement( m_formulaData->formulaElement() );

     KoShape::setSize(m_formulaData->formulaElement()->boundingRect().size());
}


BasicElement* KoFormulaShape::elementAt( const QPointF& p )
{
    return m_formulaData->formulaElement()->childElementAt( p );
}

void KoFormulaShape::resize( const QSizeF& )
{ /* do nothing as FormulaShape is fixed size */ }

FormulaData* KoFormulaShape::formulaData() const
{
    return  m_formulaData;
}

FormulaRenderer* KoFormulaShape::formulaRenderer() const
{
    return m_formulaRenderer;
}

bool KoFormulaShape::loadOdf( const KoXmlElement& element, KoShapeLoadingContext &context )
{
    kDebug() <<"Loading ODF in Formula";
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

bool KoFormulaShape::loadOdfFrameElement(const KoXmlElement &element,
                                         KoShapeLoadingContext &context)
{
    // If this formula is embedded and not inline, then load the embedded document.
    if ( element.tagName() == "object" && element.hasAttributeNS( KoXmlNS::xlink, "href" )) {
        m_isInline = false;

        // This calls loadOdfEmbedded().
        return loadEmbeddedDocument( context.odfLoadingContext().store(),
                                     element,
                                     context.odfLoadingContext() );
    }

    // It's not a frame:object, so it must be inline.
    const KoXmlElement& topLevelElement = KoXml::namedItemNS(element, KoXmlNS::math, "math");
    if (topLevelElement.isNull()) {
        kWarning() << "no math element as first child";
        return false;
    }

    // Create a new root element, load the formula and replace the old one.
    FormulaElement* formulaElement = new FormulaElement();
    formulaElement->readMathML( topLevelElement );
    delete m_formulaData->formulaElement();
    m_formulaData->setFormulaElement(formulaElement);
    m_formulaData->notifyDataChange(0, false);

    m_isInline = true;

    return true;
}

bool KoFormulaShape::loadEmbeddedDocument( KoStore *store,
                                           const KoXmlElement &objectElement,
                                           const KoOdfLoadingContext &odfLoadingContext)
{
    if ( !objectElement.hasAttributeNS( KoXmlNS::xlink, "href" ) ) {
        kError() << "Object element has no valid xlink:href attribute";
        return false;
    }

    QString url = objectElement.attributeNS( KoXmlNS::xlink, "href" );

    // It can happen that the url is empty e.g. when it is a
    // presentation:placeholder.
    if ( url.isEmpty() ) {
        return true;
    }

    QString tmpURL;
    if ( url[0] == '#' )
        url = url.mid( 1 );

#define INTERNAL_PROTOCOL "intern"
#define STORE_PROTOCOL "tar"

    if (KUrl::isRelativeUrl( url )) {
        if ( url.startsWith( "./" ) )
            tmpURL = QString( INTERNAL_PROTOCOL ) + ":/" + url.mid( 2 );
        else
            tmpURL = QString( INTERNAL_PROTOCOL ) + ":/" + url;
    }
    else
        tmpURL = url;

    QString path = tmpURL;
    if ( tmpURL.startsWith( INTERNAL_PROTOCOL ) ) {
        path = store->currentDirectory();
        if ( !path.isEmpty() && !path.endsWith( '/' ) )
            path += '/';
        QString relPath = KUrl( tmpURL ).path();
        path += relPath.mid( 1 ); // remove leading '/'
    }
    if ( !path.endsWith( '/' ) )
        path += '/';

    const QString mimeType = odfLoadingContext.mimeTypeForPath( path );
    //kDebug(35001) << "path for manifest file=" << path << "mimeType=" << mimeType;
    if ( mimeType.isEmpty() ) {
        //kDebug(35001) << "Manifest doesn't have media-type for" << path;
        return false;
    }

    const bool isOdf = mimeType.startsWith( "application/vnd.oasis.opendocument" );
    if ( !isOdf ) {
        tmpURL += "/maindoc.xml";
        //kDebug(35001) << "tmpURL adjusted to" << tmpURL;
    }

    //kDebug(35001) << "tmpURL=" << tmpURL;
    QString errorMsg;
    KoDocumentEntry e = KoDocumentEntry::queryByMimeType( mimeType );
    if ( e.isEmpty() ) {
        return false;
    }

    bool res = true;
    if ( tmpURL.startsWith( STORE_PROTOCOL )
         || tmpURL.startsWith( INTERNAL_PROTOCOL )
         || KUrl::isRelativeUrl( tmpURL ) )
    {
        if ( isOdf ) {
            store->pushDirectory();
            Q_ASSERT( tmpURL.startsWith( INTERNAL_PROTOCOL ) );
            QString relPath = KUrl( tmpURL ).path().mid( 1 );
            store->enterDirectory( relPath );
            res = m_document->loadOasisFromStore( store );
            store->popDirectory();
        } else {
            if ( tmpURL.startsWith( INTERNAL_PROTOCOL ) )
                tmpURL = KUrl( tmpURL ).path().mid( 1 );
            res = m_document->loadFromStore( store, tmpURL );
        }
        m_document->setStoreInternal( true );
    }
    else {
        // Reference to an external document. Hmmm...
        m_document->setStoreInternal( false );
        KUrl url( tmpURL );
        if ( !url.isLocalFile() ) {
            //QApplication::restoreOverrideCursor();

            // For security reasons we need to ask confirmation if the
            // url is remote.
            int result = KMessageBox::warningYesNoCancel(
                0, i18n( "This document contains an external link to a remote document\n%1", tmpURL ),
                i18n( "Confirmation Required" ), KGuiItem( i18n( "Download" ) ), KGuiItem( i18n( "Skip" ) ) );

            if ( result == KMessageBox::Cancel ) {
                //d->m_parent->setErrorMessage("USER_CANCELED");
                return false;
            }
            if ( result == KMessageBox::Yes )
                res = m_document->openUrl( url );
            // and if == No, res will still be false so we'll use a kounavail below
        }
        else
            res = m_document->openUrl( url );
    }

    if ( !res ) {
        QString errorMessage = m_document->errorMessage();
        return false;
    }

    tmpURL.clear();

    return res;
}

bool KoFormulaShape::loadOdfEmbedded( const KoXmlElement &topLevelElement,
                                      KoShapeLoadingContext &context )
{
    Q_UNUSED(context);
    kDebug(31000) << topLevelElement.nodeName();

#if 0
    const KoXmlElement &topLevelElement = KoXml::namedItemNS(element, "http://www.w3.org/1998/Math/MathML", "math");
    if (topLevelElement.isNull()) {
        kWarning() << "no math element as first child";
        return false;
    }
#endif
    // Create a new root element, load the formula and replace the old one.
    FormulaElement* formulaElement = new FormulaElement();
    formulaElement->readMathML( topLevelElement );
    delete m_formulaData->formulaElement();
    m_formulaData->setFormulaElement(formulaElement);
    m_formulaData->notifyDataChange(0, false);

    return true;
}


void KoFormulaShape::saveOdf( KoShapeSavingContext& context ) const
{
    // FIXME: Add saving of embedded document if m_isInline is false;

    kDebug() <<"Saving ODF in Formula";
    KoXmlWriter& writer = context.xmlWriter();
    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement( "draw:object" );
    // TODO add some namespace magic to avoid adding "math:" namespace everywhere
    formulaData()->formulaElement()->writeMathML( &context.xmlWriter() );
    writer.endElement(); // draw:object
    writer.endElement(); // draw:frame
}

KoDocumentResourceManager *KoFormulaShape::resourceManager() const
{
    return m_resourceManager;
}
