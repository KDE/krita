/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

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

#include "KoDocumentChild.h"
#include "KoOasisStore.h"
#include <KoDocument.h>
#include <KoQueryTrader.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kparts/partmanager.h>

#include <kmimetype.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <QApplication>
#include <assert.h>

// Define the protocol used here for embedded documents' URL
// This used to "store" but KUrl didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
#define INTERNAL_PROTOCOL "intern"
// Warning, keep it sync in koStore.cc and koDocument.cc

/**********************************************************
 *
 * KoDocumentChild
 *
 **********************************************************/

class KoDocumentChildPrivate
{
public:
  KoDocumentChildPrivate()
  {
  }
  ~KoDocumentChildPrivate()
  {
  }

  KoDocument *m_parent;
  KoDocument *m_doc;
  bool m_deleted;
};

KoDocumentChild::KoDocumentChild( KoDocument* parent, KoDocument* doc, const QRect& geometry )
    : KoChild( parent )
{
  d = new KoDocumentChildPrivate;
  d->m_parent = parent;
  d->m_doc = doc;
  setGeometry( geometry );
  d->m_deleted = false;
  if ( doc )
    doc->setStoreInternal( !doc->hasExternURL() );
}

KoDocumentChild::KoDocumentChild( KoDocument* parent )
    : KoChild( parent )
{
  d = new KoDocumentChildPrivate;
  d->m_parent = parent;
  d->m_doc = 0L;
  d->m_deleted = false;
}

void KoDocumentChild::setDocument( KoDocument *doc, const QRect &geometry )
{
  kDebug()<<k_funcinfo<<"doc: "<<doc->url().url()<<endl;
  d->m_doc = doc;
  setGeometry( geometry );

  updateMatrix();
}

KoDocument *KoDocumentChild::document() const
{
  return d->m_doc;
}

KoDocument* KoDocumentChild::parentDocument() const
{
  return d->m_parent;
}

KoDocument* KoDocumentChild::hitTest( const QPoint& p, KoView* view, const QMatrix& _matrix )
{
  if ( !region( _matrix ).contains( p ) || !d->m_doc )
    return 0L;

  QMatrix m( _matrix );
  m = matrix() * m;
  m.scale( xScaling(), yScaling() );

  return d->m_doc->hitTest( p, view, m );
}

void KoDocumentChild::loadOasis( const KoXmlElement &frameElement, const KoXmlElement& objectElement )
{
    double x, y, w, h;
    x = KoUnit::parseValue( frameElement.attributeNS( KoXmlNS::svg, "x", QString::null ) );
    y = KoUnit::parseValue( frameElement.attributeNS( KoXmlNS::svg, "y", QString::null ) );
    w = KoUnit::parseValue( frameElement.attributeNS( KoXmlNS::svg, "width", QString::null ) );
    h = KoUnit::parseValue( frameElement.attributeNS( KoXmlNS::svg, "height", QString::null ) );
    m_tmpGeometry = QRect((int)x, (int)y, (int)w, (int)h); // #### double->int conversion
    setGeometry(m_tmpGeometry);

    QString url = objectElement.attributeNS( KoXmlNS::xlink, "href", QString::null );
    if ( url[0] == '#' )
        url = url.mid( 1 );
    if ( url.startsWith( "./" ) )
        m_tmpURL = QString( INTERNAL_PROTOCOL ) + ":/" + url.mid( 2 );
    else
        m_tmpURL = url;
    kDebug() << k_funcinfo << m_tmpURL << endl;
}


bool KoDocumentChild::load( const KoXmlElement& element, bool uppercase )
{
    if ( element.hasAttribute( "url" ) )
        m_tmpURL = element.attribute("url");
    if ( element.hasAttribute("mime") )
        m_tmpMimeType = element.attribute("mime");

    if ( m_tmpURL.isEmpty() )
    {
        kDebug(30003) << "Empty 'url' attribute in OBJECT" << endl;
        return false;
    }
    if ( m_tmpMimeType.isEmpty() )
    {
        kDebug(30003) << "Empty 'mime' attribute in OBJECT" << endl;
        return false;
    }

    bool brect = false;
    KoXmlNode n = element.firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
    {
        KoXmlElement e = n.toElement();
        if ( e.isNull() ) continue;
        if ( e.tagName() == "rect" || ( uppercase && e.tagName() == "RECT" ) )
        {
            brect = true;
            int x, y, w, h;
            x=y=w=h=0;
            if ( e.hasAttribute( "x" ) )
                x = e.attribute( "x" ).toInt(&brect);
            if ( e.hasAttribute( "y" ) )
                y = e.attribute( "y" ).toInt(&brect);
            if ( e.hasAttribute( "w" ) )
                w = e.attribute( "w" ).toInt(&brect);
            if ( e.hasAttribute( "h" ) )
                h = e.attribute( "h" ).toInt(&brect);
            m_tmpGeometry = QRect(x, y, w, h);
            setGeometry(m_tmpGeometry);
        }
    }

    if ( !brect )
    {
        kDebug(30003) << "Missing RECT in OBJECT" << endl;
        return false;
    }

    return true;
}

