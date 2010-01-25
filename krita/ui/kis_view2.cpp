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

#include <stdio.h>

#include "kis_view2.h"
#include <qprinter.h>

#include <QDesktopWidget>
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
#include <ktogglefullscreenaction.h>
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoSelection.h>
#include <KoToolBoxFactory.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoView.h>
#include "KoColorSpaceRegistry.h"
#include <KoDockerManager.h>
#include <KoDockRegistry.h>
#include <KoResourceServerProvider.h>
#include <KoCompositeOp.h>

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
#include "kis_selection_manager.h"
#include "kis_image_manager.h"
#include "kis_control_frame.h"
#include "kis_paintop_box.h"
#include "kis_layer_manager.h"
#include "kis_zoom_manager.h"
#include "canvas/kis_grid_manager.h"
#include "canvas/kis_perspective_grid_manager.h"
#include "kis_mask_manager.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_group_layer.h"
#include "kis_custom_palette.h"
#include "kis_resource_server_provider.h"
#include "kis_node_model.h"
#include "kis_projection.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_selection.h"
#include "kis_print_job.h"
#include "kis_painting_assistants_manager.h"
#include <kis_paint_layer.h>
#include "kis_progress_widget.h"

#include <QDebug>
#include <QPoint>
#include "kis_paintop_box.h"
#include "kis_node_commands_adapter.h"
#include <kis_paintop_preset.h>
#include "ko_favorite_resource_manager.h"
#include "kis_paintop_box.h"

class KisView2::KisView2Private
{

public:

    KisView2Private()
        : canvas(0)
        , doc(0)
        , viewConverter(0)
        , canvasController(0)
        , resourceProvider(0)
        , filterManager(0)
        , statusBar(0)
        , selectionManager(0)
        , controlFrame(0)
        , nodeManager(0)
        , zoomManager(0)
        , imageManager(0)
        , gridManager(0)
        , perspectiveGridManager(0)
        , paintingAssistantManager(0)
        , favoriteResourceManager(0) {

    }

    ~KisView2Private() {
        if (canvasController) {
            KoToolManager::instance()->removeCanvasController(canvasController);
        }
        delete canvas;
        delete filterManager;
        delete selectionManager;
        delete nodeManager;
        delete zoomManager;
        delete imageManager;
        delete gridManager;
        delete perspectiveGridManager;
        delete paintingAssistantManager;
        delete viewConverter;
        delete favoriteResourceManager;
    }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KoZoomHandler * viewConverter;
    KoCanvasController * canvasController;
    KisCanvasResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KisStatusBar * statusBar;
    KAction * totalRefresh;
    KAction* toggleDockers;
    KisSelectionManager *selectionManager;
    KisControlFrame * controlFrame;
    KisNodeManager * nodeManager;
    KisZoomManager * zoomManager;
    KisImageManager * imageManager;
    KisGridManager * gridManager;
    KisPerspectiveGridManager * perspectiveGridManager;
    KisPaintingAssistantsManager* paintingAssistantManager;
    KoFavoriteResourceManager* favoriteResourceManager;
    QVector<QDockWidget*> hiddenDockwidgets;
};


