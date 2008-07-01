/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Clarence Dang <dang@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_view2.h"
#include <qprinter.h>


#include <QGridLayout>
#include <QRect>
#include <QWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QApplication>
#include <QPrintDialog>
#include <QObject>

#include <k3urldrag.h>
#include <kaction.h>
#include <klocale.h>
#include <kmenu.h>
#include <kparts/componentfactory.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kstandardaction.h>
#include <ktogglefullscreenaction.h>
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoSelection.h>
#include <KoToolBoxFactory.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoView.h>
#include <KoToolDockerFactory.h>
#include <KoColorDocker.h>
#include "KoColorSpaceRegistry.h"
#include <KoDockerManager.h>
#include <KoDockRegistry.h>
#include <KoResourceServerProvider.h>

#include <kactioncollection.h>

#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_layer.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_statusbar.h"
#include "canvas/kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_canvas_resource_provider.h"
#include "kis_selection_manager.h"
#include "kis_image_manager.h"
#include "kis_control_frame.h"
#include "kis_birdeye_box.h"
#include "kis_layer_box.h"
#include "kis_layer_manager.h"
#include "kis_zoom_manager.h"
#include "canvas/kis_grid_manager.h"
#include "canvas/kis_perspective_grid_manager.h"
#include "kis_mask_manager.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_group_layer.h"
#include "kis_custom_palette.h"
#include "ui_wdgpalettechooser.h"
#include "kis_resource_server_provider.h"
#include "kis_palette_docker.h"
#include "kis_node_model.h"
#include "kis_projection.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_selection.h"
#include "kis_print_job.h"

class KisView2::KisView2Private {

public:

    KisView2Private()
        : canvas( 0 )
        , doc( 0 )
        , viewConverter( 0 )
        , canvasController( 0 )
        , resourceProvider( 0 )
        , filterManager( 0 )
        , statusBar( 0 )
        , selectionManager( 0 )
        , controlFrame( 0 )
        , birdEyeBox( 0 )
        , layerBox( 0 )
        , nodeManager( 0 )
        , zoomManager( 0 )
        , imageManager( 0 )
        , gridManager( 0 )
        , perspectiveGridManager( 0 )
        {
        }

    ~KisView2Private()
        {
            KoToolManager::instance()->removeCanvasController( canvasController );
            delete canvas;
            delete filterManager;
            delete selectionManager;
            delete nodeManager;
            delete zoomManager;
            delete imageManager;
            delete gridManager;
            delete perspectiveGridManager;
            delete viewConverter;
        }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KoZoomHandler * viewConverter;
    KoCanvasController * canvasController;
    KisCanvasResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KisStatusBar * statusBar;
    QAction * totalRefresh;
    KisSelectionManager *selectionManager;
    KisControlFrame * controlFrame;
    KisBirdEyeBox * birdEyeBox;
    KisLayerBox * layerBox;
    KisNodeManager * nodeManager;
    KisZoomManager * zoomManager;
    KisImageManager * imageManager;
    KisGridManager * gridManager;
    KisPerspectiveGridManager * perspectiveGridManager;
};


