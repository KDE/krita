/* This file is part of the KDE project
   Copyright (C) 2006-2011 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAViewMode.h"
#include "KoPAPageProvider.h"
#include "commands/KoPAPageDeleteCommand.h"

#include <KoStore.h>
#include <KoDocumentResourceManager.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoOdfStylesReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KoOdfLoadingContext.h>
#include <KoOasisSettings.h>
#include <KoStoreDevice.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoTextShapeData.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoStyleManager.h>
#include <KoPathShape.h>
#include <KoXmlNS.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoDocumentInfo.h>
#include <KoVariableManager.h>
#include <KoPart.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QPainter>

#include <typeinfo>

class KoPADocument::Private
{
public:
    QList<KoPAPageBase*> pages;
    QList<KoPAPageBase*> masterPages;
    KoInlineTextObjectManager *inlineTextObjectManager;
    bool rulersVisible;
    KoPAPageProvider *pageProvider;
    QPointer<KoUpdater> odfProgressUpdater;
    QPointer<KoUpdater> odfMasterPageProgressUpdater;
    QPointer<KoUpdater> odfPageProgressUpdater;
};

KoPADocument::KoPADocument(KoPart *part)
    : KoDocument(part),
    d(new Private())
{
    d->inlineTextObjectManager = resourceManager()->resource(KoText::InlineTextObjectManager).value<KoInlineTextObjectManager*>();
    Q_ASSERT(d->inlineTextObjectManager);
    d->rulersVisible = false;
    connect(documentInfo(), SIGNAL(infoUpdated(const QString &, const QString &)),
            d->inlineTextObjectManager, SLOT(documentInformationUpdated(const QString &, const QString &)));

    resourceManager()->setUndoStack(undoStack());
    resourceManager()->setOdfDocument(this);
    QVariant variant;
    d->pageProvider = new KoPAPageProvider();
    variant.setValue<void*>(d->pageProvider);
    resourceManager()->setResource(KoText::PageProvider, variant);
    loadConfig();
}

KoPADocument::~KoPADocument()
{
    saveConfig();
    qDeleteAll( d->pages );
    qDeleteAll( d->masterPages );
    delete d->pageProvider;
    delete d;
}

QPixmap KoPADocument::generatePreview(const QSize& size)
{
    // use first page as preview for all pages
    KoPAPageBase *page = pageByIndex(0, false);
    Q_ASSERT( page );
    return pageThumbnail(page, size);
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect)
{
    KoPAPageBase * page = pageByIndex( 0, false );
    Q_ASSERT( page );
    QPixmap thumbnail( pageThumbnail( page, rect.size() ) );
    painter.drawPixmap( rect, thumbnail );
}

bool KoPADocument::loadXML( const KoXmlDocument & doc, KoStore * )
{
    Q_UNUSED( doc );

    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

void KoPADocument::setupOpenFileSubProgress()
{
    if (progressUpdater()) {
        d->odfProgressUpdater = progressUpdater()->startSubtask(1, "KoPADocument::loadOdf");
        d->odfMasterPageProgressUpdater = progressUpdater()->startSubtask(1, "KoPADocument::loadOdfMasterPages");
        d->odfPageProgressUpdater = progressUpdater()->startSubtask(5, "KoPADocument::loadOdfPages");
    }
}

bool KoPADocument::loadOdf( KoOdfReadStore & odfStore)
{
    updateDocumentURL();

    if (d->odfProgressUpdater) {
        d->odfProgressUpdater->setProgress(0);
    }
    KoOdfLoadingContext loadingContext( odfStore.styles(), odfStore.store(), KGlobal::mainComponent());
    KoPALoadingContext paContext(loadingContext, resourceManager());

    KoXmlElement content = odfStore.contentDoc().documentElement();
    KoXmlElement realBody ( KoXml::namedItemNS( content, KoXmlNS::office, "body" ) );

    if ( realBody.isNull() ) {
        kError(30010) << "No body tag found!" << endl;
        return false;
    }

    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, odfTagName( false ));

    if ( body.isNull() ) {
        kError(30010) << "No office:" << odfTagName( false ) << " tag found!" << endl;
        return false;
    }

    // Load text styles before the corresponding text shapes try to use them!
    KoTextSharedLoadingData * sharedData = new KoTextSharedLoadingData();
    paContext.addSharedData( KOTEXT_SHARED_LOADING_ID, sharedData );
    KoStyleManager *styleManager = resourceManager()->resource(KoText::StyleManager).value<KoStyleManager*>();

    sharedData->loadOdfStyles(paContext, styleManager);
    if (d->odfProgressUpdater) {
        d->odfProgressUpdater->setProgress(20);
    }

    d->masterPages = loadOdfMasterPages( odfStore.styles().masterPages(), paContext );
    if ( !loadOdfProlog( body, paContext ) ) {
        return false;
    }
    d->pages = loadOdfPages( body, paContext );

    // create pages if there are none
    if (d->masterPages.empty()) {
        d->masterPages.append(newMasterPage());
    }
    if (d->pages.empty()) {
        d->pages.append(newPage(static_cast<KoPAMasterPage*>(d->masterPages.first())));
    }

    if ( !loadOdfEpilogue( body, paContext ) ) {
        return false;
    }

    loadOdfDocumentStyles( paContext );

    if ( d->pages.size() > 1 ) {
        emit actionsPossible(KoPAView::ActionDeletePage, false);
    }

    updatePageCount();

    loadOdfSettings(odfStore.settingsDoc());

    if (d->odfProgressUpdater) {
        d->odfProgressUpdater->setProgress(100);
    }
    return true;
}

bool KoPADocument::saveOdf( SavingContext & documentContext )
{
    KoXmlWriter* contentWriter = documentContext.odfStore.contentWriter();
    if ( !contentWriter )
        return false;

    KoGenStyles mainStyles;
    KoXmlWriter * bodyWriter = documentContext.odfStore.bodyWriter();

    KoPASavingContext paContext(*bodyWriter, mainStyles, documentContext.embeddedSaver, 1);

    saveOdfDocumentStyles( paContext );

    bodyWriter->startElement( "office:body" );
    bodyWriter->startElement( odfTagName( true ) );

    if ( !saveOdfProlog( paContext ) ) {
        return false;
    }

    if ( !saveOdfPages( paContext, d->pages, d->masterPages ) ) {
        return false;
    }

    if ( ! saveOdfEpilogue( paContext ) ) {
        return false;
    }

    bodyWriter->endElement(); // office:odfTagName()
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles( KoGenStyles::DocumentAutomaticStyles, contentWriter );

    documentContext.odfStore.closeContentWriter();

    //add manifest line for content.xml
    documentContext.odfStore.manifestWriter()->addManifestEntry( "content.xml", "text/xml" );

    if ( ! mainStyles.saveOdfStylesDotXml( documentContext.odfStore.store(), documentContext.odfStore.manifestWriter() ) ) {
        return false;
    }

    KoStore * store = documentContext.odfStore.store();
    if ( ! store->open( "settings.xml" ) ) {
        return false;
    }

    saveOdfSettings( store );

    if ( ! store->close() ) {
        return false;
    }

    documentContext.odfStore.manifestWriter()->addManifestEntry( "settings.xml", "text/xml" );

    //setModified( false );

    bool retval = paContext.saveDataCenter( documentContext.odfStore.store(), documentContext.odfStore.manifestWriter() );

    if (retval) {
        updateDocumentURL();
    }

    return retval;
}

QList<KoPAPageBase *> KoPADocument::loadOdfMasterPages( const QHash<QString, KoXmlElement*> masterStyles, KoPALoadingContext & context )
{
    context.odfLoadingContext().setUseStylesAutoStyles( true );
    QList<KoPAPageBase *> masterPages;

    if (d->odfMasterPageProgressUpdater) {
        d->odfMasterPageProgressUpdater->setProgress(0);
    }

    QHash<QString, KoXmlElement*>::const_iterator it( masterStyles.constBegin() );
    int count = 0;
    for ( ; it != masterStyles.constEnd(); ++it )
    {
        KoPAMasterPage * masterPage = newMasterPage();
        masterPage->loadOdf( *( it.value() ), context );
        masterPages.append( masterPage );
        context.addMasterPage( it.key(), masterPage );
        if (d->odfMasterPageProgressUpdater) {
            int progress = 100 * ++count / masterStyles.size();
            d->odfMasterPageProgressUpdater->setProgress(progress);
        }
    }
    context.odfLoadingContext().setUseStylesAutoStyles( false );
    if (d->odfMasterPageProgressUpdater) {
        d->odfMasterPageProgressUpdater->setProgress(100);
    }
    return masterPages;
}

QList<KoPAPageBase *> KoPADocument::loadOdfPages( const KoXmlElement & body, KoPALoadingContext & context )
{
    if (d->masterPages.isEmpty()) { // we require at least one master page. Auto create one if the doc was faulty.
        d->masterPages << newMasterPage();
    }

    int childNodesCount = 0;
    int childCount = 0;
    if (d->odfPageProgressUpdater) {
        d->odfPageProgressUpdater->setProgress(0);
        childNodesCount = body.childNodesCount();
    }

    QList<KoPAPageBase *> pages;
    KoXmlElement element;
    forEachElement( element, body )
    {
        if ( element.tagName() == "page" && element.namespaceURI() == KoXmlNS::draw ) {
            KoPAPage *page = newPage(static_cast<KoPAMasterPage*>(d->masterPages.first()));
            page->loadOdf( element, context );
            pages.append( page );
            // in case the page name is pageX where X is the page number remove the name as this is
            // remove the page name and show the default page name like Slide X or Page X. 
            if (page->name() == QString("page%1").arg(pages.size())) {
                page->setName("");
            }
        }

        if (d->odfPageProgressUpdater) {
            int progress = 100 * ++childCount / childNodesCount;
            d->odfPageProgressUpdater->setProgress(progress);
        }
    }
    if (d->odfPageProgressUpdater) {
        d->odfPageProgressUpdater->setProgress(100);
    }
    return pages;
}

bool KoPADocument::loadOdfEpilogue( const KoXmlElement & body, KoPALoadingContext & context )
{
    Q_UNUSED( body );
    Q_UNUSED( context );
    return true;
}

bool KoPADocument::loadOdfProlog( const KoXmlElement & body, KoPALoadingContext & context )
{
    Q_UNUSED( body );
    Q_UNUSED( context );

    // Load user defined variable declarations
    if (KoVariableManager *variableManager = inlineTextObjectManager()->variableManager()) {
        variableManager->loadOdf(body);
    }

    return true;
}

bool KoPADocument::saveOdfPages( KoPASavingContext &paContext, QList<KoPAPageBase *> &pages, QList<KoPAPageBase *> &masterPages )
{
    paContext.addOption( KoPASavingContext::DrawId );
    paContext.addOption( KoPASavingContext::AutoStyleInStyleXml );

    // save master pages
    foreach( KoPAPageBase *page, masterPages ) {
        if ( paContext.isSetClearDrawIds() ) {
            paContext.clearXmlIds("shape");
        }
        page->saveOdf( paContext );
    }

    paContext.removeOption( KoPASavingContext::AutoStyleInStyleXml );

    // save pages
    foreach ( KoPAPageBase *page, pages ) {
        page->saveOdf( paContext );
        paContext.incrementPage();
    }

    return true;
}

bool KoPADocument::saveOdfProlog( KoPASavingContext & paContext )
{
    Q_UNUSED( paContext );

    // Save user defined variable declarations
    if (KoVariableManager *variableManager = inlineTextObjectManager()->variableManager()) {
        variableManager->saveOdf(&paContext.xmlWriter());
    }

    return true;
}

bool KoPADocument::saveOdfEpilogue( KoPASavingContext & paContext )
{
    Q_UNUSED( paContext );
    return true;
}

bool KoPADocument::saveOdfSettings( KoStore * store )
{
    KoStoreDevice settingsDev( store );
    KoXmlWriter * settingsWriter = KoOdfWriteStore::createOasisXmlWriter( &settingsDev, "office:document-settings" );

    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    saveUnitOdf(settingsWriter);

    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "ooo:view-settings");
    settingsWriter->startElement("config:config-item-map-indexed" );
    settingsWriter->addAttribute("config:name", "Views" );
    settingsWriter->startElement("config:config-item-map-entry" );

    guidesData().saveOdfSettings( *settingsWriter );
    gridData().saveOdfSettings( *settingsWriter );

    settingsWriter->endElement(); // config:config-item-map-entry
    settingsWriter->endElement(); // config:config-item-map-indexed
    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // office:document-settings

    settingsWriter->endDocument();

    delete settingsWriter;

    return true;
}

void KoPADocument::loadOdfSettings(  const KoXmlDocument & settingsDoc )
{
    if ( settingsDoc.isNull() ) {
        return ; // not an error if some file doesn't have settings.xml
    }

    KoOasisSettings settings( settingsDoc );
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    if ( !viewSettings.isNull() ) {
        setUnit(KoUnit::fromSymbol(viewSettings.parseConfigItemString("unit")));
        // FIXME: add other config here.
    }

    guidesData().loadOdfSettings( settingsDoc );
    gridData().loadOdfSettings( settingsDoc );
}

void KoPADocument::saveOdfDocumentStyles( KoPASavingContext & context )
{
    KoStyleManager *styleManager = resourceManager()->resource(KoText::StyleManager).value<KoStyleManager*>();
    Q_ASSERT( styleManager );
    styleManager->saveOdf(context);
}

bool KoPADocument::loadOdfDocumentStyles( KoPALoadingContext & context )
{
    Q_UNUSED( context );
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
    KoPAPageBase *page(pageByShape(shape));

    emit shapeAdded(shape);

    // it can happen in kpresenter notes view that there is no page
    if ( page ) {
        page->shapeAdded( shape );
        postAddShape( page, shape );
    }
}

void KoPADocument::postAddShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

void KoPADocument::removeShape(KoShape *shape)
{
    if (!shape)
        return;

    KoPAPageBase *page(pageByShape(shape));

    emit shapeRemoved(shape);

    page->shapeRemoved(shape);
    postRemoveShape(page, shape);
}

void KoPADocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    Q_UNUSED( page );
    Q_UNUSED( shape );
}

void KoPADocument::removePage( KoPAPageBase * page )
{
    KoPAPageDeleteCommand * command = new KoPAPageDeleteCommand( this, page );
    pageRemoved( page, command );
    addCommand( command );
}


void KoPADocument::removePages(QList<KoPAPageBase *> &pages)
{
    KoPAPageDeleteCommand *command = new KoPAPageDeleteCommand(this, pages);
    addCommand(command);
}

void KoPADocument::pageRemoved( KoPAPageBase * page, KUndo2Command * parent )
{
    Q_UNUSED( page );
    Q_UNUSED( parent );
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

//F)XME
/*
void KoPADocument::updateViews(KoPAPageBase *page)
{
    if (!page) return;

    foreach (KoView *view, views()) {
        KoPAView *paView = static_cast<KoPAView *>(view);
        if ( paView->activePage() == page ) {
            paView->viewMode()->updateActivePage( page );
        }
        else if ( dynamic_cast<KoPAMasterPage *>( page ) ) {
            // if the page changed is a master page, we need to check whether it is the current page's master page
            KoPAPage *activePage = dynamic_cast<KoPAPage *>( paView->activePage() );
            if ( activePage && activePage->masterPage() == page ) {
                paView->viewMode()->updateActivePage( activePage );
            }
        }
    }
}
*/
KoPageApp::PageType KoPADocument::pageType() const
{
    return KoPageApp::Page;
}

