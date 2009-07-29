/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAView.h"

#include <QGridLayout>
#include <QToolBar>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QLabel>

#include <KoCanvasController.h>
#include <KoCanvasResourceProvider.h>
#include <KoColorBackground.h>
#include <KoFind.h>
#include <KoTextDocumentLayout.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomHandler.h>
#include <KoToolBoxFactory.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoZoomAction.h>
#include <KoZoomController.h>
#include <KoInlineTextObjectManager.h>
#include <KoSelection.h>
#include <KoToolDocker.h>
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

KoPAView::KoPAView( KoPADocument *document, QWidget *parent )
: KoView( document, parent )
, m_doc( document )
, m_activePage( 0 )
, m_viewMode( 0 )
{
    initGUI();
    initActions();

    if ( m_doc->pageCount() > 0 )
        doUpdateActivePage( m_doc->pageByIndex( 0, false ) );
}

KoPAView::~KoPAView()
{
    KoToolManager::instance()->removeCanvasController( m_canvasController );
    delete m_zoomController;
    // delete only the view mode normal, let the derived class delete the currently
    // active view mode if it is not view mode normal
    delete m_viewModeNormal;
}


KoPAPageBase* KoPAView::activePage() const
{
    return m_activePage;
}

void KoPAView::updateReadWrite( bool readwrite )
{
    Q_UNUSED( readwrite );
}

