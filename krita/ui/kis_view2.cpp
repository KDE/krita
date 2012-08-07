/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2003-2011 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Clarence Dang <dang@kde.org>
 *                2011 José Luis Vergara <pentalis@gmail.com>
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
#include <QPrinter>

#include <QDesktopWidget>
#include <QGridLayout>
#include <QRect>
#include <QWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QApplication>
#include <QPrintDialog>
#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QScrollBar>

#include <kio/netaccess.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <klocale.h>
#include <kmenu.h>
#include <kparts/componentfactory.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include <KoStore.h>
#include <KoMainWindow.h>
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
#include <KoTemplateCreateDia.h>
#include <KoCanvasControllerWidget.h>
#include <KoDocumentEntry.h>
#include <KoProperties.h>

#include <kis_image.h>
#include <kis_undo_adapter.h>
#include "kis_composite_progress_proxy.h"
#include <kis_layer.h>

#include "canvas/kis_canvas2.h"
#include "canvas/kis_canvas_controller.h"
#include "canvas/kis_grid_manager.h"
#include "canvas/kis_perspective_grid_manager.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_control_frame.h"
#include "kis_coordinates_converter.h"
#include "kis_custom_palette.h"
#include "kis_doc2.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_group_layer.h"
#include "kis_image_manager.h"
#include "kis_mask_manager.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_painting_assistants_manager.h"
#include <kis_paint_layer.h>
#include "kis_paintop_box.h"
#include "kis_part2.h"
#include "kis_print_job.h"
#include "kis_progress_widget.h"
#include "kis_resource_server_provider.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_shape_layer.h"
#include "kis_statusbar.h"
#include "kis_zoom_manager.h"
#include "kra/kis_kra_loader.h"
#include "widgets/kis_floating_message.h"


#include <QDebug>
#include <QPoint>
#include "kis_paintop_box.h"
#include "kis_node_commands_adapter.h"
#include <kis_paintop_preset.h>
#include "ko_favorite_resource_manager.h"
#include "kis_paintop_box.h"

#include "thememanager.h"

class BlockingUserInputEventFilter : public QObject
{
    bool eventFilter(QObject *watched, QEvent *event)
    {
        Q_UNUSED(watched);
        if(dynamic_cast<QWheelEvent*>(event)
                || dynamic_cast<QKeyEvent*>(event)
                || dynamic_cast<QMouseEvent*>(event)) {
            return true;
        }
        else {
            return false;
        }
    }
};

struct KisView2::KisView2Private
{

public:

    KisView2Private()
        : canvas(0)
        , doc(0)
        , part(0)
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
        , themeManager(0)
    {
    }

    ~KisView2Private() {
        if (canvasController) {
            KoToolManager::instance()->removeCanvasController(canvasController);
        }

        delete canvasController;
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
        delete statusBar;
    }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KisPart2 *part;
    KisCoordinatesConverter * viewConverter;
    KoCanvasController * canvasController;
    KisCanvasResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KisStatusBar * statusBar;
    KAction * totalRefresh;
    KAction* mirrorCanvas;
    KAction* createTemplate;
    KAction *saveIncremental;
    KAction *saveIncrementalBackup;
    KisSelectionManager *selectionManager;
    KisControlFrame * controlFrame;
    KisNodeManager * nodeManager;
    KisZoomManager * zoomManager;
    KisImageManager * imageManager;
    KisGridManager * gridManager;
    KisPerspectiveGridManager * perspectiveGridManager;
    KisPaintingAssistantsManager* paintingAssistantManager;
    KoFavoriteResourceManager* favoriteResourceManager;
    BlockingUserInputEventFilter blockingEventFilter;
    Digikam::ThemeManager *themeManager;
};