bool KoDocumentChild::loadDocument( KoStore* store )
{
    assert( !m_tmpURL.isEmpty() );

    kDebug(30003) << "KoDocumentChild::loadDocument: trying to load " << m_tmpURL << endl;

    // Backwards compatibility
    if ( m_tmpMimeType == "application/x-killustrator" )
        m_tmpMimeType = "application/x-kontour";

    return createAndLoadDocument( store, true /*open url*/, false /*not oasis*/, m_tmpMimeType );
}

bool KoDocumentChild::loadOasisDocument( KoStore* store, const KoXmlDocument& manifestDoc )
{
    QString path = m_tmpURL;
    if ( m_tmpURL.startsWith( INTERNAL_PROTOCOL ) ) {
        path = store->currentDirectory();
        if ( !path.isEmpty() )
            path += '/';
        QString relPath = KUrl( m_tmpURL ).path();
        path += relPath.mid( 1 ); // remove leading '/'
    }
    if ( !path.endsWith( "/" ) )
        path += '/';
    const QString mimeType = KoOasisStore::mimeForPath( manifestDoc, path );
    kDebug() << k_funcinfo << "path for manifest file=" << path << " mimeType=" << mimeType << endl;
    if ( mimeType.isEmpty() ) {
        kError(30003) << "Manifest doesn't have media-type for " << path << endl;
        return false;
    }

    const bool oasis = mimeType.startsWith( "application/vnd.oasis.opendocument" );
    if ( !oasis ) {
        m_tmpURL += "/maindoc.xml";
        kDebug() << " m_tmpURL adjusted to " << m_tmpURL << endl;
    }
    return createAndLoadDocument( store, true /*open url*/, oasis, mimeType );
}

bool KoDocumentChild::createAndLoadDocument( KoStore* store, bool doOpenURL, bool oasis, const QString& mimeType )
{
    kDebug(30003) << "KoDocumentChild::loadDocumentInternal doOpenURL=" << doOpenURL << " m_tmpURL=" << m_tmpURL << endl;
    QString errorMsg;
    KoDocumentEntry e = KoDocumentEntry::queryByMimeType( mimeType );
    if ( e.isEmpty() )
    {
        kWarning(30003) << "Could not create child document with type " << mimeType << endl;
        bool res = createUnavailDocument( store, true, mimeType );
        if ( res )
        {
            // Try to turn the mimetype name into its comment
            QString mimeName = mimeType;
            KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
            if ( mime /*mime->name() != KMimeType::defaultMimeType()*/ )
                mimeName = mime->comment();
            d->m_doc->setProperty( "unavailReason", i18n( "No handler found for %1", mimeName ) );
        }
        return res;
    }

    KoDocument * doc = e.createDoc( &errorMsg, d->m_parent );
    if (!doc) {
        kWarning(30003) << "createDoc failed" << endl;
        bool res = createUnavailDocument( store, true, mimeType );
        if ( res )
        {
            d->m_doc->setProperty( "unavailReason", i18n( "Error loading %1:\n%2", e.service()->library(), errorMsg ) );
    }
        return res;
    }
    return finishLoadingDocument( store, doc, doOpenURL, oasis );
}