KisView2::KisView2(KisDoc2 * doc, QWidget * parent)
    : KoView(doc, parent),
    m_d(new KisView2Private())
{

    m_d->totalRefresh = new KAction(i18n("Total Refresh"), this);
    actionCollection()->addAction("total_refresh", m_d->totalRefresh );
    m_d->totalRefresh->setShortcut(QKeySequence(Qt::CTRL+Qt::SHIFT+Qt::Key_R));
    connect(m_d->totalRefresh, SIGNAL(triggered()), this, SLOT(slotTotalRefresh()));


    // XXX: This state is taken from a kritarc where the docker constellation was configured by hand to look
    //      like the krita 1.6 configuration. Setting this state if none is present seems to work, but there's
    //      still hte versioning problem to be accounted for.
    QString state="AAAA/wAAAAD9AAAAAgAAAAAAAAAzAAACPfwCAAAAAfsAAAAOAFQAbwBvAGwAQgBvAHgBAAAAPwAAAj0AAABGAP///wAAAAEAAADiAAACPfwCAAAADvwAAAA/AAAA9gAAANwBAAAe+gAAAAECAAAAA/sAAAAaAEsAaQBzAEIAaQByAGQAZQB5AGUAQgBvAHgBAAAAAP////8AAAAwAP////sAAAAqAEsAbwBUAG8AbwBsAE8AcAB0AGkAbwBuAHMARABvAGMAawBlAHIAIAA4AQAAAAD/////AAAAvQD////7AAAAJgBLAG8AVABvAG8AbABPAHAAdABpAG8AbgBzAEQAbwBjAGsAZQByAQAAAAD/////AAAAAAAAAAD8AAABJgAAAMcAAAAAAP////r/////AgAAAAP7AAAAGgBLAG8AQwBvAGwAbwByAEQAbwBjAGsAZQByAAAAAAD/////AAAAoAD////7AAAAIABLAGkAcwBQAGEAbABlAHQAdABlAEQAbwBjAGsAZQByAAAAAAD/////AAAAmwD////7AAAAIgBTAHQAcgBvAGsAZQAgAFAAcgBvAHAAZQByAHQAaQBlAHMAAAAAAP////8AAACmAP////wAAAE7AAAAZgAAAGYAAABm+gAAAAEBAAAAAvsAAAAuAEsAbwBTAGgAYQBwAGUAQwBvAGwAbABlAGMAdABpAG8AbgBEAG8AYwBrAGUAcgEAAAAA/////wAAAOIAAADi+wAAACQAUwBtAGEAbABsAEMAbwBsAG8AcgBTAGUAbABlAGMAdABvAHIBAAACPgAAAOIAAABWAP////sAAAAWAEsAaQBzAEwAYQB5AGUAcgBCAG8AeAEAAAGnAAAA1QAAAJAA////+wAAACIAUwBoAGEAZABvAHcAIABQAHIAbwBwAGUAcgB0AGkAZQBzAAAAAAD/////AAAAjgD////7AAAAIABTAGgAYQBwAGUAIABQAHIAbwBwAGUAcgB0AGkAZQBzAAAAAAD/////AAAAFAD////7AAAAJABTAGkAbQBwAGwAZQAgAFQAZQB4AHQAIABFAGQAaQB0AG8AcgAAAAAA/////wAAAM4A////+wAAABIAUwBjAHIAaQBwAHQAaQBuAGcAAAAAAP////8AAAB0AP////sAAAAwAEsAaQBzAFQAcgBpAGEAbgBnAGwAZQBDAG8AbABvAHIAUwBlAGwAZQBjAHQAbwByAAAAAp0AAAB4AAAAeAD////7AAAAKgBTAHAAZQBjAGkAZgBpAGMAQwBvAGwAbwByAFMAZQBsAGUAYwB0AG8AcgAAAAKzAAAA1QAAAI0A////+wAAABoAUwBoAGEAcABlAFMAZQBsAGUAYwB0AG8AcgAAAAAA/////wAAAD4A////+wAAACoASwBvAFQAbwBvAGwATwBwAHQAaQBvAG4AcwBEAG8AYwBrAGUAcgAgADMAAAACYAAAABwAAAAUAP////sAAAAuAEsAaQBzAFAAYQBpAG4AdABlAHIAbAB5AE0AaQB4AGUAcgBEAG8AYwBrAGUAcgAAAAAA/////wAAAHAA////+wAAACwASwBvAFQAbwBvAGwATwBwAHQAaQBvAG4AcwBEAG8AYwBrAGUAcgAgADEAMwAAAAAA/////wAAABQA////AAAB/wAAAj0AAAAEAAAABAAAAAgAAAAI/AAAAAEAAAACAAAAAgAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAAAAACNAAAAAAAAAAAAAAAeAEIAcgB1AHMAaABlAHMAQQBuAGQAUwB0AHUAZgBmAQAAAI0AAAKTAAAAAAAAAAA=";

    KConfigGroup group( KGlobal::config(), "krita" );
    if (!group.hasKey("State")) {
        group.writeEntry("State", state);
    }

    setComponentData(KisFactory2::componentData(), false);

    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    if( mainWindow() )
        actionCollection()->addAction(KStandardAction::KeyBindings, "keybindings", mainWindow()->guiFactory(), SLOT( configureShortcuts() ));

    m_d->doc = doc;
    m_d->viewConverter = new KoZoomHandler();
    m_d->canvasController = new KoCanvasController( this );
    m_d->canvasController->setDrawShadow( false );

    m_d->canvas = new KisCanvas2( m_d->viewConverter, this, doc->shapeController() );
    m_d->canvasController->setCanvas( m_d->canvas );

    m_d->resourceProvider = new KisCanvasResourceProvider( this );
    m_d->resourceProvider->setCanvasResourceProvider( m_d->canvas->resourceProvider() );

    connect( m_d->resourceProvider, SIGNAL( sigDisplayProfileChanged( const KoColorProfile * ) ), m_d->canvas, SLOT(slotSetDisplayProfile( const KoColorProfile * ) ) );

    createManagers();

    createActions();
    createGUI();

    loadPlugins();

    // Wait for the async image to have loaded
    if ( m_d->doc->isLoading() ) {
        connect( m_d->doc, SIGNAL( sigLoadingFinished() ), this, SLOT( slotLoadingFinished() ) );
    }
    else {
        slotLoadingFinished();
    }

    setAcceptDrops(true);
}