KisView2::KisView2(KisPart2 *part, KisDoc2 * doc, QWidget * parent)
    : KoView(part, doc, parent),
      m_d(new KisView2Private())
{
    setComponentData(KisFactory2::componentData());
    setXMLFile("krita.rc");

    // populate theme menu
    m_d->themeManager = new Digikam::ThemeManager(this);

    setFocusPolicy(Qt::NoFocus);

    if (mainWindow()) {
        actionCollection()->addAction(KStandardAction::KeyBindings, "keybindings", mainWindow()->guiFactory(), SLOT(configureShortcuts()));
    }

    m_d->doc = doc;
    m_d->part = part;
    m_d->viewConverter = new KisCoordinatesConverter();

    KoCanvasControllerWidget *canvasController = new KisCanvasController(this, actionCollection());
    canvasController->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    canvasController->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    canvasController->setDrawShadow(false);
    canvasController->setCanvasMode(KoCanvasController::Infinite);
    KisConfig cfg;
    canvasController->setZoomWithWheel(cfg.zoomWithWheel());
    canvasController->setVastScrolling(cfg.vastScrolling());

    m_d->canvasController = canvasController;

    m_d->resourceProvider = new KisCanvasResourceProvider(this);
    m_d->resourceProvider->resetDisplayProfile(QApplication::desktop()->screenNumber(this));
    m_d->canvas = new KisCanvas2(m_d->viewConverter, this, doc->shapeController());
    connect(m_d->resourceProvider, SIGNAL(sigDisplayProfileChanged(const KoColorProfile *)), m_d->canvas, SLOT(slotSetDisplayProfile(const KoColorProfile *)));

    m_d->canvasController->setCanvas(m_d->canvas);

    m_d->resourceProvider->setResourceManager(m_d->canvas->resourceManager());

    createManagers();
    createActions();

    m_d->controlFrame = new KisControlFrame(this);

    Q_ASSERT(m_d->canvasController);
    KoToolManager::instance()->addController(m_d->canvasController);
    KoToolManager::instance()->registerTools(actionCollection(), m_d->canvasController);

    // krita/krita.rc must also be modified to add actions to the menu entries

    m_d->saveIncremental = new KAction(i18n("Save Incremental &Version"), this);
    m_d->saveIncremental->setShortcut(Qt::Key_F2);
    actionCollection()->addAction("save_incremental_version", m_d->saveIncremental);
    connect(m_d->saveIncremental, SIGNAL(triggered()), this, SLOT(slotSaveIncremental()));

    m_d->saveIncrementalBackup = new KAction(i18n("Save Incremental Backup"), this);
    m_d->saveIncrementalBackup->setShortcut(Qt::Key_F4);
    actionCollection()->addAction("save_incremental_backup", m_d->saveIncrementalBackup);
    connect(m_d->saveIncrementalBackup, SIGNAL(triggered()), this, SLOT(slotSaveIncrementalBackup()));

    connect(shell(), SIGNAL(documentSaved()), this, SLOT(slotDocumentSaved()));

    if (koDocument()->localFilePath().isNull()) {
        m_d->saveIncremental->setEnabled(false);
        m_d->saveIncrementalBackup->setEnabled(false);
    }

    m_d->totalRefresh = new KAction(i18n("Total Refresh"), this);
    actionCollection()->addAction("total_refresh", m_d->totalRefresh);
    m_d->totalRefresh->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
    connect(m_d->totalRefresh, SIGNAL(triggered()), this, SLOT(slotTotalRefresh()));

    m_d->createTemplate = new KAction( i18n( "&Create Template From Image..." ), this);
    actionCollection()->addAction("createTemplate", m_d->createTemplate);
    connect(m_d->createTemplate, SIGNAL(triggered()), this, SLOT(slotCreateTemplate()));

    m_d->mirrorCanvas = new KToggleAction(i18n("Mirror Image"), this);
    m_d->mirrorCanvas->setChecked(false);
    actionCollection()->addAction("mirror_canvas", m_d->mirrorCanvas);
    m_d->mirrorCanvas->setShortcut(QKeySequence(Qt::Key_M));
    connect(m_d->mirrorCanvas, SIGNAL(toggled(bool)),m_d->canvas, SLOT(mirrorCanvas(bool)));

    KAction *rotateCanvasRight = new KAction(i18n("Rotate Canvas Right"), this);
    actionCollection()->addAction("rotate_canvas_right", rotateCanvasRight);
    rotateCanvasRight->setShortcut(QKeySequence("Ctrl+]"));
    connect(rotateCanvasRight, SIGNAL(triggered()),m_d->canvas, SLOT(rotateCanvasRight15()));

    KAction *rotateCanvasLeft = new KAction(i18n("Rotate Canvas Left"), this);
    actionCollection()->addAction("rotate_canvas_left", rotateCanvasLeft);
    rotateCanvasLeft->setShortcut(QKeySequence("Ctrl+["));
    connect(rotateCanvasLeft, SIGNAL(triggered()),m_d->canvas, SLOT(rotateCanvasLeft15()));

    KAction *resetCanvasTransformations = new KAction(i18n("Reset Canvas Transformations"), this);
    actionCollection()->addAction("reset_canvas_transformations", resetCanvasTransformations);
    resetCanvasTransformations->setShortcut(QKeySequence("Ctrl+'"));
    connect(resetCanvasTransformations, SIGNAL(triggered()),m_d->canvas, SLOT(resetCanvasTransformations()));

    KToggleAction *tAction = new KToggleAction(i18n("Show Status Bar"), this);
    tAction->setCheckedState(KGuiItem(i18n("Hide Status Bar")));
    tAction->setChecked(true);
    tAction->setToolTip(i18n("Shows or hides the status bar"));
    actionCollection()->addAction("showStatusBar", tAction);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

    tAction = new KToggleAction(i18n("Show Canvas Only"), this);
    tAction->setCheckedState(KGuiItem(i18n("Return to Window")));
    tAction->setToolTip(i18n("Shows just the canvas or the whole window"));
    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence(Qt::Key_Tab);
    tAction->setShortcuts(shortcuts);
    tAction->setChecked(false);
    actionCollection()->addAction("view_show_just_the_canvas", tAction);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showJustTheCanvas(bool)));

    //Workaround, by default has the same shortcut as mirrorCanvas
    KAction* action = dynamic_cast<KAction*>(actionCollection()->action("format_italic"));
    if (action) {
        action->setShortcut(QKeySequence(), KAction::DefaultShortcut);
        action->setShortcut(QKeySequence(), KAction::ActiveShortcut);
    }

    //Workaround, by default has the same shortcut as hide/show dockers
    action = dynamic_cast<KAction*>(shell()->actionCollection()->action("view_toggledockers"));
    if (action) {
        action->setShortcut(QKeySequence(), KAction::DefaultShortcut);
        action->setShortcut(QKeySequence(), KAction::ActiveShortcut);
    }

    if (shell())
    {
        KoToolBoxFactory toolBoxFactory(m_d->canvasController);
        shell()->createDockWidget(&toolBoxFactory);

        connect(canvasController, SIGNAL(toolOptionWidgetsChanged(const QList<QWidget *> &)),
                shell()->dockerManager(), SLOT(newOptionWidgets(const  QList<QWidget *> &)));

        shell()->dockerManager()->setIcons(false);
    }

    m_d->statusBar = new KisStatusBar(this);
    connect(m_d->canvasController->proxyObject, SIGNAL(documentMousePositionChanged(const QPointF &)),
            m_d->statusBar, SLOT(documentMousePositionChanged(const QPointF &)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->resourceProvider, SLOT(slotNodeActivated(KisNodeSP)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->controlFrame->paintopBox(), SLOT(slotCurrentNodeChanged(KisNodeSP)));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(const KoInputDevice &)),
            m_d->controlFrame->paintopBox(), SLOT(slotInputDeviceChanged(const KoInputDevice &)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            m_d->controlFrame->paintopBox(), SLOT(slotToolChanged(KoCanvasController*,int)));

    // 25 px is a distance that works well for Tablet and Mouse events
    qApp->setStartDragDistance(25);
    show();


    loadPlugins();

    // Wait for the async image to have loaded
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    if (!m_d->doc->isLoading() || m_d->doc->image()) {
        slotLoadingFinished();
    }

    // canvas sends signal that origin is changed
    connect(m_d->canvas, SIGNAL(documentOriginChanged()), m_d->zoomManager, SLOT(pageOffsetChanged()));
    connect(m_d->canvas, SIGNAL(scrollAreaSizeChanged()), m_d->zoomManager, SLOT(slotScrollAreaSizeChanged()));

    setAcceptDrops(true);

    KConfigGroup group(KGlobal::config(), "krita/shortcuts");
    foreach(KActionCollection *collection, KActionCollection::allCollections()) {
        collection->setConfigGroup("krita/shortcuts");
        collection->readSettings(&group);
    }


#if 0
    //check for colliding shortcuts
    QSet<QKeySequence> existingShortcuts;
    foreach(QAction* action, actionCollection()->actions()) {
        if(action->shortcut() == QKeySequence(0)) {
            continue;
        }
        dbgUI << "shortcut " << action->text() << " " << action->shortcut();
        Q_ASSERT(!existingShortcuts.contains(action->shortcut()));
        existingShortcuts.insert(action->shortcut());
    }
#endif
}