QPixmap KoPADocument::pageThumbnail(KoPAPageBase* page, const QSize& size)
{
    int pageNumber = pageIndex(page) + 1;
    d->pageProvider->setPageData(pageNumber, page);
    return page->thumbnail(size);
}

QImage KoPADocument::pageThumbImage(KoPAPageBase* page, const QSize& size)
{
    int pageNumber = pageIndex(page) + 1;
    d->pageProvider->setPageData(pageNumber, page);
    return page->thumbImage(size);
}

void KoPADocument::initEmpty()
{
    d->masterPages.clear();
    d->pages.clear();
    KoPAMasterPage * masterPage = newMasterPage();
    d->masterPages.append( masterPage );
    KoPAPage * page = newPage( masterPage );
    d->pages.append( page );
    KoDocument::initEmpty();
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
    updatePageCount();

    emit actionsPossible(KoPAView::ActionDeletePage, pages.size() > 1);

    emit pageAdded(page);
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
    updatePageCount();

    emit actionsPossible(KoPAView::ActionDeletePage, pages.size() > 1);

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

        updatePageCount();

        emit replaceActivePage(page, newActivePage);
        emit pageRemoved(page);
    }

    if ( pages.size() == 1 ) {
        emit actionsPossible(KoPAView::ActionDeletePage, false);
    }


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
KoInlineTextObjectManager *KoPADocument::inlineTextObjectManager() const
{
    return d->inlineTextObjectManager;
}

