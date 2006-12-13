/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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

#include <QGridLayout>
#include <QRect>
#include <QWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QApplication>

#include <k3urldrag.h>
#include <kaction.h>
#include <klocale.h>
#include <kmenu.h>
#include <kparts/componentfactory.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstdaction.h>
#include <kstdaction.h>
#include <ktogglefullscreenaction.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <kxmlguifactory.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoSelection.h>
#include <KoToolBoxFactory.h>
#include <KoShapeSelectorFactory.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoView.h>
#include <KoToolDockerFactory.h>

#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_layer.h>

#include "kis_config.h"
#include "kis_statusbar.h"
#include "kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_opengl_canvas2.h"
#include "kis_qpainter_canvas.h"
#include "kis_resource_provider.h"
#include "kis_resource_provider.h"
#include "kis_selection_manager.h"
#include "kis_image_manager.h"
#include "kis_controlframe.h"
#include "kis_birdeye_box.h"
#include "kis_layerbox.h"
#include "kis_layer_manager.h"
#include "kis_zoom_manager.h"
#include "kis_grid_manager.h"
#include "kis_perspective_grid_manager.h"
#include "kis_mask_manager.h"
#include "kis_dlg_preferences.h"
#include "kis_group_layer.h"

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
        , layerManager( 0 )
        , zoomManager( 0 )
        , imageManager( 0 )
        , maskManager( 0 )
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
            delete layerManager;
            delete zoomManager;
            delete imageManager;
            delete maskManager;
            delete gridManager;
            delete perspectiveGridManager;
        }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KoViewConverter * viewConverter;
    KoCanvasController * canvasController;
    KisResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KisStatusBar * statusBar;
    KAction * fullScreen;
    KisSelectionManager *selectionManager;
    KisControlFrame * controlFrame;
    KisBirdEyeBox * birdEyeBox;
    KisLayerBox * layerBox;
    KisLayerManager * layerManager;
    KisZoomManager * zoomManager;
    KisImageManager * imageManager;
    KisMaskManager * maskManager;
    KisGridManager * gridManager;
    KisPerspectiveGridManager * perspectiveGridManager;
};