KisView2::KisView2(KisDoc2 * doc, QWidget * parent)
    : KoView(doc, parent),
    m_d(new KisView2Private())
{

    setFocusPolicy(Qt::NoFocus);

    m_d->totalRefresh = new KAction(i18n("Total Refresh"), this);
    actionCollection()->addAction("total_refresh", m_d->totalRefresh);
    m_d->totalRefresh->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
    connect(m_d->totalRefresh, SIGNAL(triggered()), this, SLOT(slotTotalRefresh()));

    m_d->toggleDockers = new KToggleAction(i18n("Show Dockers"), this);
    m_d->toggleDockers->setChecked(true);
    actionCollection()->addAction("toggledockers", m_d->toggleDockers);


    m_d->toggleDockers->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    connect(m_d->toggleDockers, SIGNAL(toggled(bool)), this, SLOT(toggleDockers(bool)));

    setComponentData(KisFactory2::componentData(), false);

    if (!doc->isReadWrite()) {
        setXMLFile("krita_readonly.rc");
    } else {
        setXMLFile("krita.rc");
    }

    if (mainWindow()) {
        actionCollection()->addAction(KStandardAction::KeyBindings, "keybindings", mainWindow()->guiFactory(), SLOT(configureShortcuts()));
    }

    m_d->doc = doc;
    m_d->viewConverter = new KoZoomHandler();

    m_d->canvasController = new KoCanvasController(this);
    m_d->canvasController->setDrawShadow(false);
    m_d->canvasController->setMargin(10);
    m_d->canvasController->setCanvasMode(KoCanvasController::Infinite);

    m_d->canvas = new KisCanvas2(m_d->viewConverter, this, doc->shapeController());
    m_d->canvasController->setCanvas(m_d->canvas);

    m_d->resourceProvider = new KisCanvasResourceProvider(this);
    m_d->resourceProvider->setResourceManager(m_d->canvas->resourceManager());

    Q_ASSERT(m_d->canvasController);
    KoToolManager::instance()->addController(m_d->canvasController);
    KoToolManager::instance()->registerTools(actionCollection(), m_d->canvasController);


    connect(m_d->resourceProvider, SIGNAL(sigDisplayProfileChanged(const KoColorProfile *)), m_d->canvas, SLOT(slotSetDisplayProfile(const KoColorProfile *)));

    createManagers();
    createActions();


    KoToolBoxFactory toolBoxFactory(m_d->canvasController, i18n("Tools"));
    createDockWidget(&toolBoxFactory);

    KoDockerManager *dockerMng = dockerManager();
    if (!dockerMng) {
        dockerMng = new KoDockerManager(this);
        setDockerManager(dockerMng);
    }

    connect(m_d->canvasController, SIGNAL(toolOptionWidgetsChanged(const QMap<QString, QWidget *> &, QWidget*)),
            dockerMng, SLOT(newOptionWidgets(const  QMap<QString, QWidget *> &, QWidget*)));

    m_d->statusBar = new KisStatusBar(this);
    connect(m_d->canvasController, SIGNAL(documentMousePositionChanged(const QPointF &)),
            m_d->statusBar, SLOT(documentMousePositionChanged(const QPointF &)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->resourceProvider, SLOT(slotNodeActivated(KisNodeSP)));

    m_d->controlFrame = new KisControlFrame(this);

    connect(layerManager(), SIGNAL(currentColorSpaceChanged(const KoColorSpace*)),
            m_d->controlFrame->paintopBox(), SLOT(colorSpaceChanged(const KoColorSpace*)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->controlFrame->paintopBox(), SLOT(slotCurrentNodeChanged(KisNodeSP)));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(const KoInputDevice &)),
            m_d->controlFrame->paintopBox(), SLOT(slotInputDeviceChanged(const KoInputDevice &)));

    show();


    loadPlugins();

    // Wait for the async image to have loaded
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    if (!m_d->doc->isLoading() || m_d->doc->image()) {
        slotLoadingFinished();
    }

    // canvas sends signal that origin is changed
    connect(m_d->canvas, SIGNAL(documentOriginChanged()), m_d->zoomManager, SLOT(pageOffsetChanged()));

    setAcceptDrops(true);
}


KisView2::~KisView2()
{
    delete m_d;
}