void KoPAView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout( this );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 0 );
    setLayout( gridLayout );

    m_canvas = new KoPACanvas( this, m_doc );
    m_canvasController = new KoCanvasController( this );
    m_canvasController->setCanvas( m_canvas );
    KoToolManager::instance()->addController( m_canvasController );
    KoToolManager::instance()->registerTools( actionCollection(), m_canvasController );

    m_zoomController = new KoZoomController( m_canvasController, &m_zoomHandler, actionCollection());
    connect( m_zoomController, SIGNAL( zoomChanged( KoZoomMode::Mode, qreal ) ),
             this, SLOT( slotZoomChanged( KoZoomMode::Mode, qreal ) ) );

    m_zoomAction = m_zoomController->zoomAction();

    // set up status bar message
    m_status = new QLabel( QString(), statusBar() );
    m_status->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    m_status->setMinimumWidth( 300 );
    addStatusBarItem( m_status, 1 );
    connect( KoToolManager::instance(), SIGNAL( changedStatusText( const QString & ) ),
             m_status, SLOT( setText( const QString & ) ) );
    addStatusBarItem( m_zoomAction->createWidget( statusBar() ), 0, true );

    m_zoomController->setZoomMode( KoZoomMode::ZOOM_PAGE );

    m_viewModeNormal = new KoPAViewModeNormal( this, m_canvas );
    m_viewMode = m_viewModeNormal;

    //Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter( m_canvas ));
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_doc->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter( m_canvas ));
    m_verticalRuler->setUnit(m_doc->unit());
    m_verticalRuler->setShowMousePosition(true);

    new KoRulerController(m_horizontalRuler, m_canvas->resourceProvider());

    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_horizontalRuler, SLOT(setUnit(KoUnit)));
    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_verticalRuler, SLOT(setUnit(KoUnit)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget( m_canvasController, 1, 1 );

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            m_horizontalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            m_verticalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(sizeChanged(const QSize&)),
             this, SLOT(canvasControllerResized()));
    connect(m_canvasController, SIGNAL(canvasMousePositionChanged(const QPoint&)),
             this, SLOT(updateMousePosition(const QPoint&)));
    connect(m_verticalRuler, SIGNAL(guideLineCreated(Qt::Orientation, int)),
            m_canvasController, SLOT(addGuideLine(Qt::Orientation, int)));
    connect(m_horizontalRuler, SIGNAL(guideLineCreated(Qt::Orientation, int)),
            m_canvasController, SLOT(addGuideLine(Qt::Orientation, int)));

    KoToolBoxFactory toolBoxFactory(m_canvasController, i18n("Tools") );
    createDockWidget( &toolBoxFactory );

    KoDockerManager *dockerMng = dockerManager();
    if (!dockerMng) {
        dockerMng = new KoDockerManager(this);
        setDockerManager(dockerMng);
    }

    connect( m_canvasController, SIGNAL( toolOptionWidgetsChanged(const QMap<QString, QWidget *> &, QWidget*) ),
             dockerMng, SLOT( newOptionWidgets(const  QMap<QString, QWidget *> &, QWidget*) ) );

    connect(shapeManager(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_canvas, SIGNAL(documentSize(const QSize&)), m_canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(m_canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            m_canvas, SLOT(setDocumentOffset(const QPoint&)));

    if (shell()) {
        KoPADocumentStructureDockerFactory structureDockerFactory( KoDocumentSectionView::ThumbnailMode, m_doc->pageType() );
        m_documentStructureDocker = qobject_cast<KoPADocumentStructureDocker*>( createDockWidget( &structureDockerFactory ) );
        connect( shell()->partManager(), SIGNAL( activePartChanged( KParts::Part * ) ),
                m_documentStructureDocker, SLOT( setPart( KParts::Part * ) ) );
        connect(m_documentStructureDocker, SIGNAL(pageChanged(KoPAPageBase*)), this, SLOT(updateActivePage(KoPAPageBase*)));
        connect(m_documentStructureDocker, SIGNAL(dockerReset()), this, SLOT(reinitDocumentDocker()));

        KoToolManager::instance()->requestToolActivation( m_canvasController );
    }
}

void KoPAView::initActions()
{
    KAction *action = actionCollection()->addAction( KStandardAction::Cut, "edit_cut", 0, 0);
    new KoCutController(kopaCanvas(), action);
    action = actionCollection()->addAction( KStandardAction::Copy, "edit_copy", 0, 0 );
    new KoCopyController(kopaCanvas(), action);
    m_editPaste = actionCollection()->addAction( KStandardAction::Paste, "edit_paste", this, SLOT( editPaste() ) );
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(m_canvas->toolProxy(), SIGNAL(toolChanged(const QString&)), this, SLOT(clipboardDataChanged()));
    clipboardDataChanged();
    actionCollection()->addAction(KStandardAction::SelectAll,  "edit_select_all", this, SLOT(editSelectAll()));
    actionCollection()->addAction(KStandardAction::Deselect,  "edit_deselect_all", this, SLOT(editDeselectAll()));

    m_deleteSelectionAction = new KAction(KIcon("edit-delete"), i18n("D&elete"), this);
    actionCollection()->addAction("edit_delete", m_deleteSelectionAction );
    m_deleteSelectionAction->setShortcut(QKeySequence("Del"));
    m_deleteSelectionAction->setEnabled(false);
    connect(m_deleteSelectionAction, SIGNAL(triggered()), this, SLOT(editDeleteSelection()));
    connect(m_canvas->toolProxy(), SIGNAL(selectionChanged(bool)), m_deleteSelectionAction, SLOT(setEnabled(bool)));

    KToggleAction *showGrid= m_doc->gridData().gridToggleAction(m_canvas);
    actionCollection()->addAction("view_grid", showGrid );

    m_actionViewSnapToGrid = new KToggleAction(i18n("Snap to Grid"), this);
    m_actionViewSnapToGrid->setChecked(m_doc->gridData().snapToGrid());
    actionCollection()->addAction("view_snaptogrid", m_actionViewSnapToGrid);
    connect( m_actionViewSnapToGrid, SIGNAL( triggered( bool ) ), this, SLOT (viewSnapToGrid( bool )));

    m_actionViewShowGuides  = new KToggleAction( KIcon( "guides" ), i18n( "Show Guides" ), this );
    m_actionViewShowGuides->setChecked( m_doc->guidesData().showGuideLines() );
    m_actionViewShowGuides->setCheckedState( KGuiItem( i18n( "Hide Guides" ) ) );
    m_actionViewShowGuides->setToolTip( i18n( "Shows or hides guides" ) );
    actionCollection()->addAction( "view_show_guides", m_actionViewShowGuides );
    connect( m_actionViewShowGuides, SIGNAL(triggered(bool)), this, SLOT(viewGuides(bool)));

    m_actionViewShowMasterPages = new KToggleAction(i18n( "Show Master Pages" ), this );
    actionCollection()->addAction( "view_masterpages", m_actionViewShowMasterPages );
    connect( m_actionViewShowMasterPages, SIGNAL( triggered( bool ) ), this, SLOT( setMasterMode( bool ) ) );

    m_viewRulers  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection()->addAction("view_rulers", m_viewRulers );
    m_viewRulers->setToolTip(i18n("Show/hide the view's rulers"));
    connect(m_viewRulers, SIGNAL(triggered(bool)), this, SLOT(setShowRulers(bool)));
    setShowRulers(m_doc->rulersVisible());

    m_actionInsertPage = new KAction( KIcon("document-new"), i18n( "Insert Page" ), this );
    actionCollection()->addAction( "page_insertpage", m_actionInsertPage );
    m_actionInsertPage->setToolTip( i18n( "Insert a new page after the current one" ) );
    m_actionInsertPage->setWhatsThis( i18n( "Insert a new page after the current one" ) );
    connect( m_actionInsertPage, SIGNAL( triggered() ), this, SLOT( insertPage() ) );

    m_actionCopyPage = new KAction( i18n( "Copy Page" ), this );
    actionCollection()->addAction( "page_copypage", m_actionCopyPage );
    m_actionCopyPage->setToolTip( i18n( "Copy the current page" ) );
    m_actionCopyPage->setWhatsThis( i18n( "Copy the current page" ) );
    connect( m_actionCopyPage, SIGNAL( triggered() ), this, SLOT( copyPage() ) );

    m_actionDeletePage = new KAction( i18n( "Delete Page" ), this );
    m_actionDeletePage->setEnabled( m_doc->pageCount() > 1 );
    actionCollection()->addAction( "page_deletepage", m_actionDeletePage );
    m_actionDeletePage->setToolTip( i18n( "Delete the current page" ) );
    m_actionDeletePage->setWhatsThis( i18n( "Delete the current page" ) );
    connect( m_actionDeletePage, SIGNAL( triggered() ), this, SLOT( deletePage() ) );

    m_actionMasterPage = new KAction(i18n("Master Page..."), this);
    actionCollection()->addAction("format_masterpage", m_actionMasterPage);
    connect(m_actionMasterPage, SIGNAL(triggered()), this, SLOT(formatMasterPage()));

    m_actionPageLayout = new KAction( i18n( "Page Layout..." ), this );
    actionCollection()->addAction( "format_pagelayout", m_actionPageLayout );
    connect( m_actionPageLayout, SIGNAL( triggered() ), this, SLOT( formatPageLayout() ) );

    actionCollection()->addAction(KStandardAction::Prior,  "page_previous", this, SLOT(goToPreviousPage()));
    actionCollection()->addAction(KStandardAction::Next,  "page_next", this, SLOT(goToNextPage()));
    actionCollection()->addAction(KStandardAction::FirstPage,  "page_first", this, SLOT(goToFirstPage()));
    actionCollection()->addAction(KStandardAction::LastPage,  "page_last", this, SLOT(goToLastPage()));

    KActionMenu *actionMenu = new KActionMenu(i18n("Variable"), this);
    foreach(QAction *action, m_doc->inlineTextObjectManager()->createInsertVariableActions(m_canvas))
        actionMenu->addAction(action);
    actionCollection()->addAction("insert_variable", actionMenu);

    KAction * am = new KAction(i18n("Import Document..."), this);
    actionCollection()->addAction("import_document", am);
    connect(am, SIGNAL(triggered()), this, SLOT(importDocument()));

    m_find = new KoFind( this, m_canvas->resourceProvider(), actionCollection() );
}

void KoPAView::importDocument()
{
    KFileDialog *dialog = new KFileDialog( KUrl("kfiledialog:///OpenDialog"),QString(), this );
    dialog->setObjectName( "file dialog" );
    dialog->setMode( KFile::File );
    if ( m_doc->pageType() == KoPageApp::Slide ) {
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
    mimeFilter << KoOdf::mimeType( m_doc->documentType() ) << KoOdf::templateMimeType( m_doc->documentType() );
#else
    mimeFilter = KoFilterManager::mimeFilter( KoDocument::readNativeFormatMimeType(m_doc->componentData()), KoFilterManager::Import,
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
            data.setData( KoOdf::mimeType( m_doc->documentType() ), ba);
            KoPAPastePage paste( m_doc,m_activePage );
            if ( ! paste.paste( m_doc->documentType(), &data ) ) {
                KMessageBox::error( 0L, i18n("Could not import\n%1", url.pathOrUrl()));
            }
        }
        else {
            KMessageBox::error( 0L, i18n("Could not import\n%1", url.pathOrUrl()));
        }
    }
    delete dialog;
}

void KoPAView::viewSnapToGrid(bool snap)
{
    m_doc->gridData().setSnapToGrid(snap);
    m_actionViewSnapToGrid->setChecked(snap);
}

void KoPAView::viewGuides(bool show)
{
    m_doc->guidesData().setShowGuideLines(show);
    m_canvas->update();
}

void KoPAView::editPaste()
{
    if ( !m_canvas->toolProxy()->paste() ) {
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
            KoPAPastePage paste( m_doc, m_activePage );
            paste.paste( documentTypes[i], data );
            break;
        }
    }
}