KisView2::KisView2(KisDoc2 * doc, KoViewConverter * viewConverter, QWidget * parent)
    : KoView(doc, parent)
{

    setInstance(KisFactory2::instance(), false);

    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    KStdAction::keyBindings( mainWindow()->guiFactory(),
                             SLOT( configureShortcuts() ),
                             actionCollection() );

    m_d = new KisView2Private();

    m_d->doc = doc;
    m_d->viewConverter = viewConverter;
    m_d->canvas = new KisCanvas2( m_d->viewConverter, QPAINTER, this, static_cast<KoShapeControllerBase*>( doc ) );
    m_d->canvasController = new KoCanvasController( this );

    m_d->canvasController->setCanvas( m_d->canvas );
    m_d->resourceProvider = new KisResourceProvider( this );

    createActions();
    createManagers();
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

            KAction insertAsNewLayer(i18n("Insert as New Layer"), 0, "insert_as_new_layer");
            KAction insertAsNewLayers(i18n("Insert as New Layers"), 0, "insert_as_new_layers");

            KAction openInNewDocument(i18n("Open in New Document"), 0, "open_in_new_document");
            KAction openInNewDocuments(i18n("Open in New Documents"), 0, "open_in_new_documents");

            KAction cancel(i18n("Cancel"), 0, "cancel");

            if (urls.count() == 1) {
                if (!image().isNull()) {
                    popup.addAction(&insertAsNewLayer);
                }
                popup.addAction(&openInNewDocument);
            }
            else {
                if (!image().isNull()) {
                    popup.addAction(&insertAsNewLayers);
                }
                popup.addAction(&openInNewDocuments);
            }

            (void)popup.addSeparator();
            popup.addAction(&cancel);

            QAction *action = popup.exec(QCursor::pos());

            if (action != 0 && action != &cancel) {
                for (KUrl::List::ConstIterator it = urls.begin (); it != urls.end (); ++it) {
                    KUrl url = *it;

                    if (action == &insertAsNewLayer || action == &insertAsNewLayers) {
                        m_d->imageManager->importImage(url);
                    } else {
                        Q_ASSERT(action == &openInNewDocument || action == &openInNewDocuments);

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

    // It should be so that the only part (child) we can activate, is
    // the current layer:
    KisImageSP img = image();
    if ( img && img->activeLayer())
    {
        if (a) {
            img->activeLayer()->activate();
        } else {
            img->activeLayer()->deactivate();
        }
    }

    KoView::slotChildActivated(a);
}


void KisView2::canvasAddChild(KoViewChild *child) {
    KoView::canvasAddChild(child);
    connect(this, SIGNAL(viewTransformationsChanged()), child, SLOT(reposition()));
}



KisImageSP KisView2::image()
{
    return m_d->doc->currentImage();
}

KisResourceProvider * KisView2::resourceProvider()
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

KisLayerManager * KisView2::layerManager()
{
    return m_d->layerManager;
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

KisUndoAdapter * KisView2::undoAdapter()
{
    return m_d->doc->undoAdapter();
}


void KisView2::slotLoadingFinished()
{
    disconnect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));

    m_d->canvas->setCanvasSize( image()->width(), image()->height() );

    m_d->layerManager->layersUpdated();
    updateGUI();

    KoToolDockerFactory toolDockerFactory;
    KoToolDocker * d =  dynamic_cast<KoToolDocker*>( createDockWidget( &toolDockerFactory ) );
    if(d)
        m_d->canvasController->setToolOptionDocker( d );
    else
        kDebug() << "Could not create tool docker: " << d << endl;

    connectCurrentImage();
    KisImageSP img = image();
    KisGroupLayerSP rootLayer = img ->rootLayer();
    KisLayerSP activeLayer = rootLayer->firstChild();
    kDebug() << "image finished loading, active layer: " << activeLayer << ", root layer: " << rootLayer << endl;
    if ( activeLayer )
        img->activate( activeLayer );
}


void KisView2::createGUI()
{

    KoToolBoxFactory toolBoxFactory( "Krita" );
    createDockWidget( &toolBoxFactory );

    KoShapeSelectorFactory shapeSelectorFactory;
    createDockWidget( &shapeSelectorFactory );

    KoToolManager::instance()->addControllers(m_d->canvasController);

    KisBirdEyeBoxFactory birdeyeFactory(this);
    m_d->birdEyeBox = qobject_cast<KisBirdEyeBox*>( createDockWidget( &birdeyeFactory ) );

    KisLayerBoxFactory layerboxFactory( this );
    m_d->layerBox = qobject_cast<KisLayerBox*>( createDockWidget( &layerboxFactory ) );

    m_d->statusBar = new KisStatusBar( KoView::statusBar(), this );
    m_d->controlFrame = new KisControlFrame( mainWindow(), this );

    show();
}


void KisView2::createActions()
{
    m_d->fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this );
    connect( m_d->fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));

    KStdAction::preferences(this, SLOT(slotPreferences()), actionCollection(), "preferences");


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

    m_d->layerManager = new KisLayerManager( this, m_d->doc );
    m_d->layerManager->setup( actionCollection() );

    m_d->zoomManager = new KisZoomManager( this, m_d->viewConverter, m_d->canvasController);
    m_d->zoomManager->setup( actionCollection() );

    m_d->imageManager = new KisImageManager( this );
    m_d->imageManager->setup( actionCollection() );

    m_d->maskManager = new KisMaskManager( this );
    m_d->maskManager->setup( actionCollection() );

    m_d->gridManager = new KisGridManager( this );
    m_d->gridManager->setup( actionCollection() );

    m_d->perspectiveGridManager = new KisPerspectiveGridManager( this );
    m_d->perspectiveGridManager->setup( actionCollection() );

}

void KisView2::updateGUI()
{

    m_d->layerManager->updateGUI();
    m_d->selectionManager->updateGUI();
    m_d->filterManager->updateGUI();
    m_d->zoomManager->updateGUI();
    m_d->imageManager->updateGUI();
    m_d->maskManager->updateGUI();
    m_d->gridManager->updateGUI();
    m_d->perspectiveGridManager->updateGUI();

}