KisView2::~KisView2()
{
    {
        KConfigGroup group(KGlobal::config(), "theme");
        group.writeEntry("Theme", m_d->themeManager->currentThemeName());
    }
    {
        KConfigGroup group(KGlobal::config(), "krita/shortcuts");
        foreach(KActionCollection *collection, KActionCollection::allCollections()) {
            collection->setConfigGroup("krita/shortcuts");
            collection->writeSettings(&group);
        }
    }
    delete m_d;
}


void KisView2::dragEnterEvent(QDragEnterEvent *event)
{
    dbgUI << "KisView2::dragEnterEvent";
    // Only accept drag if we're not busy, particularly as we may
    // be showing a progress bar and calling qApp->processEvents().
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node")) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisView2::dropEvent(QDropEvent *event)
{
    KisImageSP kisimage = image();

    QPointF pos = kisimage->documentToIntPixel(m_d->viewConverter->viewToDocument(event->pos() + m_d->canvas->documentOffset() - m_d->canvas->documentOrigin()));

    if (event->mimeData()->hasFormat("application/x-krita-node") || event->mimeData()->hasImage())
    {
        KisNodeSP node;

        if (event->mimeData()->hasFormat("application/x-krita-node")) {

            QByteArray ba = event->mimeData()->data("application/x-krita-node");

            KisDoc2 tempDoc;
            tempDoc.loadNativeFormatFromStore(ba);

            KisImageWSP tempImage = tempDoc.image();
            node = tempImage->rootLayer()->firstChild();
            tempImage->removeNode(node);

            // layers store a link to the image, so update it
            KisLayer *layer = dynamic_cast<KisLayer*>(node.data());
            if (layer) {
                layer->setImage(kisimage);
            }
            KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node.data());
            if (shapeLayer) {
                KoShapeContainer * parentContainer =
                    dynamic_cast<KoShapeContainer*>(m_d->doc->shapeForNode(kisimage->rootLayer()));

                KisShapeLayer *shapeLayer2 = new KisShapeLayer(parentContainer, m_d->doc->shapeController(), kisimage, node->name(), node->opacity());
                QList<KoShape *> shapes = shapeLayer->shapes();
                shapeLayer->removeAllShapes();
                foreach(KoShape *shape, shapes) {
                    shapeLayer2->addShape(shape);
                }
                node = shapeLayer2;
            }

        }
        else if (event->mimeData()->hasImage()) {
            QImage qimage = qvariant_cast<QImage>(event->mimeData()->imageData());

            if (kisimage) {
                KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
                device->convertFromQImage(qimage, 0);
                node = new KisPaintLayer(kisimage.data(), kisimage->nextLayerName(), OPACITY_OPAQUE_U8, device);
            }
        }

        if (node) {

            node->setX(pos.x() - node->projection()->exactBounds().width());
            node->setY(pos.y() - node->projection()->exactBounds().height());

            KisNodeCommandsAdapter adapter(this);
            if (!m_d->nodeManager->activeLayer()) {
                adapter.addNode(node, kisimage->rootLayer() , 0);
            } else {
                adapter.addNode(node,
                                m_d->nodeManager->activeLayer()->parent(),
                                m_d->nodeManager->activeLayer());
            }
        }

        return;

    }

    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.length() > 0) {

            KMenu popup;
            popup.setObjectName("drop_popup");

            QAction *insertAsNewLayer = new KAction(i18n("Insert as New Layer"), &popup);
            QAction *insertAsNewLayers = new KAction(i18n("Insert as New Layers"), &popup);

            QAction *openInNewDocument = new KAction(i18n("Open in New Document"), &popup);
            QAction *openInNewDocuments = new KAction(i18n("Open in New Documents"), &popup);

            QAction *replaceCurrentDocument = new KAction(i18n("Replace Current Document"), &popup);

            QAction *cancel = new KAction(i18n("Cancel"), &popup);

            if (urls.count() == 1) {
                if (!image().isNull()) {
                    popup.addAction(insertAsNewLayer);
                }
                popup.addAction(openInNewDocument);
                popup.addAction(replaceCurrentDocument);
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
                foreach(QUrl url, urls) {

                    if (action == insertAsNewLayer || action == insertAsNewLayers) {
                        m_d->imageManager->importImage(KUrl(url));
                    }
                    else if (action == replaceCurrentDocument) {
                        if (m_d->part->isModified()) {
                            m_d->part->save();
                        }
                        if (shell() != 0) {
                            shell()->openDocument(url);
                        }
                        shell()->close();
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

KisPaintopBox* KisView2::paintOpBox() const
{
    return m_d->controlFrame->paintopBox();
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

KisNodeSP KisView2::activeNode()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activeNode();
    else
        return 0;
}

KisLayerSP KisView2::activeLayer()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activeLayer();
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

bool KisView2::selectionEditable()
{
    KisLayerSP layer = activeLayer();
    if (layer) {
        KoProperties properties;
        QList<KisNodeSP> masks = layer->childNodes(QStringList("KisSelectionMask"), properties);
        if (masks.size() == 1) {
            return masks[0]->isEditable();
        }
    }
    // global selection is always editable
    return true;
}

KisUndoAdapter * KisView2::undoAdapter()
{
    KisImageWSP image = m_d->doc->image();
    Q_ASSERT(image);

    return image->undoAdapter();
}


void KisView2::slotLoadingFinished()
{
    /**
     * Cold-start of image-size signals
     */
    slotImageSizeChanged();
    if (m_d->statusBar) {
        m_d->statusBar->imageSizeChanged(image()->width(), image()->height());
    }
    if (m_d->resourceProvider) {
        m_d->resourceProvider->slotImageSizeChanged();
    }
    if (m_d->nodeManager) {
        m_d->nodeManager->nodesUpdated();
    }

    connectCurrentImage();

    if (image()->locked()) {
        // If this is the first view on the image, the image will have been locked
        // so unlock it.
        image()->blockSignals(false);
        image()->unlock();
    }

    KisNodeSP activeNode = m_d->doc->preActivatedNode();
    m_d->doc->setPreActivatedNode(0); // to make sure that we don't keep a reference to a layer the user can later delete.

    if (!activeNode) {
        activeNode = image()->rootLayer()->firstChild();
    }

    while (activeNode && !activeNode->inherits("KisLayer")) {
        activeNode = activeNode->nextSibling();
    }

    if (activeNode) {
        m_d->nodeManager->slotNonUiActivatedNode(activeNode);
    }


    // get the assistants and push them to the manager
    QList<KisPaintingAssistant*> paintingAssistants = m_d->doc->preLoadedAssistants();
    foreach (KisPaintingAssistant* assistant, paintingAssistants) {
        m_d->paintingAssistantManager->addAssistant(assistant);
    }

    /**
     * Dirty hack alert
     */
    if (m_d->viewConverter)
        m_d->viewConverter->setZoomMode(KoZoomMode::ZOOM_PAGE);
    if (m_d->zoomManager && m_d->zoomManager->zoomController())
        m_d->zoomManager->zoomController()->setAspectMode(true);
    if (m_d->paintingAssistantManager){
        foreach(KisPaintingAssistant* assist, m_d->doc->preLoadedAssistants()){
            m_d->paintingAssistantManager->addAssistant(assist);
        }
        m_d->paintingAssistantManager->setVisible(true);
    }
    updateGUI();

    emit sigLoadingFinished();
}


void KisView2::createActions()
{
    actionCollection()->addAction(KStandardAction::Preferences,  "preferences", this, SLOT(slotPreferences()));

    KAction* action = new KAction(i18n("Edit Palette..."), this);
    actionCollection()->addAction("edit_palette", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditPalette()));

    KConfigGroup group(KGlobal::config(), "theme");
    m_d->themeManager->setThemeMenuAction(new KActionMenu(i18n("&Themes"), this));
    m_d->themeManager->registerThemeActions(actionCollection());
    m_d->themeManager->setCurrentTheme(group.readEntry("Theme",
                                                       m_d->themeManager->defaultThemeName()));

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
            connect(image(), SIGNAL(sigProfileChanged(const KoColorProfile *)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
            connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->statusBar, SLOT(imageSizeChanged(qint32, qint32)));

        }
        connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), m_d->resourceProvider, SLOT(slotImageSizeChanged()));

        connect(image(), SIGNAL(sigResolutionChanged(double, double)),
                m_d->resourceProvider, SLOT(slotOnScreenResolutionChanged()));
        connect(zoomManager()->zoomController(), SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
                m_d->resourceProvider, SLOT(slotOnScreenResolutionChanged()));

        connect(image(), SIGNAL(sigSizeChanged(qint32, qint32)), this, SLOT(slotImageSizeChanged()));
        connect(image(), SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotImageSizeChanged()));
        connect(image(), SIGNAL(sigNodeChanged(KisNodeSP)), this, SLOT(slotNodeChanged()));
        connect(image()->undoAdapter(), SIGNAL(selectionChanged()), selectionManager(), SLOT(selectionChanged()));

        /**
         * WARNING: Currently we access the global progress bar in two ways:
         * connecting to composite progress proxy (strokes) and creating
         * progress updaters. The latter way should be depracated in favour
         * of displaying the status of the global strokes queue
         */
        //image()->compositeProgressProxy()->addProxy(m_d->statusBar->progress()->progressProxy());
    }

    m_d->canvas->connectCurrentImage();

    if (m_d->controlFrame) {
        connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace *)), m_d->controlFrame->paintopBox(), SLOT(slotColorSpaceChanged(const KoColorSpace*)));
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