void KoPAView::editDeleteSelection()
{
    m_canvas->toolProxy()->deleteSelection();
}

void KoPAView::editSelectAll()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( !selection )
        return;

    QList<KoShape*> shapes = activePage()->childShapes();

    foreach( KoShape *shape, shapes ) {
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer *>( shape );

        if ( layer ) {
            QList<KoShape*> layerShapes( layer->childShapes() );
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
    KoPAPage *page = dynamic_cast<KoPAPage *>(m_activePage);
    Q_ASSERT(page);
    KoPAMasterPageDialog *dialog = new KoPAMasterPageDialog(m_doc, page->masterPage(), m_canvas);

    if (dialog->exec() == QDialog::Accepted) {
        KoPAMasterPage *masterPage = dialog->selectedMasterPage();
        KoPAPage *page = dynamic_cast<KoPAPage *>(m_activePage);
        if (page) {
            KoPAChangeMasterPageCommand * command = new KoPAChangeMasterPageCommand( m_doc, page, masterPage );
            m_canvas->addCommand( command );
        }
    }

    delete dialog;
}

void KoPAView::formatPageLayout()
{
    const KoPageLayout &pageLayout = m_viewMode->activePageLayout();

    KoPAPageLayoutDialog dialog( m_doc, pageLayout, m_canvas );

    if ( dialog.exec() == QDialog::Accepted ) {
        QUndoCommand *command = new QUndoCommand( i18n( "Change page layout" ) );
        m_viewMode->changePageLayout( dialog.pageLayout(), dialog.applyToDocument(), command );

        m_canvas->addCommand( command );
    }

}

void KoPAView::slotZoomChanged( KoZoomMode::Mode mode, qreal zoom )
{
    Q_UNUSED(mode);
    Q_UNUSED(zoom);
    kopaCanvas()->update();
}

void KoPAView::setMasterMode( bool master )
{
    m_viewMode->setMasterMode( master );
    if (shell()) {
        m_documentStructureDocker->setMasterMode(master);
    }
    m_actionMasterPage->setEnabled(!master);

    QList<KoPAPageBase*> pages = m_doc->pages( master );
    m_actionDeletePage->setEnabled( pages.size() > 1 );
}

KoShapeManager* KoPAView::shapeManager() const
{
    return m_canvas->shapeManager();
}

KoPAViewMode* KoPAView::viewMode() const
{
    return m_viewMode;
}

void KoPAView::setViewMode( KoPAViewMode* mode )
{
    Q_ASSERT( mode );
    if ( mode != m_viewMode )
    {
        KoPAViewMode * previousViewMode = m_viewMode;
        m_viewMode->deactivate();
        m_viewMode = mode;
        m_viewMode->activate( previousViewMode );
    }
}

KoShapeManager* KoPAView::masterShapeManager() const
{
    return m_canvas->masterShapeManager();
}

void KoPAView::updateActivePage( KoPAPageBase * page )
{
    m_viewMode->updateActivePage( page );
}

void KoPAView::reinitDocumentDocker()
{
    if (shell()) {
        m_documentStructureDocker->setActivePage( m_activePage );
    }
}

void KoPAView::doUpdateActivePage( KoPAPageBase * page )
{
    bool pageChanged = page != m_activePage;
    setActivePage( page );

    m_canvas->updateSize();
    KoPageLayout &layout = m_activePage->pageLayout();
    m_horizontalRuler->setRulerLength(layout.width);
    m_verticalRuler->setRulerLength(layout.height);
    m_horizontalRuler->setActiveRange(layout.left, layout.width - layout.right);
    m_verticalRuler->setActiveRange(layout.top, layout.height - layout.bottom);

    QSizeF pageSize( layout.width, layout.height );
    m_zoomController->setPageSize( pageSize );
    m_zoomController->setDocumentSize( pageSize );
    m_canvas->resourceProvider()->setResource( KoCanvasResource::PageSize, pageSize );

    m_canvas->update();

    updatePageNavigationActions();

    if ( pageChanged ) {
        emit activePageChanged();
    }
}

void KoPAView::setActivePage( KoPAPageBase* page )
{
    if ( !page )
        return;

    bool pageChanged = page != m_activePage;

    shapeManager()->removeAdditional( m_activePage );
    m_activePage = page;
    shapeManager()->addAdditional( m_activePage );
    QList<KoShape*> shapes = page->childShapes();
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
        QList<KoShape*> masterShapes = masterPage->childShapes();
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
        m_documentStructureDocker->setActivePage(m_activePage);
    }
}

