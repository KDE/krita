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
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoOdfStylesReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoTextShapeData.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoShapeStyleWriter.h>
#include <KoStyleManager.h>
#include <KoPathShape.h>
#include <KoLineBorder.h>
#include <KoXmlNS.h>
#include <KoDataCenter.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <typeinfo>

class KoPADocument::Private
{
public:
    QList<KoPAPageBase*> pages;
    QList<KoPAPageBase*> masterPages;
    KoInlineTextObjectManager *inlineTextObjectManager;
    bool rulersVisible;
    QMap<QString, KoDataCenter *>  dataCenterMap;
};

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode ),
    d(new Private())
{
    d->inlineTextObjectManager = new KoInlineTextObjectManager(this);

    // Ask every shapefactory to populate the dataCenterMap
    foreach(QString id, KoShapeRegistry::instance()->keys())
    {
        KoShapeFactory *shapeFactory = KoShapeRegistry::instance()->value(id);
        shapeFactory->populateDataCenterMap(d->dataCenterMap);
    }

    loadConfig();
}

KoPADocument::~KoPADocument()
{
    saveConfig();
    qDeleteAll( d->pages );
    qDeleteAll( d->masterPages );
    qDeleteAll( d->dataCenterMap );
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
    KoOdfLoadingContext loadingContext( odfStore.styles(), odfStore.store() );
    KoPALoadingContext paContext( loadingContext, this );

    KoXmlElement content = odfStore.contentDoc().documentElement();
    KoXmlElement realBody ( KoXml::namedItemNS( content, KoXmlNS::office, "body" ) );

    if ( realBody.isNull() ) {
        kError() << "No body tag found!" << endl;
        return false;
    }

    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, odfTagName( false ));

    if ( body.isNull() ) {
        kError() << "No office:" << odfTagName( false ) << " tag found!" << endl;
        return false;
    }

    // Load text styles before the corresponding text shapes try to use them!
    KoTextSharedLoadingData * sharedData = new KoTextSharedLoadingData();
    KoStyleManager * styleManager = dynamic_cast<KoStyleManager *>( dataCenterMap()["StyleManager"] );
    sharedData->loadOdfStyles( loadingContext, styleManager, true );
    paContext.addSharedData( KOTEXT_SHARED_LOADING_ID, sharedData );

    loadingContext.setUseStylesAutoStyles( true );
    d->masterPages = loadOdfMasterPages( odfStore.styles().masterPages(), paContext );
    loadingContext.setUseStylesAutoStyles( false );
    d->pages = loadOdfPages( body, paContext );
    if ( d->pages.size() > 1 ) {
        setActionEnabled( KoPAView::ActionDeletePage, false );
    }

    emit sigProgress( -1 );
    return true;
}

bool KoPADocument::completeLoading( KoStore* store )
{
    bool ok=true;
    foreach(KoDataCenter *dataCenter, d->dataCenterMap)
    {
        ok = ok && dataCenter->completeLoading(store);
    }
    return ok;
}

bool KoPADocument::saveOdf( SavingContext & documentContext )
{
    KoXmlWriter* contentWriter = documentContext.odfStore.contentWriter();
    if ( !contentWriter )
        return false;

    KoGenStyles mainStyles;
    KoXmlWriter * bodyWriter = documentContext.odfStore.bodyWriter();

    KoPASavingContext paContext( *bodyWriter, mainStyles, documentContext.embeddedSaver, 1, KoShapeSavingContext::Store );

    if ( !saveOasisPages( paContext, d->pages, d->masterPages ) ) {
        return false;
    }

    mainStyles.saveOdfAutomaticStyles( contentWriter, false );

    documentContext.odfStore.closeContentWriter();

    //add manifest line for content.xml
    documentContext.odfStore.manifestWriter()->addManifestEntry( "content.xml", "text/xml" );

    bool ok=true;
    foreach(KoDataCenter *dataCenter, d->dataCenterMap)
    {
        ok = ok && dataCenter->completeSaving(documentContext.odfStore.store(), documentContext.odfStore.manifestWriter());
    }
    if(!ok)
        return false;

    return mainStyles.saveOdfStylesDotXml( documentContext.odfStore.store(), documentContext.odfStore.manifestWriter() );
}

bool KoPADocument::completeSaving( KoStore* store)
{
}

QList<KoPAPageBase *> KoPADocument::loadOdfMasterPages( const QHash<QString, KoXmlElement*> masterStyles, KoPALoadingContext & context )
{
    QList<KoPAPageBase *> masterPages;

    QHash<QString, KoXmlElement*>::const_iterator it( masterStyles.constBegin() );
    for ( ; it != masterStyles.constEnd(); ++it )
    {
        kDebug(30010) << "Master:" << it.key();
        KoPAMasterPage * masterPage = newMasterPage();
        masterPage->loadOdf( *( it.value() ), context );
        masterPages.append( masterPage );
        context.addMasterPage( it.key(), masterPage );
    }
    return masterPages;
}