KisView2::~KisView2()
{
    delete m_d;
}


void KisView2::dragEnterEvent(QDragEnterEvent *event)
{
    dbgUI <<"KisView2::dragEnterEvent";
    // Only accept drag if we're not busy, particularly as we may
    // be showing a progress bar and calling qApp->processEvents().
    if (K3URLDrag::canDecode(event) && QApplication::overrideCursor() == 0) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisView2::dropEvent(QDropEvent *event)
{
    KUrl::List urls;

    if (K3URLDrag::decode(event, urls))
    {
        if (urls.count() > 0) {

            KMenu popup(this);
            popup.setObjectName("drop_popup");

            QAction *insertAsNewLayer = new QAction(i18n("Insert as New Layer"), &popup);
            QAction *insertAsNewLayers = new QAction(i18n("Insert as New Layers"), &popup);

            QAction *openInNewDocument = new QAction(i18n("Open in New Document"), &popup);
            QAction *openInNewDocuments = new QAction(i18n("Open in New Documents"), &popup);

            QAction *cancel = new QAction(i18n("Cancel"), &popup);

            if (urls.count() == 1) {
                if (!image().isNull()) {
                    popup.addAction(insertAsNewLayer);
                }
                popup.addAction(openInNewDocument);
            }
            else {
                if (!image().isNull()) {
                    popup.addAction(insertAsNewLayers);
                }
                popup.addAction(openInNewDocuments);
            }

            popup.addSeparator();
            popup.addAction(cancel);

            QAction *action = popup.exec(QCursor::pos());

            if (action != 0 && action != cancel) {
                for (KUrl::List::ConstIterator it = urls.begin (); it != urls.end (); ++it) {
                    KUrl url = *it;

                    if (action == insertAsNewLayer || action == insertAsNewLayers) {
                        m_d->imageManager->importImage(url);
                    } else {
                        Q_ASSERT(action == openInNewDocument || action == openInNewDocuments);

                        if (shell() != 0) {
                            shell()->openDocument(url);
                        }
                    }
                }
            }
        }
    }
}


void KisView2::slotChildActivated(bool a) {

    KoView::slotChildActivated(a);
}


void KisView2::canvasAddChild(KoViewChild *child) {
    KoView::canvasAddChild(child);
    connect(this, SIGNAL(viewTransformationsChanged()), child, SLOT(reposition()));
}KisImageSP KisView2::image()
{
    return m_d->doc->image();
}

KisCanvasResourceProvider * KisView2::resourceProvider()
{
    return m_d->resourceProvider;
}

KisCanvas2 * KisView2::canvasBase() const
{
    return m_d->canvas;
}

QWidget* KisView2::canvas() const
{
    return m_d->canvas->canvasWidget();
}

KisStatusBar * KisView2::statusBar() const
{
    return m_d->statusBar;
}

KisSelectionManager * KisView2::selectionManager()
{
    return m_d->selectionManager;
}

KoCanvasController * KisView2::canvasController()
{
    return m_d->canvasController;
}

KisLayerManager * KisView2::layerManager()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->layerManager();
    else
        return 0;
}