void KoPAView::navigatePage( KoPageApp::PageNavigation pageNavigation )
{
    KoPAPageBase * newPage = m_doc->pageByNavigation( m_activePage, pageNavigation );

    if ( newPage != m_activePage ) {
        doUpdateActivePage( newPage );
    }
}

KoPrintJob * KoPAView::createPrintJob()
{
    return new KoPAPrintJob(this);
}

void KoPAView::canvasControllerResized()
{
    m_horizontalRuler->setOffset( m_canvasController->canvasOffsetX() );
    m_verticalRuler->setOffset( m_canvasController->canvasOffsetY() );
}

void KoPAView::updateMousePosition(const QPoint& position)
{
    QPoint canvasOffset( m_canvasController->canvasOffsetX(), m_canvasController->canvasOffsetY() );
    // the offset is positive it the canvas is shown fully visible
    canvasOffset.setX(canvasOffset.x() < 0 ? canvasOffset.x(): 0);
    canvasOffset.setY(canvasOffset.y() < 0 ? canvasOffset.y(): 0);
    QPoint viewPos = position - canvasOffset;

    m_horizontalRuler->updateMouseCoordinate(viewPos.x());
    m_verticalRuler->updateMouseCoordinate(viewPos.y());

    // Update the selection borders in the rulers while moving with the mouse
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    }
}