bool KoDocumentChild::finishLoadingDocument( KoStore* store, KoDocument* doc, bool doOpenURL, bool oasis )
{
    setDocument( doc, m_tmpGeometry );

    bool res = true;
    if ( doOpenURL )
    {
        bool internalURL = false;
        if ( m_tmpURL.startsWith( STORE_PROTOCOL ) || m_tmpURL.startsWith( INTERNAL_PROTOCOL ) || KUrl::isRelativeUrl( m_tmpURL ) )
        {
            if ( oasis ) {
                store->pushDirectory();
                assert( m_tmpURL.startsWith( INTERNAL_PROTOCOL ) );
                QString relPath = KUrl( m_tmpURL ).path().mid( 1 );
                store->enterDirectory( relPath );
                res = d->m_doc->loadOasisFromStore( store );
                store->popDirectory();
            } else {
                if ( m_tmpURL.startsWith( INTERNAL_PROTOCOL ) )
                    m_tmpURL = KUrl( m_tmpURL ).path().mid( 1 );
                res = d->m_doc->loadFromStore( store, m_tmpURL );
            }
            internalURL = true;
            d->m_doc->setStoreInternal( true );
        }
        else
        {
            // Reference to an external document. Hmmm...
            d->m_doc->setStoreInternal( false );
            KUrl url( m_tmpURL );
            if ( !url.isLocalFile() )
            {
                QApplication::restoreOverrideCursor();
                // For security reasons we need to ask confirmation if the url is remote
                int result = KMessageBox::warningYesNoCancel(
                    0, i18n( "This document contains an external link to a remote document\n%1", m_tmpURL),
                    i18n( "Confirmation Required" ), KGuiItem(i18n( "Download" )), KGuiItem(i18n( "Skip" ) ));

                if ( result == KMessageBox::Cancel )
                {
                    d->m_parent->setErrorMessage("USER_CANCELED");
                    return false;
                }
                if ( result == KMessageBox::Yes )
                    res = d->m_doc->openURL( url );
                // and if == No, res will still be false so we'll use a kounavail below
            }
            else
                res = d->m_doc->openURL( url );
        }
        if ( !res )
        {
            // Keep the error message from the attempted loading
            QString errorMessage = d->m_doc->errorMessage();
            delete d->m_doc;
            d->m_doc = 0;
            QString tmpURL = m_tmpURL; // keep a copy, createUnavailDocument will erase it
            // Not found -> use a kounavail instead
            res = createUnavailDocument( store, false /* the URL doesn't exist, don't try to open it */, m_tmpMimeType );
            if ( res )
            {
                d->m_doc->setProperty( "realURL", tmpURL ); // so that it gets saved correctly
                d->m_doc->setStoreInternal( true );
                if ( internalURL )
                    d->m_doc->setProperty( "unavailReason", i18n( "Could not load embedded object:\n%1", errorMessage ) );
                else
                    d->m_doc->setProperty( "unavailReason", i18n( "Could not load external document %1:\n%2", tmpURL, errorMessage ) );
            }
            return res;
        }
        // Still waiting...
        QApplication::setOverrideCursor( Qt::WaitCursor );
    }

    m_tmpURL = QString();

    // see KoDocument::insertChild for an explanation what's going on
    // now :-)
    if ( parentDocument() )
    {
        KoDocument *parent = parentDocument();

        KParts::PartManager* manager = parent->manager();
        if ( manager && !manager->parts().isEmpty() )
        {
            if ( !manager->parts().contains( d->m_doc ) &&
                 !parent->isSingleViewMode() )
                manager->addPart( d->m_doc, false );
        }
    }

    QApplication::restoreOverrideCursor();

    return res;
}

bool KoDocumentChild::createUnavailDocument( KoStore* store, bool doOpenURL, const QString& mimeType )
{
    // We don't need a trader query here. We're looking for a very specific component.
    KService::Ptr serv = KService::serviceByDesktopName( "kounavail" );
    if ( serv.isNull() )
    {
        kWarning(30003) << "ERROR: service kounavail not found " << endl;
        return false;
    }
    KoDocumentEntry e( serv );
    if ( e.isEmpty() )
        return false;
    QString errorMsg;
    KoDocument * doc = e.createDoc( &errorMsg, d->m_parent );
    if ( !doc )
        return false;
    if ( !finishLoadingDocument( store, doc, doOpenURL, false ) )
        return false;
    d->m_doc->setProperty( "mimetype", mimeType );
    return true;
}