void KoPADocument::loadConfig()
{
    KSharedConfigPtr config = KGlobal::mainComponent().config();

    if( config->hasGroup( "Grid" ) )
    {
        KoGridData defGrid;
        KConfigGroup configGroup = config->group( "Grid" );
        bool showGrid = configGroup.readEntry<bool>( "ShowGrid", defGrid.showGrid() );
        gridData().setShowGrid(showGrid);
        bool paintGridInBackground = configGroup.readEntry("PaintGridInBackground", defGrid.paintGridInBackground());
        gridData().setPaintGridInBackground(paintGridInBackground);
        bool snapToGrid = configGroup.readEntry<bool>( "SnapToGrid", defGrid.snapToGrid() );
        gridData().setSnapToGrid(snapToGrid);
        qreal spacingX = configGroup.readEntry<qreal>( "SpacingX", defGrid.gridX() );
        qreal spacingY = configGroup.readEntry<qreal>( "SpacingY", defGrid.gridY() );
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
    KSharedConfigPtr config = KGlobal::mainComponent().config();
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

    qreal spacingX = gridData().gridX();
    if ((spacingX == defGrid.gridX()) && !configGroup.hasDefault("SpacingX"))
        configGroup.revertToDefault("SpacingX");
    else
        configGroup.writeEntry("SpacingX", spacingX);

    qreal spacingY = gridData().gridY();
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

void KoPADocument::updatePageCount()
{
    if (resourceManager()->hasResource(KoText::InlineTextObjectManager)) {
        QVariant var = resourceManager()->resource(KoText::InlineTextObjectManager);
        KoInlineTextObjectManager *om = var.value<KoInlineTextObjectManager*>();
        om->setProperty( KoInlineObject::PageCount, pageCount() );
    }
}

void KoPADocument::updateDocumentURL()
{
    if (resourceManager()->hasResource(KoText::InlineTextObjectManager)) {
        QVariant var = resourceManager()->resource(KoText::InlineTextObjectManager);
        KoInlineTextObjectManager *om = var.value<KoInlineTextObjectManager*>();
        om->setProperty(KoInlineObject::DocumentURL, url().pathOrUrl());
    }
}

#include <KoPADocument.moc>