void KoPAView::selectionChanged()
{
    // Show the borders of the selection in the rulers
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->setShowSelectionBorders(true);
        m_verticalRuler->setShowSelectionBorders(true);
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    } else {
        m_horizontalRuler->setShowSelectionBorders(false);
        m_verticalRuler->setShowSelectionBorders(false);
    }
}

void KoPAView::setShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);

    m_viewRulers->setChecked(show);
    m_doc->setRulersVisible(show);
}

void KoPAView::insertPage()
{
    KoPAPageBase * page = 0;
    if ( m_viewMode->masterMode() ) {
        KoPAMasterPage * masterPage = m_doc->newMasterPage();
        masterPage->setBackground( new KoColorBackground( Qt::white ) );
        // use the layout of the current active page for the new page
        KoPageLayout & layout = masterPage->pageLayout();
        KoPAMasterPage * activeMasterPage = dynamic_cast<KoPAMasterPage *>( m_activePage );
        if ( activeMasterPage ) {
            layout = activeMasterPage->pageLayout();
        }
        page = masterPage;
    }
    else {
        KoPAPage * activePage = dynamic_cast<KoPAPage*>( m_activePage );
        KoPAMasterPage * masterPage = activePage->masterPage();
        page = m_doc->newPage( masterPage );
    }

    KoPAPageInsertCommand * command = new KoPAPageInsertCommand( m_doc, page, m_activePage );
    m_canvas->addCommand( command );

    doUpdateActivePage(page);
}

void KoPAView::copyPage()
{
    QList<KoPAPageBase *> pages;
    pages.append( m_activePage );
    KoPAOdfPageSaveHelper saveHelper( m_doc, pages );
    KoDrag drag;
    drag.setOdf( KoOdf::mimeType( m_doc->documentType() ), saveHelper );
    drag.addToClipboard();
}

void KoPAView::deletePage()
{
    if ( !isMasterUsed( m_activePage ) ) {
        m_doc->removePage( m_activePage );
    }
}

void KoPAView::setActionEnabled( int actions, bool enable )
{
    if ( actions & ActionInsertPage )
    {
        m_actionInsertPage->setEnabled( enable );
    }
    if ( actions & ActionCopyPage )
    {
        m_actionCopyPage->setEnabled( enable );
    }
    if ( actions & ActionDeletePage )
    {
        m_actionDeletePage->setEnabled( enable );
    }
    if ( actions & ActionViewShowMasterPages )
    {
        m_actionViewShowMasterPages->setEnabled( enable );
    }
    if ( actions & ActionFormatMasterPage )
    {
        m_actionMasterPage->setEnabled( enable );
    }
}