void KisView2::dragEnterEvent(QDragEnterEvent *event)
{
    dbgUI << "KisView2::dragEnterEvent";
    // Only accept drag if we're not busy, particularly as we may
    // be showing a progress bar and calling qApp->processEvents().
    if (K3URLDrag::canDecode(event) && QApplication::overrideCursor() == 0) {
        event->accept();
    } else if (event->mimeData()->hasImage()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisView2::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasImage()) {
        QImage qimage = qvariant_cast<QImage>(event->mimeData()->imageData());
        KisImageWSP kisimage = image();

        if (kisimage) {
            KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
            device->convertFromQImage(qimage, "");
            KisLayerSP layer = new KisPaintLayer(kisimage.data(), kisimage->nextLayerName(), OPACITY_OPAQUE, device);

            QPointF pos = kisimage->documentToIntPixel(m_d->viewConverter->viewToDocument(event->pos() + m_d->canvas->documentOffset() - m_d->canvas->documentOrigin()));
            layer->setX(pos.x());
            layer->setY(pos.y());

            if (layer) {
                KisNodeCommandsAdapter adapter(this);
                if (!m_d->nodeManager->layerManager()->activeLayer()) {
                    adapter.addNode(layer.data(), kisimage->rootLayer().data() , 0);
                } else {
                    adapter.addNode(layer.data(), m_d->nodeManager->layerManager()->activeLayer()->parent().data(), m_d->nodeManager->layerManager()->activeLayer().data());
                }
                layer->setDirty();
                canvas()->update();
                nodeManager()->activateNode(layer);
            }
        }
        return;
    }

    KUrl::List urls;
    if (K3URLDrag::decode(event, urls)) {
        if (urls.count() > 0) {

            KMenu popup;
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
            } else {
                if (!image().isNull()) {
                    popup.addAction(insertAsNewLayers);
                }
                popup.addAction(openInNewDocuments);
            }

            popup.addSeparator();
            popup.addAction(cancel);

            QAction *action = popup.exec(QCursor::pos());

            if (action != 0 && action != cancel) {
                for (KUrl::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it) {
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

KoZoomController *KisView2::zoomController() const
{
    return m_d->zoomManager->zoomController();
}

KisImageWSP KisView2::image()
{
    if (m_d && m_d->doc) {
        return m_d->doc->image();
    }
    return 0;
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

KoProgressUpdater* KisView2::createProgressUpdater(KoProgressUpdater::Mode mode)
{
    return m_d->statusBar->progress()->createUpdater(mode);
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
    if (m_d->nodeManager && m_d->nodeManager->layerManager())
        return m_d->nodeManager->layerManager()->activeLayer();
    else
        return 0;
}

KisPaintDeviceSP KisView2::activeDevice()
{
    if (m_d->nodeManager)
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
    if (layer)
        return layer->selection(); // falls through to the global
    // selection, or 0 in the end
    return image()->globalSelection();
}


KisUndoAdapter * KisView2::undoAdapter()
{
    return m_d->doc->undoAdapter();
}


void KisView2::slotLoadingFinished()
{
    image()->refreshGraph();

    slotImageSizeChanged();

    if (m_d->statusBar) {
        m_d->statusBar->imageSizeChanged(image()->width(), image()->height());
    }
    m_d->resourceProvider->slotImageSizeChanged();

    m_d->nodeManager->nodesUpdated();

    connectCurrentImage();

    if (image()->locked()) {
        // If this is the first view on the image, the image will have been locked
        // so unlock it.
        image()->blockSignals(false);
        image()->unlock();
    }

    if (KisNodeSP node = image()->rootLayer()->firstChild()) {
        m_d->nodeManager->activateNode(node);
    }

    /**
     * Dirty hack alert
     */
    m_d->viewConverter->setZoomMode(KoZoomMode::ZOOM_PAGE);
    m_d->zoomManager->zoomController()->setAspectMode(true);

    updateGUI();

    emit sigLoadingFinished();
}


void KisView2::createActions()
{
    actionCollection()->addAction(KStandardAction::FullScreen, "full_screen", this, SLOT(slotUpdateFullScreen(bool)));
    actionCollection()->addAction(KStandardAction::Preferences,  "preferences", this, SLOT(slotPreferences()));

    KAction* action = new KAction(i18n("Edit Palette..."), this);
    actionCollection()->addAction("edit_palette", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditPalette()));
}


void KisView2::createManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
    // managers

    m_d->filterManager = new KisFilterManager(this, m_d->doc);
    m_d->filterManager->setup(actionCollection());

    m_d->selectionManager = new KisSelectionManager(this, m_d->doc);
    m_d->selectionManager->setup(actionCollection());

    m_d->nodeManager = new KisNodeManager(this, m_d->doc);
    m_d->nodeManager->setup(actionCollection());
    connect(m_d->doc->nodeModel(), SIGNAL(requestAddNode(KisNodeSP, KisNodeSP)), m_d->nodeManager, SLOT(addNode(KisNodeSP, KisNodeSP)));
    connect(m_d->doc->nodeModel(), SIGNAL(requestAddNode(KisNodeSP, KisNodeSP, int)), m_d->nodeManager, SLOT(insertNode(KisNodeSP, KisNodeSP, int)));
    connect(m_d->doc->nodeModel(), SIGNAL(requestMoveNode(KisNodeSP, KisNodeSP)), m_d->nodeManager, SLOT(moveNode(KisNodeSP, KisNodeSP)));
    connect(m_d->doc->nodeModel(), SIGNAL(requestMoveNode(KisNodeSP, KisNodeSP, int)), m_d->nodeManager, SLOT(moveNodeAt(KisNodeSP, KisNodeSP, int)));


    // the following cast is not really safe, but better here than in the zoomManager
    // best place would be outside kisview too
    m_d->zoomManager = new KisZoomManager(this, m_d->viewConverter, m_d->canvasController);
    m_d->zoomManager->setup(actionCollection());

    m_d->imageManager = new KisImageManager(this);
    m_d->imageManager->setup(actionCollection());

    m_d->gridManager = new KisGridManager(this);
    m_d->gridManager->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->gridManager);

    m_d->perspectiveGridManager = new KisPerspectiveGridManager(this);
    m_d->perspectiveGridManager->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->perspectiveGridManager);

    m_d->paintingAssistantManager = new KisPaintingAssistantsManager(this);
    m_d->paintingAssistantManager->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->paintingAssistantManager);
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
}


