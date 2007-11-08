/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#include "KoPADocument.h"

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoOasisStyles.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>
#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoShapeStyleWriter.h>
#include <KoStyleManager.h>
#include <KoPathShape.h>
#include <KoLineBorder.h>
#include <KoDom.h>
#include <KoXmlNS.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

#include <kdebug.h>

#include <typeinfo>

class KoPADocument::Private {
public:

    QList<KoPAPageBase*> pages;
    QList<KoPAPageBase*> masterPages;
    KoInlineTextObjectManager *inlineTextObjectManager;
    KoStyleManager *styleManager;
};

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode ),
    d(new Private())
{
    d->inlineTextObjectManager = new KoInlineTextObjectManager(this);
    d->styleManager = new KoStyleManager(this);
}

KoPADocument::~KoPADocument()
{
    qDeleteAll( d->pages );
    qDeleteAll( d->masterPages );
    delete d;
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect)
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
}

bool KoPADocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    Q_UNUSED( doc );

    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOdf( KoOdfReadStore & odfStore )
{
    emit sigProgress( 0 );
    KoOasisLoadingContext loadingContext( this, odfStore.styles(), odfStore.store() );
    KoPALoadingContext paContext( loadingContext );

    KoXmlElement content = odfStore.contentDoc().documentElement();
    KoXmlElement realBody ( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );

    if ( realBody.isNull() ) {
        kError() << "No body tag found!" << endl;
        return false;
    }

    KoXmlElement body = KoDom::namedItemNS(realBody, KoXmlNS::office, odfTagName( false ));

    if ( body.isNull() ) {
        kError() << "No office:" << odfTagName( false ) << " tag found!" << endl;
        return false;
    }

    d->masterPages = loadOdfMasterPages( odfStore.styles().masterPages(), paContext );
    d->pages = loadOdfPages( body, paContext );
    if ( d->pages.size() > 1 ) {
        setActionEnabled( KoPAView::ActionDeletePage, false );
    }

    emit sigProgress( 100 );
    return true;
}

bool KoPADocument::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    KoOdfWriteStore oasisStore( store );
    KoXmlWriter* contentWriter = oasisStore.contentWriter();
    if ( !contentWriter )
        return false;

    KoGenStyles mainStyles;
    KoXmlWriter * bodyWriter = oasisStore.bodyWriter();

    KoPASavingContext paContext( *bodyWriter, mainStyles, 1, KoShapeSavingContext::Store );

    if ( !saveOasisPages( paContext, d->pages, d->masterPages ) ) {
        return false;
    }

    mainStyles.saveOdfAutomaticStyles( contentWriter, false );

    oasisStore.closeContentWriter();

    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    return mainStyles.saveOdfStylesDotXml( store, manifestWriter );
}

QList<KoPAPageBase *> KoPADocument::loadOdfMasterPages( const QHash<QString, KoXmlElement*> masterStyles, KoPALoadingContext & context )
{
    QList<KoPAPageBase *> masterPages;

    QHash<QString, KoXmlElement*>::const_iterator it( masterStyles.constBegin() );
    for ( ; it != masterStyles.constEnd(); ++it )
    {
        qDebug() << "Master:" << it.key();
        KoPAMasterPage * masterPage = newMasterPage();
        masterPage->loadOdf( *( it.value() ), context );
        masterPages.append( masterPage );
        context.addMasterPage( it.key(), masterPage );
    }
    return masterPages;
}

QList<KoPAPageBase *> KoPADocument::loadOdfPages( KoXmlElement & body, KoPALoadingContext & context )
{
    QList<KoPAPageBase *> pages;
    KoXmlElement element;
    forEachElement( element, body )
    {
        if ( element.tagName() == "page" && element.namespaceURI() == KoXmlNS::draw ) {
            KoPAPage* page = newPage();
            page->loadOdf( element, context );
            pages.append( page );
        }
    }
    return pages;
}

bool KoPADocument::saveOasisPages( KoPASavingContext &paContext, QList<KoPAPageBase *> &pages, QList<KoPAPageBase *> &masterPages )
{
    paContext.setOptions( KoPASavingContext::DrawId | KoPASavingContext::AutoStyleInStyleXml );

    // save master pages
    foreach( KoPAPageBase *page, masterPages ) {
        page->saveOdf( paContext );
    }

    KoXmlWriter & bodyWriter = paContext.xmlWriter();
    bodyWriter.startElement( "office:body" );
    bodyWriter.startElement( odfTagName( true ) );

    paContext.setOptions( KoPASavingContext::DrawId );

    // save pages
    foreach ( KoPAPageBase *page, pages ) {
        page->saveOdf( paContext );
        paContext.incrementPage();
    }

    bodyWriter.endElement(); // office:odfTagName()
    bodyWriter.endElement(); // office:body

    return true;
}

KoPAPageBase* KoPADocument::pageByIndex( int index, bool masterPage ) const
{
    if ( masterPage )
    {
        return d->masterPages.at( index );
    }
    else
    {
        return d->pages.at( index );
    }
}

