/* This file is part of the KDE project

   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2009 Inge Wallin   <inge@lysator.liu.se>

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

#include "KoPAView.h"

#include <QGridLayout>
#include <QToolBar>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QLabel>

#include <KoCanvasController.h>
#include <KoResourceManager.h>
#include <KoColorBackground.h>
#include <KoFind.h>
#include <KoTextDocumentLayout.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomHandler.h>
#include <KoStandardAction.h>
#include <KoToolBoxFactory.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoZoomAction.h>
#include <KoZoomController.h>
#include <KoInlineTextObjectManager.h>
#include <KoSelection.h>
#include <KoMainWindow.h>
#include <KoDockerManager.h>
#include <KoShapeLayer.h>
#include <KoRuler.h>
#include <KoRulerController.h>
#include <KoDrag.h>
#include <KoShapeDeleteCommand.h>
#include <KoCutController.h>
#include <KoCopyController.h>
#include <KoFilterManager.h>

#include "KoPADocumentStructureDocker.h"
#include "KoShapeTraversal.h"
#include "KoPACanvas.h"
#include "KoPADocument.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPAViewModeNormal.h"
#include "KoPAOdfPageSaveHelper.h"
#include "KoPAPastePage.h"
#include "KoPAPrintJob.h"
#include "commands/KoPAPageInsertCommand.h"
#include "commands/KoPAChangeMasterPageCommand.h"
#include "commands/KoPAChangePageLayoutCommand.h"
#include "dialogs/KoPAMasterPageDialog.h"
#include "dialogs/KoPAPageLayoutDialog.h"
#include "dialogs/KoPAConfigureDialog.h"

#include <kfiledialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kstatusbar.h>
#include <kmessagebox.h>
#include <kparts/event.h>
#include <kparts/partmanager.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>


class KoPAView::Private
{
public:
    Private( KoPADocument *document )
    : doc( document )
    , canvas( 0 )
    , activePage( 0 )
    , viewMode( 0 )
    {}

    ~Private()
    {}

    // These were originally private in the .h file
    KoPADocumentStructureDocker *documentStructureDocker;

    KoCanvasController *canvasController;
    KoZoomController *zoomController;
    KoZoomHandler zoomHandler;

    KAction *editPaste;
    KAction *deleteSelectionAction;

    KToggleAction *actionViewSnapToGrid;
    KToggleAction *actionViewShowMasterPages;

    KAction *actionInsertPage;
    KAction *actionCopyPage;
    KAction *actionDeletePage;

    KAction *actionMasterPage;
    KAction *actionPageLayout;

    KAction *actionConfigure;

    KoRuler *horizontalRuler;
    KoRuler *verticalRuler;
    KToggleAction *viewRulers;

    KoZoomAction  *zoomAction;

    KoFind *find;

    KoPAViewMode *viewModeNormal;

    // status bar
    QLabel *status;       ///< ordinary status
    QWidget *zoomActionWidget;

    // These used to be protected.
    KoPADocument *doc;
    KoPACanvas *canvas;
    KoPAPageBase *activePage;
    KoPAViewMode *viewMode;
};



KoPAView::KoPAView( KoPADocument *document, QWidget *parent )
: KoView( document, parent )
, d( new Private( document ) )
{
    initGUI();
    initActions();

    if ( d->doc->pageCount() > 0 )
        doUpdateActivePage( d->doc->pageByIndex( 0, false ) );
}

KoPAView::~KoPAView()
{
    KoToolManager::instance()->removeCanvasController( d->canvasController );

    removeStatusBarItem(d->status);
    removeStatusBarItem(d->zoomActionWidget);

    delete d->zoomController;
    // Delete only the view mode normal, let the derived class delete
    // the currently active view mode if it is not view mode normal
    delete d->viewModeNormal;

    delete d;
}

void KoPAView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout( this );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 0 );
    setLayout( gridLayout );

    d->canvas = new KoPACanvas( this, d->doc );
    d->canvasController = new KoCanvasController( this );
    d->canvasController->setCanvas( d->canvas );
    KoToolManager::instance()->addController( d->canvasController );
    KoToolManager::instance()->registerTools( actionCollection(), d->canvasController );

    d->zoomController = new KoZoomController( d->canvasController, &d->zoomHandler, actionCollection());
    connect( d->zoomController, SIGNAL( zoomChanged( KoZoomMode::Mode, qreal ) ),
             this, SLOT( slotZoomChanged( KoZoomMode::Mode, qreal ) ) );

    d->zoomAction = d->zoomController->zoomAction();

    // set up status bar message
    d->status = new QLabel( QString() );
    d->status->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    d->status->setMinimumWidth( 300 );
    addStatusBarItem( d->status, 1 );
    connect( KoToolManager::instance(), SIGNAL( changedStatusText( const QString & ) ),
             d->status, SLOT( setText( const QString & ) ) );
    d->zoomActionWidget = d->zoomAction->createWidget(  statusBar() );
    addStatusBarItem( d->zoomActionWidget, 0 );

    d->zoomController->setZoomMode( KoZoomMode::ZOOM_PAGE );

    d->viewModeNormal = new KoPAViewModeNormal( this, d->canvas );
    d->viewMode = d->viewModeNormal;

    // The rulers
    d->horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter( d->canvas ));
    d->horizontalRuler->setShowMousePosition(true);
    d->horizontalRuler->setUnit(d->doc->unit());
    d->verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter( d->canvas ));
    d->verticalRuler->setUnit(d->doc->unit());
    d->verticalRuler->setShowMousePosition(true);

    new KoRulerController(d->horizontalRuler, d->canvas->resourceManager());

    connect(d->doc, SIGNAL(unitChanged(const KoUnit&)),
            d->horizontalRuler, SLOT(setUnit(const KoUnit&)));
    connect(d->doc, SIGNAL(unitChanged(const KoUnit&)),
            d->verticalRuler, SLOT(setUnit(const KoUnit&)));

    gridLayout->addWidget(d->horizontalRuler, 0, 1);
    gridLayout->addWidget(d->verticalRuler, 1, 0);
    gridLayout->addWidget( d->canvasController, 1, 1 );

    connect(d->canvasController, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(pageOffsetChanged()));
    connect(d->canvasController, SIGNAL(canvasOffsetYChanged(int)),
            this, SLOT(pageOffsetChanged()));
    connect(d->canvasController, SIGNAL(sizeChanged(const QSize&)),
            this, SLOT(pageOffsetChanged()));
    connect(d->canvasController, SIGNAL(canvasMousePositionChanged(const QPoint&)),
            this, SLOT(updateMousePosition(const QPoint&)));
    d->verticalRuler->createGuideToolConnection(d->canvas);
    d->horizontalRuler->createGuideToolConnection(d->canvas);

    KoToolBoxFactory toolBoxFactory(d->canvasController, i18n("Tools") );
    if (shell())
    {
        shell()->createDockWidget( &toolBoxFactory );

        connect( d->canvasController, SIGNAL( toolOptionWidgetsChanged(const QMap<QString, QWidget *> &, QWidget*) ),
             shell()->dockerManager(), SLOT( newOptionWidgets(const  QMap<QString, QWidget *> &, QWidget*) ) );
    }

    connect(shapeManager(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(d->canvas, SIGNAL(documentSize(const QSize&)), d->canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(d->canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            d->canvas, SLOT(setDocumentOffset(const QPoint&)));

    if (shell()) {
        KoPADocumentStructureDockerFactory structureDockerFactory( KoDocumentSectionView::ThumbnailMode, d->doc->pageType() );
        d->documentStructureDocker = qobject_cast<KoPADocumentStructureDocker*>( shell()->createDockWidget( &structureDockerFactory ) );
        connect( shell()->partManager(), SIGNAL( activePartChanged( KParts::Part * ) ),
                d->documentStructureDocker, SLOT( setPart( KParts::Part * ) ) );
        connect(d->documentStructureDocker, SIGNAL(pageChanged(KoPAPageBase*)), this, SLOT(updateActivePage(KoPAPageBase*)));
        connect(d->documentStructureDocker, SIGNAL(dockerReset()), this, SLOT(reinitDocumentDocker()));

        KoToolManager::instance()->requestToolActivation( d->canvasController );
    }
}

void KoPAView::initActions()
{
    KAction *action = actionCollection()->addAction( KStandardAction::Cut, "edit_cut", 0, 0);
    new KoCutController(kopaCanvas(), action);
    action = actionCollection()->addAction( KStandardAction::Copy, "edit_copy", 0, 0 );
    new KoCopyController(kopaCanvas(), action);
    d->editPaste = actionCollection()->addAction( KStandardAction::Paste, "edit_paste", this, SLOT( editPaste() ) );
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(d->canvas->toolProxy(), SIGNAL(toolChanged(const QString&)), this, SLOT(clipboardDataChanged()));
    clipboardDataChanged();
    actionCollection()->addAction(KStandardAction::SelectAll,  "edit_select_all", this, SLOT(editSelectAll()));
    actionCollection()->addAction(KStandardAction::Deselect,  "edit_deselect_all", this, SLOT(editDeselectAll()));

    d->deleteSelectionAction = new KAction(KIcon("edit-delete"), i18n("D&elete"), this);
    actionCollection()->addAction("edit_delete", d->deleteSelectionAction );
    d->deleteSelectionAction->setShortcut(QKeySequence("Del"));
    d->deleteSelectionAction->setEnabled(false);
    connect(d->deleteSelectionAction, SIGNAL(triggered()), this, SLOT(editDeleteSelection()));
    connect(d->canvas->toolProxy(),    SIGNAL(selectionChanged(bool)), 
            d->deleteSelectionAction, SLOT(setEnabled(bool)));

    KToggleAction *showGrid= d->doc->gridData().gridToggleAction(d->canvas);
    actionCollection()->addAction("view_grid", showGrid );

    d->actionViewSnapToGrid = new KToggleAction(i18n("Snap to Grid"), this);
    d->actionViewSnapToGrid->setChecked(d->doc->gridData().snapToGrid());
    actionCollection()->addAction("view_snaptogrid", d->actionViewSnapToGrid);
    connect( d->actionViewSnapToGrid, SIGNAL( triggered( bool ) ), this, SLOT (viewSnapToGrid( bool )));

    KToggleAction *actionViewShowGuides = KoStandardAction::showGuides(this, SLOT(viewGuides(bool)), this);
    actionViewShowGuides->setChecked( d->doc->guidesData().showGuideLines() );
    actionCollection()->addAction(KoStandardAction::name(KoStandardAction::ShowGuides),
            actionViewShowGuides );

    d->actionViewShowMasterPages = new KToggleAction(i18n( "Show Master Pages" ), this );
    actionCollection()->addAction( "view_masterpages", d->actionViewShowMasterPages );
    connect( d->actionViewShowMasterPages, SIGNAL( triggered( bool ) ), this, SLOT( setMasterMode( bool ) ) );

    d->viewRulers  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection()->addAction("view_rulers", d->viewRulers );
    d->viewRulers->setToolTip(i18n("Show/hide the view's rulers"));
    connect(d->viewRulers, SIGNAL(triggered(bool)), this, SLOT(setShowRulers(bool)));
    setShowRulers(d->doc->rulersVisible());

    d->actionInsertPage = new KAction( KIcon("document-new"), i18n( "Insert Page" ), this );
    actionCollection()->addAction( "page_insertpage", d->actionInsertPage );
    d->actionInsertPage->setToolTip( i18n( "Insert a new page after the current one" ) );
    d->actionInsertPage->setWhatsThis( i18n( "Insert a new page after the current one" ) );
    connect( d->actionInsertPage, SIGNAL( triggered() ), this, SLOT( insertPage() ) );

    d->actionCopyPage = new KAction( i18n( "Copy Page" ), this );
    actionCollection()->addAction( "page_copypage", d->actionCopyPage );
    d->actionCopyPage->setToolTip( i18n( "Copy the current page" ) );
    d->actionCopyPage->setWhatsThis( i18n( "Copy the current page" ) );
    connect( d->actionCopyPage, SIGNAL( triggered() ), this, SLOT( copyPage() ) );

    d->actionDeletePage = new KAction( i18n( "Delete Page" ), this );
    d->actionDeletePage->setEnabled( d->doc->pageCount() > 1 );
    actionCollection()->addAction( "page_deletepage", d->actionDeletePage );
    d->actionDeletePage->setToolTip( i18n( "Delete the current page" ) );
    d->actionDeletePage->setWhatsThis( i18n( "Delete the current page" ) );
    connect( d->actionDeletePage, SIGNAL( triggered() ), this, SLOT( deletePage() ) );

    d->actionMasterPage = new KAction(i18n("Master Page..."), this);
    actionCollection()->addAction("format_masterpage", d->actionMasterPage);
    connect(d->actionMasterPage, SIGNAL(triggered()), this, SLOT(formatMasterPage()));

    d->actionPageLayout = new KAction( i18n( "Page Layout..." ), this );
    actionCollection()->addAction( "format_pagelayout", d->actionPageLayout );
    connect( d->actionPageLayout, SIGNAL( triggered() ), this, SLOT( formatPageLayout() ) );

    actionCollection()->addAction(KStandardAction::Prior,  "page_previous", this, SLOT(goToPreviousPage()));
    actionCollection()->addAction(KStandardAction::Next,  "page_next", this, SLOT(goToNextPage()));
    actionCollection()->addAction(KStandardAction::FirstPage,  "page_first", this, SLOT(goToFirstPage()));
    actionCollection()->addAction(KStandardAction::LastPage,  "page_last", this, SLOT(goToLastPage()));

    KActionMenu *actionMenu = new KActionMenu(i18n("Variable"), this);
    foreach(QAction *action, d->doc->inlineTextObjectManager()->createInsertVariableActions(d->canvas))
        actionMenu->addAction(action);
    actionCollection()->addAction("insert_variable", actionMenu);

    KAction * am = new KAction(i18n("Import Document..."), this);
    actionCollection()->addAction("import_document", am);
    connect(am, SIGNAL(triggered()), this, SLOT(importDocument()));

    d->actionConfigure = new KAction(KIcon("configure"), i18n("Configure..."), this);
    actionCollection()->addAction("configure", d->actionConfigure);
    connect(d->actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));

    d->find = new KoFind( this, d->canvas->resourceManager(), actionCollection() );

    actionCollection()->action( "object_group" )->setShortcut( QKeySequence( "Ctrl+G" ) );
    actionCollection()->action( "object_ungroup" )->setShortcut( QKeySequence( "Ctrl+Shift+G" ) );
}


KoPACanvas* KoPAView::kopaCanvas() const
{
    return d->canvas;
}

KoPADocument * KoPAView::kopaDocument() const
{
    return d->doc;
}

KoPAPageBase* KoPAView::activePage() const
{
    return d->activePage;
}

void KoPAView::updateReadWrite( bool readwrite )
{
    d->canvas->setReadWrite(readwrite);
    KoToolManager::instance()->updateReadWrite(d->canvasController, readwrite);
}

KoViewConverter* KoPAView::viewConverter( KoPACanvas * canvas )
{
    Q_UNUSED( canvas );

    return &d->zoomHandler;
}

KoZoomHandler* KoPAView::zoomHandler() const
{
    return &d->zoomHandler;
}

KoZoomController* KoPAView::zoomController() const
{
    return d->zoomController;
}

KoRuler* KoPAView::horizontalRuler()
{
    return d->horizontalRuler;
}

KoRuler* KoPAView::verticalRuler()
{
    return d->verticalRuler;
}



void KoPAView::importDocument()
{
    KFileDialog *dialog = new KFileDialog( KUrl("kfiledialog:///OpenDialog"),QString(), this );
    dialog->setObjectName( "file dialog" );
    dialog->setMode( KFile::File );
    if ( d->doc->pageType() == KoPageApp::Slide ) {
        dialog->setCaption(i18n("Import Slideshow"));
    }
    else {
        dialog->setCaption(i18n("Import Document"));
    }

    // TODO make it possible to select also other supported types (then the default format) here.
    // this needs to go via the filters to get the file in the correct format.
    // For now we only support the native mime types
    QStringList mimeFilter;
#if 1
    mimeFilter << KoOdf::mimeType( d->doc->documentType() ) << KoOdf::templateMimeType( d->doc->documentType() );
#else
    mimeFilter = KoFilterManager::mimeFilter( KoDocument::readNativeFormatMimeType(d->doc->componentData()), KoFilterManager::Import,
                                              KoDocument::readExtraNativeMimeTypes() );
#endif

    dialog->setMimeFilter( mimeFilter );
    if (dialog->exec() == QDialog::Accepted) {
        KUrl url(dialog->selectedUrl());
        QString tmpFile;
        if ( KIO::NetAccess::download( url, tmpFile, 0 ) ) {
            QFile file( tmpFile );
            file.open( QIODevice::ReadOnly );
            QByteArray ba;
            ba = file.readAll();

            // set the correct mime type as otherwise it does not find the correct tag when loading
            QMimeData data;
            data.setData( KoOdf::mimeType( d->doc->documentType() ), ba);
            KoPAPastePage paste( d->doc,d->activePage );
            if ( ! paste.paste( d->doc->documentType(), &data ) ) {
                KMessageBox::error(0, i18n("Could not import\n%1", url.pathOrUrl()));
            }
        }
        else {
            KMessageBox::error(0, i18n("Could not import\n%1", url.pathOrUrl()));
        }
    }
    delete dialog;
}

void KoPAView::viewSnapToGrid(bool snap)
{
    d->doc->gridData().setSnapToGrid(snap);
    d->actionViewSnapToGrid->setChecked(snap);
}

void KoPAView::viewGuides(bool show)
{
    d->doc->guidesData().setShowGuideLines(show);
    d->canvas->update();
}

void KoPAView::editPaste()
{
    if ( !d->canvas->toolProxy()->paste() ) {
        pagePaste();
    }
}

void KoPAView::pagePaste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    KoOdf::DocumentType documentTypes[] = { KoOdf::Graphics, KoOdf::Presentation };

    for ( unsigned int i = 0; i < sizeof( documentTypes ) / sizeof( KoOdf::DocumentType ); ++i )
    {
        if ( data->hasFormat( KoOdf::mimeType( documentTypes[i] ) ) ) {
            KoPAPastePage paste( d->doc, d->activePage );
            paste.paste( documentTypes[i], data );
            break;
        }
    }
}

void KoPAView::editDeleteSelection()
{
    d->canvas->toolProxy()->deleteSelection();
}

void KoPAView::editSelectAll()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( !selection )
        return;

    QList<KoShape*> shapes = activePage()->shapes();

    foreach( KoShape *shape, shapes ) {
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer *>( shape );

        if ( layer ) {
            QList<KoShape*> layerShapes( layer->shapes() );
            foreach( KoShape *layerShape, layerShapes ) {
                selection->select( layerShape );
                layerShape->update();
            }
        }
    }

    selectionChanged();
}

void KoPAView::editDeselectAll()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( selection )
        selection->deselectAll();

    selectionChanged();
    kopaCanvas()->update();
}

void KoPAView::formatMasterPage()
{
    KoPAPage *page = dynamic_cast<KoPAPage *>(d->activePage);
    Q_ASSERT(page);
    KoPAMasterPageDialog *dialog = new KoPAMasterPageDialog(d->doc, page->masterPage(), d->canvas);

    if (dialog->exec() == QDialog::Accepted) {
        KoPAMasterPage *masterPage = dialog->selectedMasterPage();
        KoPAPage *page = dynamic_cast<KoPAPage *>(d->activePage);
        if (page) {
            KoPAChangeMasterPageCommand * command = new KoPAChangeMasterPageCommand( d->doc, page, masterPage );
            d->canvas->addCommand( command );
        }
    }

    delete dialog;
}

void KoPAView::formatPageLayout()
{
    const KoPageLayout &pageLayout = d->viewMode->activePageLayout();

    KoPAPageLayoutDialog dialog( d->doc, pageLayout, d->canvas );

    if ( dialog.exec() == QDialog::Accepted ) {
        QUndoCommand *command = new QUndoCommand( i18n( "Change page layout" ) );
        d->viewMode->changePageLayout( dialog.pageLayout(), dialog.applyToDocument(), command );

        d->canvas->addCommand( command );
    }

}

void KoPAView::slotZoomChanged( KoZoomMode::Mode mode, qreal zoom )
{
    Q_UNUSED(zoom);
    if (d->activePage) {
        if (mode == KoZoomMode::ZOOM_PAGE) {
            KoPageLayout &layout = d->activePage->pageLayout();
            QRectF pageRect( 0, 0, layout.width, layout.height );
            d->canvasController->ensureVisible(d->canvas->viewConverter()->documentToView(pageRect));
        } else if (mode == KoZoomMode::ZOOM_WIDTH) {
            // horizontally center the page
            KoPageLayout &layout = d->activePage->pageLayout();
            QRectF pageRect( 0, 0, layout.width, layout.height );
            QRect viewRect = d->canvas->viewConverter()->documentToView(pageRect).toRect();
            viewRect.translate(d->canvas->documentOrigin());
            QRect currentVisible(qMax(0, -d->canvasController->canvasOffsetX()), qMax(0, -d->canvasController->canvasOffsetY()), d->canvasController->visibleWidth(), d->canvasController->visibleHeight());
            int horizontalMove = viewRect.center().x() - currentVisible.center().x();
            d->canvasController->pan(QPoint(horizontalMove, 0));
        }
        kopaCanvas()->update();
    }
}

void KoPAView::configure()
{
    QPointer<KoPAConfigureDialog> dialog(new KoPAConfigureDialog(this));
    dialog->exec();
    delete dialog;
    // TODO update canvas
}

void KoPAView::setMasterMode( bool master )
{
    d->viewMode->setMasterMode( master );
    if (shell()) {
        d->documentStructureDocker->setMasterMode(master);
    }
    d->actionMasterPage->setEnabled(!master);

    QList<KoPAPageBase*> pages = d->doc->pages( master );
    d->actionDeletePage->setEnabled( pages.size() > 1 );
}

KoShapeManager* KoPAView::shapeManager() const
{
    return d->canvas->shapeManager();
}

KoPAViewMode* KoPAView::viewMode() const
{
    return d->viewMode;
}

void KoPAView::setViewMode( KoPAViewMode* mode )
{
    Q_ASSERT( mode );
    if ( mode != d->viewMode )
    {
        KoPAViewMode * previousViewMode = d->viewMode;
        d->viewMode->deactivate();
        d->viewMode = mode;
        d->viewMode->activate( previousViewMode );
    }
}

KoShapeManager* KoPAView::masterShapeManager() const
{
    return d->canvas->masterShapeManager();
}

void KoPAView::updateActivePage( KoPAPageBase * page )
{
    d->viewMode->updateActivePage( page );
}

void KoPAView::reinitDocumentDocker()
{
    if (shell()) {
        d->documentStructureDocker->setActivePage( d->activePage );
    }
}

void KoPAView::doUpdateActivePage( KoPAPageBase * page )
{
    // save the old offset into the page so we can use it also on the new page
    QPoint scrollValue(d->canvasController->scrollBarValue());

    bool pageChanged = page != d->activePage;
    setActivePage( page );

    d->canvas->updateSize();
    KoPageLayout &layout = d->activePage->pageLayout();
    d->horizontalRuler->setRulerLength(layout.width);
    d->verticalRuler->setRulerLength(layout.height);
    d->horizontalRuler->setActiveRange(layout.leftMargin, layout.width - layout.rightMargin);
    d->verticalRuler->setActiveRange(layout.topMargin, layout.height - layout.bottomMargin);

    QSizeF pageSize( layout.width, layout.height );
    d->canvas->setDocumentOrigin(QPointF(layout.width, layout.height));
    // the page is in the center of the canvas
    d->zoomController->setDocumentSize(pageSize * 3);
    d->zoomController->setPageSize(pageSize);
    d->canvas->resourceManager()->setResource( KoCanvasResource::PageSize, pageSize );

    d->canvas->update();

    updatePageNavigationActions();

    if ( pageChanged ) {
        emit activePageChanged();
    }

    pageOffsetChanged();
    d->canvasController->setScrollBarValue(scrollValue);
}

void KoPAView::setActivePage( KoPAPageBase* page )
{
    if ( !page )
        return;

    bool pageChanged = page != d->activePage;

    shapeManager()->removeAdditional( d->activePage );
    d->activePage = page;
    shapeManager()->addAdditional( d->activePage );
    QList<KoShape*> shapes = page->shapes();
    shapeManager()->setShapes(shapes, KoShapeManager::AddWithoutRepaint);
    //Make the top most layer active
    if ( !shapes.isEmpty() ) {
        KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>( shapes.last() );
        shapeManager()->selection()->setActiveLayer( layer );
    }

    // if the page is not a master page itself set shapes of the master page
    KoPAPage * paPage = dynamic_cast<KoPAPage *>( page );
    if ( paPage ) {
        KoPAMasterPage * masterPage = paPage->masterPage();
        QList<KoShape*> masterShapes = masterPage->shapes();
        masterShapeManager()->setShapes(masterShapes, KoShapeManager::AddWithoutRepaint);
        //Make the top most layer active
        if ( !masterShapes.isEmpty() ) {
            KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>( masterShapes.last() );
            masterShapeManager()->selection()->setActiveLayer( layer );
        }
    }
    else {
        // if the page is a master page no shapes are in the masterShapeManager
        masterShapeManager()->setShapes( QList<KoShape*>() );
    }

    if ( shell() && pageChanged ) {
        d->documentStructureDocker->setActivePage(d->activePage);
    }
    
    // Set the current page number in the canvas resource provider
    d->canvas->resourceManager()->setResource( KoCanvasResource::CurrentPage, d->doc->pageIndex(d->activePage)+1 );
}

void KoPAView::navigatePage( KoPageApp::PageNavigation pageNavigation )
{
    KoPAPageBase * newPage = d->doc->pageByNavigation( d->activePage, pageNavigation );

    if ( newPage != d->activePage ) {
        updateActivePage( newPage );
    }
}

KoPrintJob * KoPAView::createPrintJob()
{
    return new KoPAPrintJob(this);
}

void KoPAView::pageOffsetChanged()
{
    QPoint documentOrigin(d->canvas->documentOrigin());
    d->horizontalRuler->setOffset(d->canvasController->canvasOffsetX() + documentOrigin.x());
    d->verticalRuler->setOffset(d->canvasController->canvasOffsetY() + documentOrigin.y());
}

void KoPAView::updateMousePosition(const QPoint& position)
{
    QPoint canvasOffset( d->canvasController->canvasOffsetX(), d->canvasController->canvasOffsetY() );
    // the offset is positive it the canvas is shown fully visible
    canvasOffset.setX(canvasOffset.x() < 0 ? canvasOffset.x(): 0);
    canvasOffset.setY(canvasOffset.y() < 0 ? canvasOffset.y(): 0);
    QPoint viewPos = position - canvasOffset;

    d->horizontalRuler->updateMouseCoordinate(viewPos.x());
    d->verticalRuler->updateMouseCoordinate(viewPos.y());

    // Update the selection borders in the rulers while moving with the mouse
    if(d->canvas->shapeManager()->selection() && (d->canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = d->canvas->shapeManager()->selection()->boundingRect();
        d->horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        d->verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    }
}

void KoPAView::selectionChanged()
{
    // Show the borders of the selection in the rulers
    if(d->canvas->shapeManager()->selection() && (d->canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = d->canvas->shapeManager()->selection()->boundingRect();
        d->horizontalRuler->setShowSelectionBorders(true);
        d->verticalRuler->setShowSelectionBorders(true);
        d->horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        d->verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    } else {
        d->horizontalRuler->setShowSelectionBorders(false);
        d->verticalRuler->setShowSelectionBorders(false);
    }
}

void KoPAView::setShowRulers(bool show)
{
    d->horizontalRuler->setVisible(show);
    d->verticalRuler->setVisible(show);

    d->viewRulers->setChecked(show);
    d->doc->setRulersVisible(show);
}

void KoPAView::insertPage()
{
    KoPAPageBase * page = 0;
    if ( d->viewMode->masterMode() ) {
        KoPAMasterPage * masterPage = d->doc->newMasterPage();
        masterPage->setBackground( new KoColorBackground( Qt::white ) );
        // use the layout of the current active page for the new page
        KoPageLayout & layout = masterPage->pageLayout();
        KoPAMasterPage * activeMasterPage = dynamic_cast<KoPAMasterPage *>( d->activePage );
        if ( activeMasterPage ) {
            layout = activeMasterPage->pageLayout();
        }
        page = masterPage;
    }
    else {
        KoPAPage * activePage = dynamic_cast<KoPAPage*>( d->activePage );
        KoPAMasterPage * masterPage = activePage->masterPage();
        page = d->doc->newPage( masterPage );
    }

    KoPAPageInsertCommand * command = new KoPAPageInsertCommand( d->doc, page, d->activePage );
    d->canvas->addCommand( command );

    doUpdateActivePage(page);
}

void KoPAView::copyPage()
{
    QList<KoPAPageBase *> pages;
    pages.append( d->activePage );
    KoPAOdfPageSaveHelper saveHelper( d->doc, pages );
    KoDrag drag;
    drag.setOdf( KoOdf::mimeType( d->doc->documentType() ), saveHelper );
    drag.addToClipboard();
}

void KoPAView::deletePage()
{
    if ( !isMasterUsed( d->activePage ) ) {
        d->doc->removePage( d->activePage );
    }
}

void KoPAView::setActionEnabled( int actions, bool enable )
{
    if ( actions & ActionInsertPage )
    {
        d->actionInsertPage->setEnabled( enable );
    }
    if ( actions & ActionCopyPage )
    {
        d->actionCopyPage->setEnabled( enable );
    }
    if ( actions & ActionDeletePage )
    {
        d->actionDeletePage->setEnabled( enable );
    }
    if ( actions & ActionViewShowMasterPages )
    {
        d->actionViewShowMasterPages->setEnabled( enable );
    }
    if ( actions & ActionFormatMasterPage )
    {
        d->actionMasterPage->setEnabled( enable );
    }
}

QPixmap KoPAView::pageThumbnail(KoPAPageBase* page, const QSize& size)
{
    return d->doc->pageThumbnail(page, size);
}

bool KoPAView::exportPageThumbnail( KoPAPageBase * page, const KUrl& url, const QSize& size,
                                    const char * format, int quality )
{
    bool res = false;
    QPixmap pix = d->doc->pageThumbnail( page, size );
    if ( !pix.isNull() ) {
        // Depending on the desired target size due to rounding
        // errors during zoom the resulting pixmap *might* be
        // 1 pixel or 2 pixels wider/higher than desired: we just
        // remove the additional columns/rows.  This can be done
        // since KPresenter is leaving a minimal border below/at
        // the right of the image anyway.
        if ( size != pix.size() ) {
            pix = pix.copy( 0, 0, size.width(), size.height() );
        }
        // save the pixmap to the desired file
        KUrl fileUrl( url );
        if ( fileUrl.protocol().isEmpty() ) {
            fileUrl.setProtocol( "file" );
        }
        const bool bLocalFile = fileUrl.isLocalFile();
        KTemporaryFile* tmpFile = bLocalFile ? 0 : new KTemporaryFile();
        if( bLocalFile || tmpFile->open() ) {
            QFile file( bLocalFile ? fileUrl.path() : tmpFile->fileName() );
            if ( file.open( QIODevice::ReadWrite ) ) {
                res = pix.save( &file, format, quality );
                file.close();
            }
            if ( !bLocalFile ) {
                if ( res ) {
                    res = KIO::NetAccess::upload( tmpFile->fileName(), fileUrl, this );
                }
            }
        }
        if ( !bLocalFile ) {
            delete tmpFile;
        }
   }
   return res;
}

KoPADocumentStructureDocker* KoPAView::documentStructureDocker() const
{
    return d->documentStructureDocker;
}

void KoPAView::clipboardDataChanged()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();
    bool paste = false;

    if (data)
    {
        // TODO see if we can use the KoPasteController instead of having to add this feature in each koffice app.
        QStringList mimeTypes = d->canvas->toolProxy()->supportedPasteMimeTypes();
        mimeTypes << KoOdf::mimeType( KoOdf::Graphics );
        mimeTypes << KoOdf::mimeType( KoOdf::Presentation );

        foreach(const QString & mimeType, mimeTypes)
        {
            if ( data->hasFormat( mimeType ) ) {
                paste = true;
                break;
            }
        }

    }

    d->editPaste->setEnabled(paste);
}

void KoPAView::partActivateEvent(KParts::PartActivateEvent* event)
{
    if ( event->widget() == this ) {
        if ( event->activated() ) {
            clipboardDataChanged();
            connect( d->find, SIGNAL( findDocumentSetNext( QTextDocument * ) ),
                     this,    SLOT( findDocumentSetNext( QTextDocument * ) ) );
            connect( d->find, SIGNAL( findDocumentSetPrevious( QTextDocument * ) ),
                     this,    SLOT( findDocumentSetPrevious( QTextDocument * ) ) );
        }
        else {
            disconnect( d->find, 0, 0, 0 );
        }
    }

    KoView::partActivateEvent(event);
}

void KoPAView::goToPreviousPage()
{
    navigatePage( KoPageApp::PagePrevious );
}

void KoPAView::goToNextPage()
{
    navigatePage( KoPageApp::PageNext );
}

void KoPAView::goToFirstPage()
{
    navigatePage( KoPageApp::PageFirst );
}

void KoPAView::goToLastPage()
{
    navigatePage( KoPageApp::PageLast );
}

void KoPAView::findDocumentSetNext( QTextDocument * document )
{
    KoPAPageBase * page = 0;
    KoShape * startShape = 0;
    KoTextDocumentLayout *lay = document ? qobject_cast<KoTextDocumentLayout*>(document->documentLayout()) : 0;
    if ( lay != 0 ) {
        startShape = lay->shapes().value( 0 );
        Q_ASSERT( startShape->shapeId() == "TextShapeID" );
        page = d->doc->pageByShape( startShape );
        if ( d->doc->pageIndex( page ) == -1 ) {
            page = 0;
        }
    }

    if ( page == 0 ) {
        page = d->activePage;
        startShape = page;
    }

    KoShape * shape = startShape;

    do {
        // find next text shape
        shape = KoShapeTraversal::nextShape( shape, "TextShapeID" );
        // get next text shape
        if ( shape != 0 ) {
            if ( page != d->activePage ) {
                setActivePage( page );
                d->canvas->update();
            }
            KoSelection* selection = kopaCanvas()->shapeManager()->selection();
            selection->deselectAll();
            selection->select( shape );
            // TODO can this be done nicer? is there a way to get the shape id and the tool id from the shape?
            KoToolManager::instance()->switchToolRequested( "TextToolFactory_ID" );
            break;
        }
        else {
            //if none is found go to next page and try again
            if ( d->doc->pageIndex( page ) < d->doc->pages().size() - 1 ) {
                // TODO use also master slides
                page = d->doc->pageByNavigation( page, KoPageApp::PageNext );
            }
            else {
                page = d->doc->pageByNavigation( page, KoPageApp::PageFirst );
            }
            shape = page;
        }
        // do until you find the same start shape or you are on the same page again only if there was none
    } while ( page != startShape );
}

void KoPAView::findDocumentSetPrevious( QTextDocument * document )
{
    KoPAPageBase * page = 0;
    KoShape * startShape = 0;
    KoTextDocumentLayout *lay = document ? qobject_cast<KoTextDocumentLayout*>(document->documentLayout()) : 0;
    if ( lay != 0 ) {
        startShape = lay->shapes().value( 0 );
        Q_ASSERT( startShape->shapeId() == "TextShapeID" );
        page = d->doc->pageByShape( startShape );
        if ( d->doc->pageIndex( page ) == -1 ) {
            page = 0;
        }
    }

    bool check = false;
    if ( page == 0 ) {
        page = d->activePage;
        startShape = KoShapeTraversal::last( page );
        check = true;
    }

    KoShape * shape = startShape;

    do {
        if ( !check || shape->shapeId() != "TextShapeID" ) {
            shape = KoShapeTraversal::previousShape( shape, "TextShapeID" );
        }
        // get next text shape
        if ( shape != 0 ) {
            if ( page != d->activePage ) {
                setActivePage( page );
                d->canvas->update();
            }
            KoSelection* selection = kopaCanvas()->shapeManager()->selection();
            selection->deselectAll();
            selection->select( shape );
            // TODO can this be done nicer? is there a way to get the shape id and the tool id from the shape?
            KoToolManager::instance()->switchToolRequested( "TextToolFactory_ID" );
            break;
        }
        else {
            //if none is found go to next page and try again
            if ( d->doc->pageIndex( page ) > 0 ) {
                // TODO use also master slides
                page = d->doc->pageByNavigation( page, KoPageApp::PagePrevious );
            }
            else {
                page = d->doc->pageByNavigation( page, KoPageApp::PageLast );
            }
            shape = KoShapeTraversal::last( page );
            check = true;
        }
        // do until you find the same start shape or you are on the same page again only if there was none
    } while ( shape != startShape );
}

void KoPAView::updatePageNavigationActions()
{
    int index = d->doc->pageIndex(activePage());
    int pageCount = d->doc->pages(d->viewMode->masterMode()).count();

    actionCollection()->action("page_previous")->setEnabled(index > 0);
    actionCollection()->action("page_first")->setEnabled(index > 0);
    actionCollection()->action("page_next")->setEnabled(index < pageCount - 1);
    actionCollection()->action("page_last")->setEnabled(index < pageCount - 1);
}

bool KoPAView::isMasterUsed( KoPAPageBase * page )
{
    KoPAMasterPage * master = dynamic_cast<KoPAMasterPage *>( page );

    bool used = false;

    if ( master ) {
        QList<KoPAPageBase*> pages = d->doc->pages();
        foreach( KoPAPageBase * page, pages ) {
            KoPAPage * p = dynamic_cast<KoPAPage *>( page );
            Q_ASSERT( p );
            if ( p && p->masterPage() == master ) {
                used = true;
                break;
            }
        }
    }

    return used;
}

#include <KoPAView.moc>