void KisView2::connectCurrentImage()
{
    if (image()) {
        if (m_d->statusBar) {
            connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace *)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
            connect(image(), SIGNAL(sigProfileChanged(KoColorProfile *)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
            connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->statusBar, SLOT(imageSizeChanged(qint32, qint32)));

        }
        connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->resourceProvider, SLOT(slotImageSizeChanged()));
        connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), this, SLOT(slotImageSizeChanged()));
        connect(image(), SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotImageSizeChanged()));
        connect(image()->undoAdapter(), SIGNAL(selectionChanged()), selectionManager(), SLOT(selectionChanged()));

    }

    m_d->canvas->connectCurrentImage();

    connect(m_d->doc->nodeModel(), SIGNAL(nodeActivated(KisNodeSP)),
            m_d->nodeManager, SLOT(activateNode(KisNodeSP)));

    if (m_d->controlFrame) {
        connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace *)), m_d->controlFrame->paintopBox(), SLOT(colorSpaceChanged(const KoColorSpace*)));
    }

}

void KisView2::disconnectCurrentImage()
{
    if (image()) {

        image()->disconnect(this);
        image()->disconnect(m_d->nodeManager);
        image()->disconnect(m_d->selectionManager);
        if (m_d->statusBar)
            image()->disconnect(m_d->statusBar);

        m_d->canvas->disconnectCurrentImage();
    }
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
    if (KisDlgPreferences::editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        m_d->resourceProvider->resetDisplayProfile();

        // Update the settings for all nodes -- they don't query
        // KisConfig directly because they need the settings during
        // compositing, and they don't connect to the confignotifier
        // because nodes are not QObjects (because only one base class
        // can be a QObject).
        KisNode* node = dynamic_cast<KisNode*>(image()->rootLayer().data());
        node->updateSettings();
    }
}

void KisView2::slotEditPalette()
{
    QList<KoColorSet*> palettes = KoResourceServerProvider::instance()->paletteServer()->resources();

    KDialog* base = new KDialog(this);
    base->setCaption(i18n("Edit Palette"));
    base->setButtons(KDialog::Ok);
    base->setDefaultButton(KDialog::Ok);
    KisCustomPalette* cp = new KisCustomPalette(palettes, base, "edit palette", i18n("Edit Palette"), this);
    base->setMainWidget(cp);
    base->show();
}