KisMaskManager * KisView2::maskManager()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->maskManager();
    else
        return 0;
}

KisNodeSP KisView2::activeNode()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activeNode();
    else
        return 0;
}

KisLayerSP KisView2::activeLayer()
{
    if ( m_d->nodeManager && m_d->nodeManager->layerManager() )
        return m_d->nodeManager->layerManager()->activeLayer();
    else
        return 0;
}

KisPaintDeviceSP KisView2::activeDevice()
{
    if ( m_d->nodeManager )
        return m_d->nodeManager->activePaintDevice();
    else
        return 0;
}

KisZoomManager * KisView2::zoomManager()
{
    return m_d->zoomManager;
}

KisFilterManager * KisView2::filterManager()
{
    return m_d->filterManager;
}

KisImageManager * KisView2::imageManager()
{
    return m_d->imageManager;
}

KisSelectionSP KisView2::selection()
{
    KisLayerSP layer = activeLayer();
    if ( layer )
        return layer->selection(); // falls through to the global
                                   // selection, or 0 in the end
    return 0;
}


KisUndoAdapter * KisView2::undoAdapter()
{
    return m_d->doc->undoAdapter();
}


void KisView2::slotLoadingFinished()
{
    disconnect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));

    KisImageSP img = image();
    slotSetImageSize( img->width(), img->height() );

    if(m_d->statusBar) {
        m_d->statusBar->imageSizeChanged(img->width(), img->height());
    }
    m_d->resourceProvider->slotSetImageSize( img->width(), img->height() );

    m_d->nodeManager->nodesUpdated();

    KoDockerManager *dockerManager = new KoDockerManager(this);
    connect( m_d->canvasController, SIGNAL( toolOptionWidgetsChanged(const QMap<QString, QWidget *> &) ),
             dockerManager, SLOT( newOptionWidgets(const  QMap<QString, QWidget *> &) ) );


    connectCurrentImage();

    if (img->locked()) {
        // If this is the first view on the image, the image will have been locked
        // so unlock it.
        img->blockSignals( false );
        img->unlock();
    }


    if ( KisNodeSP node = img->rootLayer()->firstChild() ) {
        m_d->layerBox->setCurrentNode( node );
        m_d->nodeManager->activateNode( node );
    }

    m_d->zoomManager->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);

    updateGUI();
//     dbgUI <<"image finished loading, active layer:" << img->activeLayer() <<", root layer:" << img->rootLayer();

}