void KisView2::connectCurrentImage()
{
    KisImageSP img = image();
    if (img) {
        connect(img.data(), SIGNAL(sigActiveSelectionChanged(KisImageSP)), m_d->selectionManager, SLOT(imgSelectionChanged(KisImageSP)));
        //connect(img.data(), SIGNAL(sigActiveSelectionChanged(KisImageSP)), this, SLOT(updateCanvas()));
        connect(img.data(), SIGNAL(sigColorSpaceChanged(KoColorSpace *)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
        connect(img.data(), SIGNAL(sigProfileChanged(KoColorProfile * )), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));

        connect(img.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(img.data(), SIGNAL(sigMaskInfoChanged()), m_d->maskManager, SLOT(maskUpdated()));
        connect(img.data(), SIGNAL(sigLayerAdded(KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(img.data(), SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(img.data(), SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(img.data(), SIGNAL(sigLayerActivated(KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));
        connect(img.data(), SIGNAL(sigLayerActivated(KisLayerSP)), m_d->canvas, SLOT(updateCanvas()));
        connect(img.data(), SIGNAL(sigLayerPropertiesChanged(KisLayerSP)), m_d->layerManager, SLOT(layersUpdated()));

#if 0 // XXX: What about parts
        KisConnectPartLayerVisitor v(img, this, true);
        img->rootLayer()->accept(v);
        connect(img.data(), SIGNAL(sigLayerAdded(KisLayerSP)),
                SLOT(handlePartLayerAdded(KisLayerSP)));
#endif
        m_d->maskManager->maskUpdated();
#if 0
#ifdef HAVE_OPENGL
        if (!m_OpenGLImageContext.isNull()) {
            connect(m_OpenGLImageContext.data(), SIGNAL(sigImageUpdated(QRect)), SLOT(slotOpenGLImageUpdated(QRect)));
            connect(m_OpenGLImageContext.data(), SIGNAL(sigSizeChanged(qint32, qint32)), SLOT(slotImageSizeChanged(qint32, qint32)));
        } else
#endif
#endif
        {
            connect(img.data(), SIGNAL(sigImageUpdated(QRect)), m_d->canvas, SLOT(updateCanvas(QRect)));
            connect(img.data(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->canvas, SLOT(updateCanvas( )));
        }

        connect( m_d->doc, SIGNAL( sigCommandExecuted() ), img.data(), SLOT( slotCommandExecuted() ) );
    }

    m_d->layerBox->setImage(img);
    m_d->birdEyeBox->setImage(img);

}

void KisView2::disconnectCurrentImage()
{
    KisImageSP img = image();

    if (img) {

        img->disconnect(this);
        img->disconnect( m_d->layerManager );
        img->disconnect( m_d->canvas );
        img->disconnect( m_d->selectionManager );
        img->disconnect( m_d->statusBar );

        disconnect( m_d->doc, SIGNAL( sigCommandExecuted() ), img.data(), SLOT( slotCommandExecuted() ) );

        m_d->layerBox->setImage(KisImageSP(0));
        m_d->birdEyeBox->setImage(KisImageSP(0));

#if 0 // XXX: What about parts?
        KisConnectPartLayerVisitor v(img, this, false);
        img->rootLayer()->accept(v);

#endif
    }
#if 0
#ifdef HAVE_OPENGL
    if (!m_OpenGLImageContext.isNull()) {
        m_OpenGLImageContext->disconnect(this);
    }
#endif
#endif
}

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
#if 0
#ifdef HAVE_OPENGL
    bool canvasWasOpenGL = m_canvas->isOpenGLCanvas();
#endif
#endif
    if (PreferencesDialog::editPreferences())
    {
        KisConfig cfg;
        m_d->canvas->resetMonitorProfile();
#if 0
#ifdef HAVE_OPENGL
        if (cfg.useOpenGL() != canvasWasOpenGL) {

            disconnectCurrentImg();

            //XXX: Need to notify other views that this global setting has changed.
            if (cfg.useOpenGL()) {
                m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(m_image, monitorProfile());
                m_canvas->createOpenGLCanvas(m_OpenGLImageContext->sharedContextWidget());
            } else
            {
                m_OpenGLImageContext = 0;
                m_canvas->createQPaintDeviceCanvas();
            }

            connectCurrentImg();

            resizeEvent(0);
        }

        if (cfg.useOpenGL()) {
            m_OpenGLImageContext->setMonitorProfile(monitorProfile());
        }
#endif
#endif
        m_d->canvas->canvasWidget()->update();

#if 0 // XXX: How to do this with KoToolManager?
        if (m_toolManager->currentTool()) {
            setCanvasCursor(m_toolManager->currentTool()->cursor());
        }
#endif

    }
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
            kDebug(41006) << "found plugin " << service->property("Name").toString() << "\n";
            insertChildClient(plugin);
        }
        else {
            kDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KLibLoader::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }
    }
}

KisDoc2 * KisView2::document() const
{
    return m_d->doc;
}

#include "kis_view2.moc"