KoPAPageBase* KoPADocument::pageByNavigation( KoPAPageBase * currentPage, KoPageApp::PageNavigation pageNavigation ) const
{
    const QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( currentPage ) ? d->masterPages : d->pages;

    Q_ASSERT( ! pages.isEmpty() );

    KoPAPageBase * newPage = currentPage;

    switch ( pageNavigation )
    {
        case KoPageApp::PageFirst:
            newPage = pages.first();
            break;
        case KoPageApp::PageLast:
            newPage = pages.last();
            break;
        case KoPageApp::PagePrevious:
        {
            int index = pages.indexOf( currentPage ) - 1;
            if ( index >= 0 )
            {
                newPage = pages.at( index );
            }
        }   break;
        case KoPageApp::PageNext:
            // fall through
        default:
        {
            int index = pages.indexOf( currentPage ) + 1;
            if ( index < pages.size() )
            {
                newPage = pages.at( index );
            }
            break;
        }
    }

    return newPage;
}

void KoPADocument::addShape( KoShape * shape )
{
    if(!shape)
        return;

    KoTextShapeData *data = qobject_cast<KoTextShapeData*> (shape->userData());
    if (data) { // its a text shape.
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (data->document()->documentLayout());
        if(lay)
            lay->setStyleManager(d->styleManager);
    }

    // the KoShapeController sets the active layer as parent
    KoPAPageBase * page( pageByShape( shape ) );

    bool isMaster = dynamic_cast<KoPAMasterPage*>( page ) != 0;

    foreach( KoView *view, views() )
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        KoPAPage * p;
        if ( page == kopaView->activePage() ) {
            kopaView->kopaCanvas()->shapeManager()->add( shape );
        }
        else if ( isMaster && ( p = dynamic_cast<KoPAPage*>( kopaView->activePage() ) ) != 0 ) {
            if ( p->masterPage() == page ) {
                kopaView->kopaCanvas()->masterShapeManager()->add( shape );
            }
        }
    }

    postAddShape( page, shape );
}

void KoPADocument::postAddShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

void KoPADocument::removeShape( KoShape *shape )
{
    if(!shape)
        return;

    KoPAPageBase * page( pageByShape( shape ) );

    bool isMaster = dynamic_cast<KoPAMasterPage*>( page ) != 0;

    foreach( KoView *view, views() )
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        KoPAPage * p;
        if ( page == kopaView->activePage() ) {
            kopaView->kopaCanvas()->shapeManager()->remove( shape );
        }
        else if ( isMaster && ( p = dynamic_cast<KoPAPage*>( kopaView->activePage() ) ) != 0 ) {
            if ( p->masterPage() == page ) {
                kopaView->kopaCanvas()->masterShapeManager()->remove( shape );
            }
        }
    }

    postRemoveShape( page, shape );
}

void KoPADocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

KoPAPageBase * KoPADocument::pageByShape( KoShape * shape ) const
{
    KoShape * parent = shape;
    KoPAPageBase * page = 0;
    while ( !page && ( parent = parent->parent() ) )
    {
        page = dynamic_cast<KoPAPageBase*>( parent );
    }
    return page;
}

void KoPADocument::setActionEnabled( int actions, bool enable )
{
    foreach( KoView *view, views() )
    {
        KoPAView * kopaView = static_cast<KoPAView*>( view );
        kopaView->setActionEnabled( actions, enable );
    }
}

void KoPADocument::insertPage( KoPAPageBase* page, int index )
{
    if ( !page )
        return;

    QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? d->masterPages : d->pages;

    if ( index > pages.size() || index < 0 )
    {
        index = pages.size();
    }

    pages.insert( index, page );

    if ( pages.size() == 2 ) {
        setActionEnabled( KoPAView::ActionDeletePage, true );
    }
}

void KoPADocument::insertPage( KoPAPageBase* page, KoPAPageBase* after )
{
    if ( !page )
        return;

    QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? d->masterPages : d->pages;

    int index = 0;

    if ( after != 0 )
    {
        index = pages.indexOf( after ) + 1;

        // Append the page if after wasn't found in pages
        if ( index == 0 )
            index = pages.count();
    }

    pages.insert( index, page );

    if ( pages.size() == 2 ) {
        setActionEnabled( KoPAView::ActionDeletePage, true );
    }

    // move active view to new page
}

int KoPADocument::takePage( KoPAPageBase *page )
{
    Q_ASSERT( page );

    QList<KoPAPageBase *>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? d->masterPages : d->pages;

    int index = pages.indexOf( page );

    // it should not be possible to delete the last page
    Q_ASSERT( pages.size() > 1 );

    if ( index != -1 ) {
        pages.removeAt( index );

        // change to previous page when the page is the active one if the first one is delete go to the next one
        int newIndex = index == 0 ? 0 : index - 1;
        KoPAPageBase * newActivePage = pages.at( newIndex );
        foreach( KoView *view, views() )
        {
            KoPAView * kopaView = static_cast<KoPAView*>( view );
            if ( page == kopaView->activePage() ) {
                kopaView->setActivePage( newActivePage );
            }
        }
    }

    if ( pages.size() == 1 ) {
        setActionEnabled( KoPAView::ActionDeletePage, false );
    }
    return index;
}

QList<KoPAPageBase*> KoPADocument::pages() const
{
    return d->pages;
}

KoPAPage * KoPADocument::newPage( KoPAMasterPage * masterPage )
{
    return new KoPAPage( masterPage );
}

KoPAMasterPage * KoPADocument::newMasterPage()
{
    return new KoPAMasterPage();
}

/// return the inlineTextObjectManager for this document.
KoInlineTextObjectManager *KoPADocument::inlineTextObjectManager() const {
    return d->inlineTextObjectManager;
}


#include "KoPADocument.moc"