void KisView2::createGUI()
{
    KoToolManager::instance()->addController(m_d->canvasController);
    KoToolManager::instance()->registerTools( actionCollection(), m_d->canvasController );

    // Remove the plugin dock widgets
    QList<QDockWidget*> dockWidgets = shell()->dockWidgets();
    foreach( QDockWidget * dockWidget, dockWidgets ) {
        shell()->removeDockWidget( dockWidget );
    }

    KoToolBoxFactory toolBoxFactory( m_d->canvasController, "Tools" );
    createDockWidget( &toolBoxFactory );

    KoToolDockerFactory toolDockerFactory;
    createDockWidget( &toolDockerFactory );

    KisBirdEyeBoxFactory birdeyeFactory(this);
    m_d->birdEyeBox = qobject_cast<KisBirdEyeBox*>( createDockWidget( &birdeyeFactory ) );


    KisPaletteDockerFactory paletteDockerFactory(this);
    createDockWidget( &paletteDockerFactory );

    KoColorDockerFactory colorDockerFactory;
    createDockWidget( &colorDockerFactory );

    KisLayerBoxFactory layerboxFactory;
    m_d->layerBox = qobject_cast<KisLayerBox*>( createDockWidget( &layerboxFactory ) );

    // Add the plugin dock widgets again
    foreach( QDockWidget * dockWidget, dockWidgets ) {
        shell()->addDockWidget( Qt::RightDockWidgetArea, dockWidget );

    }

    QDockWidget * birdEyeBox = qobject_cast<QDockWidget*>( shell()->findChild<QDockWidget*>( "KisBirdeyeBox" ));
    QDockWidget * toolBox = qobject_cast<QDockWidget*>( shell()->findChild<QDockWidget*>("KoToolOptionsDocker" ));
    QDockWidget * colorDocker = qobject_cast<QDockWidget*>( shell()->findChild<QDockWidget*>("KoColorDocker"));
    QDockWidget * paletteDocker = qobject_cast<QDockWidget*>( shell()->findChild<QDockWidget*>("KisPaletteDocker"));
    QDockWidget * strokeDocker = qobject_cast<QDockWidget*>( shell()->findChild<QDockWidget*>("Stroke Properties"));

    if (birdEyeBox != 0 && toolBox != 0) {
        shell()->tabifyDockWidget(birdEyeBox, toolBox);
    }

    if (colorDocker != 0 && paletteDocker != 0 && strokeDocker != 0) {
        shell()->tabifyDockWidget(colorDocker, paletteDocker);
        shell()->tabifyDockWidget(paletteDocker, strokeDocker);
    }
    colorDocker->show();

    KConfigGroup group( KGlobal::config(), "GUI" );
    QFont dockWidgetFont  = KGlobalSettings::generalFont();
    double pointSize = group.readEntry("palettefontsize", dockWidgetFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    dockWidgetFont.setPointSizeF(pointSize);

    foreach( QObject * object, shell()->children() )
    {
        if (object->inherits("QTabBar")) {
            QTabBar * tabBar = qobject_cast<QTabBar*>(object);
            tabBar->setFont( dockWidgetFont );
        }
    }

    m_d->statusBar = KoView::statusBar() ? new KisStatusBar( KoView::statusBar(), this ) : 0;
    connect(m_d->canvasController, SIGNAL( documentMousePositionChanged(const QPointF & )),
            m_d->statusBar, SLOT( documentMousePositionChanged( const QPointF & ) ) );

    m_d->controlFrame = new KisControlFrame( this );

    show();

    connect(m_d->layerBox, SIGNAL(sigRequestNewNode( const QString &)),
            m_d->nodeManager, SLOT(createNode( const QString &)));

    connect(m_d->layerBox, SIGNAL(sigRequestNodeProperties(KisNodeSP)),
            m_d->nodeManager, SLOT(nodeProperties(KisNodeSP)));

    connect(m_d->layerBox, SIGNAL(sigOpacityChanged(double, bool)),
            m_d->nodeManager, SLOT(nodeOpacityChanged(double, bool)));

    connect(m_d->layerBox, SIGNAL(sigItemComposite(const KoCompositeOp*)),
            m_d->nodeManager, SLOT(nodeCompositeOpChanged(const KoCompositeOp*)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->resourceProvider, SLOT(slotNodeActivated(KisNodeSP)));
}


void KisView2::createActions()
{
    actionCollection()->addAction(KStandardAction::FullScreen, "full_screen", this, SLOT( slotUpdateFullScreen( bool )));
    actionCollection()->addAction(KStandardAction::Preferences,  "preferences", this, SLOT(slotPreferences()));

    KAction* action = new KAction(i18n("Edit Palette..."), this);
    actionCollection()->addAction("edit_palette", action );
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditPalette()));
}


void KisView2::createManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
    // managers

    m_d->filterManager = new KisFilterManager(this, m_d->doc);
    m_d->filterManager->setup(actionCollection());

    m_d->selectionManager = new KisSelectionManager( this, m_d->doc );
    m_d->selectionManager->setup( actionCollection() );

    m_d->nodeManager = new KisNodeManager( this, m_d->doc );
    m_d->nodeManager->setup( actionCollection() );
    connect(m_d->doc->nodeModel(), SIGNAL(requestAddNode(KisNodeSP, KisNodeSP)), m_d->nodeManager, SLOT(addNode(KisNodeSP, KisNodeSP)));
    connect(m_d->doc->nodeModel(), SIGNAL(requestMoveNode(KisNodeSP, KisNodeSP)), m_d->nodeManager, SLOT(moveNode(KisNodeSP, KisNodeSP)));

    // the following cast is not really safe, but better here than in the zoomManager
    // best place would be outside kisview too
    m_d->zoomManager = new KisZoomManager( this, m_d->viewConverter, m_d->canvasController);
    m_d->zoomManager->setup( actionCollection() );

    m_d->imageManager = new KisImageManager( this );
    m_d->imageManager->setup( actionCollection() );

    m_d->gridManager = new KisGridManager( this );
    m_d->gridManager->setup( actionCollection() );
    m_d->canvas->addDecoration( m_d->gridManager );

    m_d->perspectiveGridManager = new KisPerspectiveGridManager( this );
    m_d->perspectiveGridManager->setup( actionCollection() );
    m_d->canvas->addDecoration( m_d->perspectiveGridManager );

}