void KisView2::slotImageSizeChanged()
{
    QSizeF size(image()->width() / image()->xRes(), image()->height() / image()->yRes());
    m_d->zoomManager->zoomController()->setPageSize(size);
    m_d->zoomManager->zoomController()->setDocumentSize(size);

    QSize documentSize(int(ceil(m_d->viewConverter->documentToViewX(image()->width()  / image()->xRes()))),
                       int(ceil(m_d->viewConverter->documentToViewY(image()->height() / image()->yRes()))));
    m_d->canvasController->setDocumentSize(documentSize);

    m_d->zoomManager->updateGUI();
}


void KisView2::loadPlugins()
{
    // Load all plugins
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/ViewPlugin"),
                                                              QString::fromLatin1("(Type == 'Service') and "
                                                                                  "([X-Krita-Version] == 3)"));
    KService::List::ConstIterator iter;
    for (iter = offers.constBegin(); iter != offers.constEnd(); ++iter) {

        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
                KService::createInstance<KParts::Plugin> (service, this, QStringList(), &errCode);
        if (plugin) {
            insertChildClient(plugin);
        } else {
            if (errCode == KLibLoader::ErrNoLibrary) {
                warnKrita << " Error loading plugin was : ErrNoLibrary" << KLibLoader::self()->lastErrorMessage();
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
    return new KisPrintJob(image());
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

KisPaintingAssistantsManager* KisView2::paintingAssistantManager()
{
    return m_d->paintingAssistantManager;
}

void KisView2::slotTotalRefresh()
{
    KisConfig cfg;
    m_d->canvas->resetCanvas(cfg.useOpenGL());
}

KoFavoriteResourceManager* KisView2::favoriteResourceManager()
{
    return m_d->favoriteResourceManager;
}

void KisView2::setFavoriteResourceManager(KisPaintopBox* paintopBox)
{
    qDebug() << "KisView2: Setting favoriteResourceManager";
    m_d->favoriteResourceManager = new KoFavoriteResourceManager(paintopBox, m_d->canvas->canvasWidget());
    connect(this, SIGNAL(favoritePaletteCalled(const QPoint&)), favoriteResourceManager(), SLOT(slotShowPopupPalette(const QPoint&)));
    connect(resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)), favoriteResourceManager(), SLOT(slotAddRecentColor(KoColor)));
    connect(favoriteResourceManager(), SIGNAL(sigSetFGColor(KoColor)), resourceProvider(), SLOT(slotSetFGColor(KoColor)));

}

void KisView2::slotCanvasDestroyed(QWidget* w)
{
    qDebug() << "[KisView2] Resetting popupPalette parent";
    if (m_d->favoriteResourceManager != 0)
    {
        m_d->favoriteResourceManager->resetPopupPaletteParent(w);
    }
}

void KisView2::toggleDockers(bool toggle)
{
    Q_UNUSED(toggle);
    if (m_d->hiddenDockwidgets.isEmpty()){
        foreach(QObject* widget, mainWindow()->children()) {
            if (widget->inherits("QDockWidget")) {
                QDockWidget* dw = static_cast<QDockWidget*>(widget);
                if (dw->isVisible()) {
                    dw->hide();
                    m_d->hiddenDockwidgets << dw;
                }
            }
        }
    }
    else {
        foreach(QDockWidget* dw, m_d->hiddenDockwidgets) {
            dw->show();
        }
        m_d->hiddenDockwidgets.clear();
    }

}

void KisView2::resizeEvent ( QResizeEvent * event )
{
    dbgUI << "resize: " << event->oldSize() << " to " << event->size() << "main window" << mainWindow()->size();

    if (mainWindow()->size().height() > QApplication::desktop()->availableGeometry(this).height()) {
        mainWindow()->resize(mainWindow()->width(),
                             QApplication::desktop()->availableGeometry(this).height());
    }
}


#include "kis_view2.moc"