void KisView2::slotPreferences()
{
    if (KisDlgPreferences::editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        m_d->resourceProvider->resetDisplayProfile(QApplication::desktop()->screenNumber(this));
        KisConfig cfg;
        static_cast<KoCanvasControllerWidget*>(m_d->canvasController)->setZoomWithWheel(cfg.zoomWithWheel());

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

    canvasBase()->notifyZoomChanged();
    QSize widgetSize = canvasBase()->coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect().size();
    m_d->canvasController->updateDocumentSize(widgetSize, true);

    m_d->zoomManager->updateGUI();

    //update view
    canvas()->update();
}

void KisView2::slotNodeChanged()
{
    updateGUI();
}

void KisView2::loadPlugins()
{
    // Load all plugins
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/ViewPlugin"),
                                                              QString::fromLatin1("(Type == 'Service') and "
                                                                                  "([X-Krita-Version] == 5)"));
    KService::List::ConstIterator iter;
    for (iter = offers.constBegin(); iter != offers.constEnd(); ++iter) {
        KService::Ptr service = *iter;
        dbgUI << "Load plugin " << service->name();
        QString error;

        KParts::Plugin* plugin =
                dynamic_cast<KParts::Plugin*>(service->createInstance<QObject>(this, QVariantList(), &error));
        if (plugin) {
            insertChildClient(plugin);
        } else {
            errKrita << "Fail to create an instance for " << service->name() << " " << error;
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

void KisView2::slotCreateTemplate()
{
    KoTemplateCreateDia::createTemplate("krita_template", ".kra",
                                        KisFactory2::componentData(), m_d->doc, this);
}

void KisView2::slotDocumentSaved()
{
    m_d->saveIncremental->setEnabled(true);
    m_d->saveIncrementalBackup->setEnabled(true);
}

void KisView2::slotSaveIncremental()
{
    KoDocument* pDoc = koDocument();
    if (!pDoc) return;

    bool foundVersion;
    bool fileAlreadyExists;
    bool isBackup;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString fileName = pDoc->localFilePath();

    // Find current version filenames
    // v v Regexp to find incremental versions in the filename, taking our backup scheme into account as well
    // Considering our incremental version and backup scheme, format is filename_001~001.ext
    QRegExp regex("_\\d{1,4}[.]|_\\d{1,4}[a-z][.]|_\\d{1,4}[~]|_\\d{1,4}[a-z][~]");
    regex.indexIn(fileName);     //  Perform the search
    QStringList matches = regex.capturedTexts();
    foundVersion = matches.at(0).isEmpty() ? false : true;

    // Ensure compatibility with Save Incremental Backup
    // If this regex is not kept separate, the entire algorithm needs modification;
    // It's simpler to just add this.
    QRegExp regexAux("_\\d{1,4}[~]|_\\d{1,4}[a-z][~]");
    regexAux.indexIn(fileName);     //  Perform the search
    QStringList matchesAux = regexAux.capturedTexts();
    isBackup = matchesAux.at(0).isEmpty() ? false : true;

    // If the filename has a version, prepare it for incrementation
    if (foundVersion) {
        version = matches.at(matches.count() - 1);     //  Look at the last index, we don't care about other matches
        if (version.contains(QRegExp("[a-z]"))) {
            version.chop(1);             //  Trim "."
            letter = version.right(1);   //  Save letter
            version.chop(1);             //  Trim letter
        } else {
            version.chop(1);             //  Trim "."
        }
        version.remove(0, 1);            //  Trim "_"
    } else {
        // ...else, simply add a version to it so the next loop works
        QRegExp regex2("[.][a-z]{2,4}$");  //  Heuristic to find file extension
        regex2.indexIn(fileName);
        QStringList matches2 = regex2.capturedTexts();
        QString extensionPlusVersion = matches2.at(0);
        extensionPlusVersion.prepend(version);
        extensionPlusVersion.prepend("_");
        fileName.replace(regex2, extensionPlusVersion);
    }

    // Prepare the base for new version filename
    int intVersion = version.toInt(0);
    ++intVersion;
    QString baseNewVersion = QString::number(intVersion);
    while (baseNewVersion.length() < version.length()) {
        baseNewVersion.prepend("0");
    }

    // Check if the file exists under the new name and search until options are exhausted (test appending a to z)
    do {
        newVersion = baseNewVersion;
        newVersion.prepend("_");
        if (!letter.isNull()) newVersion.append(letter);
        if (isBackup) {
            newVersion.append("~");
        } else {
            newVersion.append(".");
        }
        fileName.replace(regex, newVersion);
        fileAlreadyExists = KIO::NetAccess::exists(fileName, KIO::NetAccess::DestinationSide, this);
        if (fileAlreadyExists) {
            if (!letter.isNull()) {
                char letterCh = letter.at(0).toLatin1();
                ++letterCh;
                letter = QString(QChar(letterCh));
            } else {
                letter = "a";
            }
        }
    } while (fileAlreadyExists && letter != "{");  // x, y, z, {...

    if (letter == "{") {
        KMessageBox::error(this, "Alternative names exhausted, try manually saving with a higher number", "Couldn't save incremental version");
        return;
    }
    pDoc->setSaveInBatchMode(true);
    m_d->part->saveAs(fileName);
    pDoc->setSaveInBatchMode(false);

    shell()->updateCaption();
}

void KisView2::slotSaveIncrementalBackup()
{
    KoDocument* pDoc = koDocument();
    if (!pDoc) return;

    bool workingOnBackup;
    bool fileAlreadyExists;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString fileName = pDoc->localFilePath();

    // First, discover if working on a backup file, or a normal file
    QRegExp regex("~\\d{1,4}[.]|~\\d{1,4}[a-z][.]");
    regex.indexIn(fileName);     //  Perform the search
    QStringList matches = regex.capturedTexts();
    workingOnBackup = matches.at(0).isEmpty() ? false : true;

    if (workingOnBackup) {
        // Try to save incremental version (of backup), use letter for alt versions
        version = matches.at(matches.count() - 1);     //  Look at the last index, we don't care about other matches
        if (version.contains(QRegExp("[a-z]"))) {
            version.chop(1);             //  Trim "."
            letter = version.right(1);   //  Save letter
            version.chop(1);             //  Trim letter
        } else {
            version.chop(1);             //  Trim "."
        }
        version.remove(0, 1);            //  Trim "~"

        // Prepare the base for new version filename
        int intVersion = version.toInt(0);
        ++intVersion;
        QString baseNewVersion = QString::number(intVersion);
        while (baseNewVersion.length() < version.length()) {
            baseNewVersion.prepend("0");
        }

        // Check if the file exists under the new name and search until options are exhausted (test appending a to z)
        do {
            newVersion = baseNewVersion;
            newVersion.prepend("~");
            if (!letter.isNull()) newVersion.append(letter);
            newVersion.append(".");
            fileName.replace(regex, newVersion);
            fileAlreadyExists = KIO::NetAccess::exists(fileName, KIO::NetAccess::DestinationSide, this);
            if (fileAlreadyExists) {
                if (!letter.isNull()) {
                    char letterCh = letter.at(0).toLatin1();
                    ++letterCh;
                    letter = QString(QChar(letterCh));
                } else {
                    letter = "a";
                }
            }
        } while (fileAlreadyExists && letter != "{");  // x, y, z, {...

        if (letter == "{") {
            KMessageBox::error(this, "Alternative names exhausted, try manually saving with a higher number", "Couldn't save incremental backup");
            return;
        }
        m_d->part->saveAs(fileName);

        shell()->updateCaption();
    }
    else { // if NOT working on a backup...
        // Navigate directory searching for latest backup version, ignore letters
        const quint8 HARDCODED_DIGIT_COUNT = 3;
        QString baseNewVersion = "000";
        QString backupFileName = pDoc->localFilePath();
        QRegExp regex2("[.][a-z]{2,4}$");  //  Heuristic to find file extension
        regex2.indexIn(backupFileName);
        QStringList matches2 = regex2.capturedTexts();
        QString extensionPlusVersion = matches2.at(0);
        extensionPlusVersion.prepend(baseNewVersion);
        extensionPlusVersion.prepend("~");
        backupFileName.replace(regex2, extensionPlusVersion);

        // Save version with 1 number higher than the highest version found ignoring letters
        do {
            newVersion = baseNewVersion;
            newVersion.prepend("~");
            newVersion.append(".");
            backupFileName.replace(regex, newVersion);
            fileAlreadyExists = KIO::NetAccess::exists(backupFileName, KIO::NetAccess::DestinationSide, this);
            if (fileAlreadyExists) {
                // Prepare the base for new version filename, increment by 1
                int intVersion = baseNewVersion.toInt(0);
                ++intVersion;
                baseNewVersion = QString::number(intVersion);
                while (baseNewVersion.length() < HARDCODED_DIGIT_COUNT) {
                    baseNewVersion.prepend("0");
                }
            }
        } while (fileAlreadyExists);

        // Save both as backup and on current file for interapplication workflow
        pDoc->setSaveInBatchMode(true);
        m_d->part->saveAs(backupFileName);
        m_d->part->saveAs(fileName);
        pDoc->setSaveInBatchMode(false);

        shell()->updateCaption();
    }
}

void KisView2::disableControls()
{
    // prevents possible crashes, if somebody changes the paintop during dragging by using the mousewheel
    // this is for Bug 250944
    // the solution blocks all wheel, mouse and key event, while dragging with the freehand tool
    // see KisToolFreehand::initPaint() and endPaint()
    m_d->controlFrame->paintopBox()->installEventFilter(&m_d->blockingEventFilter);
    foreach(QObject* child, m_d->controlFrame->paintopBox()->children()) {
        child->installEventFilter(&m_d->blockingEventFilter);
    }
}

void KisView2::enableControls()
{
    m_d->controlFrame->paintopBox()->removeEventFilter(&m_d->blockingEventFilter);
    foreach(QObject* child, m_d->controlFrame->paintopBox()->children()) {
        child->removeEventFilter(&m_d->blockingEventFilter);
    }
}

void KisView2::showStatusBar(bool toggled)
{
    if (KoView::statusBar()) {
        KoView::statusBar()->setVisible(toggled);
    }
}

void KisView2::showJustTheCanvas(bool toggled)
{
    KisConfig cfg;
    KToggleAction *action;

    if (cfg.hideStatusbarFullscreen()) {
        action = dynamic_cast<KToggleAction*>(actionCollection()->action("showStatusBar"));
        if (action && action->isChecked() == toggled) {
            action->setChecked(!toggled);
        }
    }

    if (cfg.hideDockersFullscreen()) {
        action = dynamic_cast<KToggleAction*>(shell()->actionCollection()->action("view_toggledockers"));
        if (action && action->isChecked() == toggled) {
            action->setChecked(!toggled);
        }
    }

    if (cfg.hideTitlebarFullscreen()) {
        action = dynamic_cast<KToggleAction*>(shell()->actionCollection()->action("view_fullscreen"));
        if (action && action->isChecked() != toggled) {
            action->setChecked(toggled);
        }
    }

    if (cfg.hideMenuFullscreen()) {
        if (shell()->menuBar()->isVisible() == toggled) {
            shell()->menuBar()->setVisible(!toggled);
        }
    }

    if (cfg.hideToolbarFullscreen()) {
        foreach(KToolBar* toolbar, shell()->toolBars()) {
            if (toolbar->isVisible() == toggled) {
                toolbar->setVisible(!toggled);
            }
        }
    }

    if (cfg.hideScrollbarsFullscreen()) {
        if (toggled) {
            dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        else {
            dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        }
    }

    if (toggled) {
        // show a fading heads-up display about the shortcut to go back
        KisFloatingMessage *floatingMessage = new KisFloatingMessage(i18n("Going into Canvas-Only mode.\nPress %1 to go back.",
                                                                          actionCollection()->action("view_show_just_the_canvas")->shortcut().toString()), this);
        floatingMessage->showMessage();
    }
}

void KisView2::showFloatingMessage(const QString message, const QIcon& icon)
{
    KisFloatingMessage *floatingMessage = new KisFloatingMessage(message, mainWindow()->centralWidget());
    floatingMessage->setShowOverParent(true);
    floatingMessage->setIcon(icon);
    floatingMessage->showMessage();
}

#include "kis_view2.moc"