void KisView2::updateGUI()
{

    m_d->nodeManager->updateGUI();

    m_d->selectionManager->updateGUI();
    m_d->filterManager->updateGUI();
    m_d->zoomManager->updateGUI();
    m_d->imageManager->updateGUI();

    m_d->gridManager->updateGUI();
    m_d->perspectiveGridManager->updateGUI();
    m_d->layerBox->updateUI();
}


void KisView2::connectCurrentImage()
{
    KisImageSP img = image();
    if (img) {
//         dbgUI <<"Going to connect current image";

        if( m_d->statusBar ) {
            connect(img.data(), SIGNAL(sigColorSpaceChanged(const KoColorSpace *)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
            connect(img.data(), SIGNAL(sigProfileChanged(KoColorProfile * )), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
            connect(img.data(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->statusBar, SLOT(imageSizeChanged(qint32, qint32)));

        }
        connect(img.data(), SIGNAL( sigSizeChanged( qint32, qint32 ) ), m_d->resourceProvider, SLOT( slotSetImageSize( qint32, qint32 ) ) );
        connect(img.data(), SIGNAL( sigSizeChanged( qint32, qint32 ) ), this, SLOT( slotSetImageSize( qint32, qint32 ) ) );
        connect(img->undoAdapter(), SIGNAL( selectionChanged() ), selectionManager(), SLOT( selectionChanged() ) );

#if 0 // XXX_NODE
        connect(img.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), m_d->layerManager, SLOT(layersUpdated()));

        connect( m_d->layerManager, SIGNAL( sigLayerActivated( KisLayerSP ) ),
                 m_d->resourceProvider, SLOT( slotLayerActivated( const KisLayerSP ) ) );

        // Temporary forwarding of signals until these deprecated
        // signals are gone from KisImage
        connect( img.data(), SIGNAL( sigColorSpaceChanged(const KoColorSpace * ) ),
                 m_d->layerManager, SIGNAL( currentColorSpaceChanged( const KoColorSpace * ) ) );

        connect(m_d->layerManager, SIGNAL(sigLayerActivated(KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(m_d->layerManager, SIGNAL(sigLayerActivated(KisLayerSP)), m_d->canvas, SLOT(updateCanvas()));
#endif
    }

    m_d->canvas->connectCurrentImage();

    if( m_d->layerBox ) {
        m_d->layerBox->setImage( m_d->nodeManager, img, m_d->doc->nodeModel() );
        connect( m_d->doc->nodeModel(), SIGNAL( nodeActivated( KisNodeSP ) ),
                 m_d->nodeManager, SLOT( activateNode( KisNodeSP ) ) );
    }

    if( m_d->birdEyeBox )
        m_d->birdEyeBox->setImage( img );

}

void KisView2::disconnectCurrentImage()
{
    KisImageSP img = image();

    if (img) {

        img->disconnect(this);
        img->disconnect( m_d->nodeManager );
        img->disconnect( m_d->selectionManager );
        if( m_d->statusBar )
            img->disconnect( m_d->statusBar );

        if( m_d->layerBox )
            m_d->layerBox->setImage(0, KisImageSP(0), 0);
        if( m_d->birdEyeBox )
            m_d->birdEyeBox->setImage(KisImageSP(0));
        m_d->canvas->disconnectCurrentImage();
    }
}

#if 0
void KisView2::setupPrinter(QPrinter &printer, QPrintDialog &printDialog)
{
    Q_UNUSED(printer);
    Q_UNUSED(printDialog);
    // XXX: implement printing
    //p.setMinMax(1, 1);
}

void KisView2::print(QPrinter& printer, QPrintDialog &printDialog)
{
    Q_UNUSED( printDialog );
    QPainter gc(&printer);

    KisImageSP img = image();
    if (!img) return;

    gc.setClipping(false);

    KisConfig cfg;
    QString printerProfileName = cfg.printerProfile();
    const KoColorProfile *printerProfile = KoColorSpaceRegistry::instance()->profileByName(printerProfileName);

    double exposure = m_d->canvas->resourceProvider()->resource(KisCanvasResourceProvider::HdrExposure ).toDouble();

    double scaleX = printer.resolution() / (72.0 * img->xRes());
    double scaleY = printer.resolution() / (72.0 * img->yRes());

    QRect r = img->bounds();

    gc.scale(scaleX, scaleY);
    img->renderToPainter(0, 0, r.x(), r.y(), r.width(), r.height(), gc, printerProfile, exposure);
}
#endif

void KisView2::slotUpdateFullScreen(bool toggle)
{
    if (KoView::shell()) {

        Qt::WindowStates newState = KoView::shell()->windowState();

        if (toggle) {
            newState |= Qt::WindowFullScreen;
        } else {
            newState &= ~Qt::WindowFullScreen;
        }

        KoView::shell()->setWindowState(newState);
    }
}

void KisView2::slotPreferences()
{
    if ( PreferencesDialog::editPreferences() ) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        m_d->resourceProvider->resetDisplayProfile();

        // Update the settings for all nodes -- they don't query
        // KisConfig directly because they need the settings during
        // compositing, and they don't connect to the confignotifier
        // because nodes are not QObjects (because only one base class
        // can be a QObject).
        KisNode* node = dynamic_cast<KisNode*>( image()->rootLayer().data() );
        node->updateSettings();
    }
}

void KisView2::slotEditPalette()
{
    QList<KoColorSet*> palettes = KoResourceServerProvider::instance()->paletteServer()->resources();

    KDialog* base = new KDialog(this );
    base->setCaption(  i18n("Edit Palette") );
    base->setButtons( KDialog::Ok);
    base->setDefaultButton( KDialog::Ok);
    KisCustomPalette* cp = new KisCustomPalette(palettes, base, "edit palette", i18n("Edit Palette"), this);
    base->setMainWidget(cp);
    base->show();
}

void KisView2::slotSetImageSize( qint32 w, qint32 h )
{
    m_d->zoomManager->zoomController()->setPageSize(QSizeF(image()->width() / image()->xRes(), image()->height() / image()->yRes() ));
    m_d->zoomManager->zoomController()->setDocumentSize(QSizeF(image()->width() / image()->xRes(), image()->height() / image()->yRes() ));
    m_d->canvasController->setDocumentSize( QSize( int( ceil( m_d->viewConverter->documentToViewX( w / image()->xRes() ) ) ),
                                                   int( ceil( m_d->viewConverter->documentToViewY( h / image()->yRes() ) ) ) ) );
    m_d->zoomManager->updateGUI();
}


void KisView2::loadPlugins()
{
    // Load all plugins
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/ViewPlugin"),
                                                              QString::fromLatin1("(Type == 'Service') and "
                                                                                  "([X-Krita-Version] == 3)"));
    KService::List::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {

        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
            KService::createInstance<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin ) {
            insertChildClient(plugin);
        }
        else {
            if( errCode == KLibLoader::ErrNoLibrary)
            {
                kWarning() <<" Error loading plugin was : ErrNoLibrary" << KLibLoader::self()->lastErrorMessage();
            }
        }
    }
}

KisDoc2 * KisView2::document() const
{
    return m_d->doc;
}

KoPrintJob * KisView2::createPrintJob()
{
    return new KisPrintJob(this);
}

KisNodeManager * KisView2::nodeManager()
{
    return m_d->nodeManager;
}

KisPerspectiveGridManager* KisView2::perspectiveGridManager()
{
    return m_d->perspectiveGridManager;
}

KisGridManager * KisView2::gridManager()
{
    return m_d->gridManager;
}

void KisView2::slotTotalRefresh()
{
    m_d->canvas->resetCanvas();
}

#include "kis_view2.moc"