bool KoDocumentChild::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    QString path;
    if ( d->m_doc->isStoredExtern() )
    {
        kDebug(30003)<<k_funcinfo<<" external (don't save) url:" << d->m_doc->url().url()<<endl;
        path = d->m_doc->url().url();
    }
    else
    {
        // The name comes from KoDocumentChild (which was set while saving the
        // parent document).
        assert( d->m_doc->url().protocol() == INTERNAL_PROTOCOL );
        const QString name = d->m_doc->url().path();
        kDebug() << k_funcinfo << "saving " << name << endl;

        if ( d->m_doc->nativeOasisMimeType().isEmpty() ) {
            // Embedded object doesn't support OASIS OpenDocument, save in the old format.
            kDebug() << k_funcinfo << "Embedded object doesn't support OASIS OpenDocument, save in the old format." << endl;

            if ( !d->m_doc->saveToStore( store, name ) )
                return false;
        }
        else
        {
            // To make the children happy cd to the correct directory
            store->pushDirectory();
            store->enterDirectory( name );

            if ( !d->m_doc->saveOasis( store, manifestWriter ) ) {
                kWarning(30003) << "KoDocumentChild::saveOasis failed" << endl;
                return false;
            }
            // Now that we're done leave the directory again
            store->popDirectory();
        }

        assert( d->m_doc->url().protocol() == INTERNAL_PROTOCOL );
        path = store->currentDirectory();
        if ( !path.isEmpty() )
            path += '/';
        path += d->m_doc->url().path();
        if ( path.startsWith( "/" ) )
            path = path.mid( 1 ); // remove leading '/', no wanted in manifest
    }

    // OOo uses a trailing slash for the path to embedded objects (== directories)
    if ( !path.endsWith( "/" ) )
        path += '/';
    QByteArray mimetype = d->m_doc->nativeOasisMimeType();
    if ( mimetype.isEmpty() )
        mimetype = d->m_doc->nativeFormatMimeType();
    manifestWriter->addManifestEntry( path, mimetype );

    return true;
}

void KoDocumentChild::saveOasisAttributes( KoXmlWriter &xmlWriter, const QString& name )
{
    if ( !d->m_doc->isStoredExtern() )
    {
        // set URL in document so that KoDocument::saveChildrenOasis will save
        // the actual embedded object with the right name in the store.
        KUrl u;
        u.setProtocol( INTERNAL_PROTOCOL );
        u.setPath( name );
        kDebug() << k_funcinfo << u << endl;
        d->m_doc->setURL( u );
    }

    //<draw:object draw:style-name="standard" draw:id="1" draw:layer="layout" svg:width="14.973cm" svg:height="4.478cm" svg:x="11.641cm" svg:y="14.613cm" xlink:href="#./Object 1" xlink:type="simple" xlink:show="embed" xlink:actuate="onLoad"/>
    xmlWriter.addAttribute( "xlink:type", "simple" );
    xmlWriter.addAttribute( "xlink:show", "embed" );
    xmlWriter.addAttribute( "xlink:actuate", "onLoad" );

    const QString ref = d->m_doc->isStoredExtern() ? d->m_doc->url().url() : "./"+ name;
    kDebug(30003) << "KoDocumentChild::saveOasis saving reference to embedded document as " << ref << endl;
    xmlWriter.addAttribute( "xlink:href", /*"#" + */ref );
}

QDomElement KoDocumentChild::save( QDomDocument& doc, bool uppercase )
{
    if( d->m_doc )
    {
        QDomElement e = doc.createElement( ( uppercase ? "OBJECT" : "object" ) );
        if ( d->m_doc->url().protocol() != INTERNAL_PROTOCOL ) {
            e.setAttribute( "url", d->m_doc->url().url() );
            kDebug() << "KoDocumentChild::save url=" << d->m_doc->url().url() << endl;
        }
        else {
            e.setAttribute( "url", d->m_doc->url().path().mid( 1 ) );
            kDebug() << "KoDocumentChild::save url=" << d->m_doc->url().path().mid( 1 ) << endl;
        }
        e.setAttribute( "mime", QString( d->m_doc->nativeFormatMimeType() ) );
        kDebug() << "KoDocumentChild::save mime=" << d->m_doc->nativeFormatMimeType() << endl;
        QDomElement rect = doc.createElement( ( uppercase ? "RECT" : "rect" ) );
        rect.setAttribute( "x", geometry().left() );
        rect.setAttribute( "y", geometry().top() );
        rect.setAttribute( "w", geometry().width() );
        rect.setAttribute( "h", geometry().height() );
        e.appendChild(rect);
        return e;
    }
    return QDomElement();
}

bool KoDocumentChild::isStoredExtern() const
{
    assert( d->m_doc );
    return d->m_doc->isStoredExtern();
}

KUrl KoDocumentChild::url() const
{
    return ( d->m_doc ? d->m_doc->url() : KUrl() );
}

KoDocumentChild::~KoDocumentChild()
{
  if ( d->m_doc ) {
    delete d->m_doc;
    d->m_doc=0L;
  }
  delete d;
}

bool KoDocumentChild::isDeleted() const
{
    return d->m_deleted;
}

void KoDocumentChild::setDeleted( bool on )
{
    d->m_deleted = on;
}

#include "KoDocumentChild.moc"