QList<KoPAPageBase *> KoPADocument::loadOdfPages( const KoXmlElement & body, KoPALoadingContext & context )
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
    paContext.addOption( KoPASavingContext::DrawId );
    paContext.addOption( KoPASavingContext::AutoStyleInStyleXml );

    // save master pages
    foreach( KoPAPageBase *page, masterPages ) {
        if ( paContext.isSetClearDrawIds() ) {
            paContext.clearDrawIds();
        }
        page->saveOdf( paContext );
    }

    KoXmlWriter & bodyWriter = paContext.xmlWriter();
    bodyWriter.startElement( "office:body" );
    bodyWriter.startElement( odfTagName( true ) );

    paContext.removeOption( KoPASavingContext::AutoStyleInStyleXml );

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

int KoPADocument::pageIndex( KoPAPageBase * page ) const
{
    const QList<KoPAPageBase*>& pages = dynamic_cast<KoPAMasterPage *>( page ) ? d->masterPages : d->pages;
    return pages.indexOf( page );
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

    emit shapeAdded( shape );

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

    emit shapeRemoved( shape );

    postRemoveShape( page, shape );
}

void KoPADocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

QMap<QString, KoDataCenter *> KoPADocument::dataCenterMap()
{
    return d->dataCenterMap;
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

void KoPADocument::updateViews(KoPAPageBase *page)
{
    if (!page) return;

    foreach (KoView *view, views()) {
        KoPAView *paView = static_cast<KoPAView *>(view);
        if (paView->activePage() == page)
            paView->updateActivePage(page);
    }
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

    setActionEnabled( KoPAView::ActionDeletePage, pages.size() > 1 );

    emit pageAdded( page );
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

    setActionEnabled( KoPAView::ActionDeletePage, pages.size() > 1 );

    emit pageAdded( page );
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
                kopaView->updateActivePage( newActivePage );
            }
        }
    }

    if ( pages.size() == 1 ) {
        setActionEnabled( KoPAView::ActionDeletePage, false );
    }

    emit pageRemoved( page );

    return index;
}

QList<KoPAPageBase*> KoPADocument::pages( bool masterPages ) const
{
    return masterPages ? d->masterPages : d->pages;
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

void KoPADocument::loadConfig()
{
    KSharedConfigPtr config = componentData().config();

    if( config->hasGroup( "Grid" ) )
    {
        KoGridData defGrid;
        KConfigGroup configGroup = config->group( "Grid" );
        bool showGrid = configGroup.readEntry<bool>( "ShowGrid", defGrid.showGrid() );
        gridData().setShowGrid(showGrid);
        bool snapToGrid = configGroup.readEntry<bool>( "SnapToGrid", defGrid.snapToGrid() );
        gridData().setSnapToGrid(snapToGrid);
        double spacingX = configGroup.readEntry<double>( "SpacingX", defGrid.gridX() );
        double spacingY = configGroup.readEntry<double>( "SpacingY", defGrid.gridY() );
        gridData().setGrid( spacingX, spacingY );
        QColor color = configGroup.readEntry( "Color", defGrid.gridColor() );
        gridData().setGridColor( color );
    }

    if( config->hasGroup( "Interface" ) )
    {
        KConfigGroup configGroup = config->group( "Interface" );
        bool showRulers = configGroup.readEntry<bool>( "ShowRulers", true);
        setRulersVisible(showRulers);
    }
}

void KoPADocument::saveConfig()
{
    KSharedConfigPtr config = componentData().config();
    KConfigGroup configGroup = config->group( "Grid" );
    KoGridData defGrid;

    bool showGrid = gridData().showGrid();
    if ((showGrid == defGrid.showGrid()) && !configGroup.hasDefault("ShowGrid"))
        configGroup.revertToDefault("ShowGrid");
    else
        configGroup.writeEntry("ShowGrid", showGrid);

    bool snapToGrid = gridData().snapToGrid();
    if ((snapToGrid == defGrid.snapToGrid()) && !configGroup.hasDefault("SnapToGrid"))
        configGroup.revertToDefault("SnapToGrid");
    else
        configGroup.writeEntry("SnapToGrid", snapToGrid);

    double spacingX = gridData().gridX();
    if ((spacingX == defGrid.gridX()) && !configGroup.hasDefault("SpacingX"))
        configGroup.revertToDefault("SpacingX");
    else
        configGroup.writeEntry("SpacingX", spacingX);

    double spacingY = gridData().gridY();
    if ((spacingY == defGrid.gridY()) && !configGroup.hasDefault("SpacingY"))
        configGroup.revertToDefault("SpacingY");
    else
        configGroup.writeEntry("SpacingY", spacingY);

    QColor color = gridData().gridColor();
    if ((color == defGrid.gridColor()) && !configGroup.hasDefault("Color"))
        configGroup.revertToDefault("Color");
    else
        configGroup.writeEntry("Color", color);

    configGroup = config->group( "Interface" );

    bool showRulers = rulersVisible();
    if ((showRulers == true) && !configGroup.hasDefault("ShowRulers"))
        configGroup.revertToDefault("ShowRulers");
    else
        configGroup.writeEntry("ShowRulers", showRulers);
}

void KoPADocument::setRulersVisible(bool visible)
{
    d->rulersVisible = visible;
}

bool KoPADocument::rulersVisible() const
{
    return d->rulersVisible;
}


int KoPADocument::pageCount() const
{
    return d->pages.count();
}

#include "KoPADocument.moc"