bool KoPAView::exportPageThumbnail( KoPAPageBase * page, const KUrl& url, const QSize& size,
                                    const char * format, int quality )
{
    bool res = false;
    QPixmap pix = page->thumbnail( size );
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
    return m_documentStructureDocker;
}

void KoPAView::clipboardDataChanged()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();
    bool paste = false;

    if (data)
    {
        // TODO see if we can use the KoPasteController instead of having to add this feature in each koffice app.
        QStringList mimeTypes = m_canvas->toolProxy()->supportedPasteMimeTypes();
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

    m_editPaste->setEnabled(paste);
}

void KoPAView::partActivateEvent(KParts::PartActivateEvent* event)
{
    if ( event->widget() == this ) {
        if ( event->activated() ) {
            clipboardDataChanged();
            connect( m_find, SIGNAL( findDocumentSetNext( QTextDocument * ) ), this, SLOT( findDocumentSetNext( QTextDocument * ) ) );
            connect( m_find, SIGNAL( findDocumentSetPrevious( QTextDocument * ) ), this, SLOT( findDocumentSetPrevious( QTextDocument * ) ) );
        }
        else {
            disconnect( m_find, 0, 0, 0 );
        }
    }

    KoView::partActivateEvent(event);
}

void KoPAView::goToPreviousPage()
{
    m_viewMode->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier));
}

void KoPAView::goToNextPage()
{
    m_viewMode->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier));
}

void KoPAView::goToFirstPage()
{
    m_viewMode->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier));
}

void KoPAView::goToLastPage()
{
    m_viewMode->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier));
}

void KoPAView::findDocumentSetNext( QTextDocument * document )
{
    KoPAPageBase * page = 0;
    KoShape * startShape = 0;
    KoTextDocumentLayout *lay = document ? dynamic_cast<KoTextDocumentLayout*> ( document->documentLayout() ) : 0;
    if ( lay != 0 ) {
        startShape = lay->shapes().value( 0 );
        Q_ASSERT( startShape->shapeId() == "TextShapeID" );
        page = m_doc->pageByShape( startShape );
        if ( m_doc->pageIndex( page ) == -1 ) {
            page = 0;
        }
    }

    if ( page == 0 ) {
        page = m_activePage;
        startShape = page;
    }

    KoShape * shape = startShape;

    do {
        // find next text shape
        shape = KoShapeTraversal::nextShape( shape, "TextShapeID" );
        // get next text shape
        if ( shape != 0 ) {
            if ( page != m_activePage ) {
                setActivePage( page );
                m_canvas->update();
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
            if ( m_doc->pageIndex( page ) < m_doc->pages().size() - 1 ) {
                // TODO use also master slides
                page = m_doc->pageByNavigation( page, KoPageApp::PageNext );
            }
            else {
                page = m_doc->pageByNavigation( page, KoPageApp::PageFirst );
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
    KoTextDocumentLayout *lay = document ? dynamic_cast<KoTextDocumentLayout*> ( document->documentLayout() ) : 0;
    if ( lay != 0 ) {
        startShape = lay->shapes().value( 0 );
        Q_ASSERT( startShape->shapeId() == "TextShapeID" );
        page = m_doc->pageByShape( startShape );
        if ( m_doc->pageIndex( page ) == -1 ) {
            page = 0;
        }
    }

    bool check = false;
    if ( page == 0 ) {
        page = m_activePage;
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
            if ( page != m_activePage ) {
                setActivePage( page );
                m_canvas->update();
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
            if ( m_doc->pageIndex( page ) > 0 ) {
                // TODO use also master slides
                page = m_doc->pageByNavigation( page, KoPageApp::PagePrevious );
            }
            else {
                page = m_doc->pageByNavigation( page, KoPageApp::PageLast );
            }
            shape = KoShapeTraversal::last( page );
            check = true;
        }
        // do until you find the same start shape or you are on the same page again only if there was none
    } while ( shape != startShape );
}

void KoPAView::updatePageNavigationActions()
{
    int index = m_doc->pageIndex(activePage());
    int pageCount = m_doc->pages(m_viewMode->masterMode()).count();

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
        QList<KoPAPageBase*> pages = m_doc->pages();
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

#include "KoPAView.moc"
