/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisView.h"

#include "KisView_p.h"

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include <KoDocumentInfo.h>
#include <KoToolManager.h>

#include <kis_icon.h>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kselectaction.h>
#include <kconfiggroup.h>

#include <QMenu>
#include <QMessageBox>
#include <QUrl>
#include <QTemporaryFile>
#include <QApplication>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>
#include <QList>
#include <QPrintDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QMoveEvent>
#include <QMdiSubWindow>
#include <QFileInfo>

#include <kis_image.h>
#include <kis_node.h>

#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_mask.h>
#include <kis_selection.h>

#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "KisDocument.h"
#include "kis_image_manager.h"
#include "KisMainWindow.h"
#include "kis_mimedata.h"
#include "kis_mirror_axis.h"
#include "kis_node_commands_adapter.h"
#include "kis_node_manager.h"
#include "KisPart.h"
#include "kis_shape_controller.h"
#include "kis_tool_freehand.h"
#include "KisViewManager.h"
#include "kis_zoom_manager.h"
#include "kis_statusbar.h"
#include "kis_painting_assistants_decoration.h"
#include "KisReferenceImagesDecoration.h"
#include "kis_progress_widget.h"
#include "kis_signal_compressor.h"
#include "kis_filter_manager.h"
#include "kis_file_layer.h"
#include "krita_utils.h"
#include "input/kis_input_manager.h"
#include "KisRemoteFileFetcher.h"
#include "kis_selection_manager.h"
#include "kis_fill_painter.h"
#include "KisImageSignals.h"
#include "kis_resources_snapshot.h"
#include "kis_processing_applicator.h"
#include "processing/fill_processing_visitor.h"
#include "utils/KisClipboardUtil.h"

//static
QString KisView::newObjectName()
{
    static int s_viewIFNumber = 0;
    QString name; name.setNum(s_viewIFNumber++); name.prepend("view_");
    return name;
}

bool KisView::s_firstView = true;

class Q_DECL_HIDDEN KisView::Private
{
public:
    Private(KisView *_q,
            KisDocument *document,
            KisViewManager *viewManager)
        : actionCollection(viewManager->actionCollection())
        , viewConverter()
        , canvasController(_q, viewManager->mainWindow(), viewManager->actionCollection())
        , canvas(&viewConverter, viewManager->canvasResourceProvider()->resourceManager(), viewManager->mainWindow(), _q, document->shapeController())
        , zoomManager(_q, &this->viewConverter, &this->canvasController)
        , viewManager(viewManager)
        , floatingMessageCompressor(100, KisSignalCompressor::POSTPONE)
    {
    }

    bool inOperation; //in the middle of an operation (no screen refreshing)?

    QPointer<KisDocument> document; // our KisDocument
    QWidget *tempActiveWidget = 0;

    KActionCollection* actionCollection;
    KisCoordinatesConverter viewConverter;
    KisCanvasController canvasController;
    KisCanvas2 canvas;
    KisZoomManager zoomManager;
    KisViewManager *viewManager = 0;
    KisNodeSP currentNode;
    KisPaintingAssistantsDecorationSP paintingAssistantsDecoration;
    KisReferenceImagesDecorationSP referenceImagesDecoration;
    bool isCurrent = false;
    bool showFloatingMessage = false;
    QPointer<KisFloatingMessage> savedFloatingMessage;
    KisSignalCompressor floatingMessageCompressor;
    QMdiSubWindow *subWindow{nullptr};

    bool softProofing = false;
    bool gamutCheck = false;

    // Hmm sorry for polluting the private class with such a big inner class.
    // At the beginning it was a little struct :)
    class StatusBarItem
    {
    public:

        StatusBarItem(QWidget * widget, int stretch, bool permanent)
            : m_widget(widget),
              m_stretch(stretch),
              m_permanent(permanent),
              m_connected(false),
              m_hidden(false) {}

        bool operator==(const StatusBarItem& rhs) {
            return m_widget == rhs.m_widget;
        }

        bool operator!=(const StatusBarItem& rhs) {
            return m_widget != rhs.m_widget;
        }

        QWidget * widget() const {
            return m_widget;
        }

        void ensureItemShown(QStatusBar * sb) {
            Q_ASSERT(m_widget);
            if (!m_connected) {
                if (m_permanent)
                    sb->addPermanentWidget(m_widget, m_stretch);
                else
                    sb->addWidget(m_widget, m_stretch);

                if(!m_hidden)
                    m_widget->show();

                m_connected = true;
            }
        }
        void ensureItemHidden(QStatusBar * sb) {
            if (m_connected) {
                m_hidden = m_widget->isHidden();
                sb->removeWidget(m_widget);
                m_widget->hide();
                m_connected = false;
            }
        }
    private:
        QWidget * m_widget = 0;
        int m_stretch;
        bool m_permanent;
        bool m_connected = false;
        bool m_hidden = false;

    };

};

KisView::KisView(KisDocument *document, KisViewManager *viewManager, QWidget *parent)
    : QWidget(parent)
    , d(new Private(this, document, viewManager))
{
    Q_ASSERT(document);
    connect(document, SIGNAL(titleModified(QString,bool)), this, SIGNAL(titleModified(QString,bool)));
    setObjectName(newObjectName());

    d->document = document;

    setFocusPolicy(Qt::StrongFocus);

    QStatusBar * sb = statusBar();
    if (sb) { // No statusbar in e.g. konqueror
        connect(d->document, SIGNAL(statusBarMessage(QString,int)),
                this, SLOT(slotSavingStatusMessage(QString,int)));
        connect(d->document, SIGNAL(clearStatusBarMessage()),
                this, SLOT(slotClearStatusText()));
    }

    d->canvas.setup();

    KisConfig cfg(false);

    d->canvasController.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->canvasController.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->canvasController.setVastScrolling(cfg.vastScrolling());
    d->canvasController.setCanvas(&d->canvas);

    d->zoomManager.setup(d->actionCollection);


    connect(&d->canvasController, SIGNAL(documentSizeChanged()), &d->zoomManager, SLOT(slotScrollAreaSizeChanged()));
    setAcceptDrops(true);

    connect(d->document, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    connect(d->document, SIGNAL(sigSavingFinished()), this, SLOT(slotSavingFinished()));

    d->referenceImagesDecoration = new KisReferenceImagesDecoration(this, document, /* viewReady = */ false);
    d->canvas.addDecoration(d->referenceImagesDecoration);
    d->referenceImagesDecoration->setVisible(true);

    d->paintingAssistantsDecoration = new KisPaintingAssistantsDecoration(this);
    d->canvas.addDecoration(d->paintingAssistantsDecoration);
    d->paintingAssistantsDecoration->setVisible(true);

    d->showFloatingMessage = cfg.showCanvasMessages();
    d->zoomManager.updateScreenResolution(this);
}

KisView::~KisView()
{
    if (d->viewManager) {
        if (d->viewManager->filterManager()->isStrokeRunning()) {
            d->viewManager->filterManager()->cancel();
        }

        d->viewManager->mainWindow()->notifyChildViewDestroyed(this);
        d->viewManager->inputManager()->registerPopupWidget(nullptr);
    }

    image()->requestStrokeCancellation();

    /**
     * KisCanvas2 maintains direct connections to the image, so we should
     * disconnect it from the image before the destruction process starts
     */
    d->canvas.disconnectImage();

    KoToolManager::instance()->removeCanvasController(&d->canvasController);
    d->canvasController.setCanvas(0);

    KisPart::instance()->removeView(this);
    delete d;
}

void KisView::notifyCurrentStateChanged(bool isCurrent)
{
    d->isCurrent = isCurrent;

    if (!d->isCurrent && d->savedFloatingMessage) {
        d->savedFloatingMessage->removeMessage();
    }

    KisInputManager *inputManager = globalInputManager();
    if (d->isCurrent) {
        inputManager->attachPriorityEventFilter(&d->canvasController);
    } else {
        inputManager->detachPriorityEventFilter(&d->canvasController);
    }

    /**
     * When current view is changed, currently selected node is also changed,
     * therefore we should update selection overlay mask
     */
    viewManager()->selectionManager()->selectionChanged();
}

bool KisView::isCurrent() const
{
    return d->isCurrent;
}

void KisView::setShowFloatingMessage(bool show)
{
    d->showFloatingMessage = show;
}

void KisView::showFloatingMessage(const QString &message, const QIcon& icon, int timeout, KisFloatingMessage::Priority priority, int alignment)
{
    if (!d->viewManager) return;

    if(d->isCurrent && d->showFloatingMessage && d->viewManager->qtMainWindow()) {
        if (d->savedFloatingMessage) {
            d->savedFloatingMessage->tryOverrideMessage(message, icon, timeout, priority, alignment);
        } else {
            d->savedFloatingMessage = new KisFloatingMessage(message, this->canvasBase()->canvasWidget(), false, timeout, priority, alignment);
            d->savedFloatingMessage->setShowOverParent(true);
            d->savedFloatingMessage->setIcon(icon);

            connect(&d->floatingMessageCompressor, SIGNAL(timeout()), d->savedFloatingMessage, SLOT(showMessage()));
            d->floatingMessageCompressor.start();
        }
    }
}

bool KisView::canvasIsMirrored() const
{
    return d->canvas.xAxisMirrored() || d->canvas.yAxisMirrored();
}

void KisView::setViewManager(KisViewManager *view)
{
    d->viewManager = view;

    KoToolManager::instance()->addController(&d->canvasController);
    KoToolManager::instance()->registerToolActions(d->actionCollection, &d->canvasController);
    dynamic_cast<KisShapeController*>(d->document->shapeController())->setInitialShapeForCanvas(&d->canvas);

    if (resourceProvider()) {
        resourceProvider()->slotImageSizeChanged();
    }

    if (d->viewManager && d->viewManager->nodeManager()) {
        d->viewManager->nodeManager()->nodesUpdated();
    }

    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SLOT(slotImageSizeChanged(QPointF,QPointF)));
    connect(image(), SIGNAL(sigResolutionChanged(double,double)), this, SLOT(slotImageResolutionChanged()));

    // executed in a context of an image thread
    connect(image(), SIGNAL(sigNodeAddedAsync(KisNodeSP)),
            SLOT(slotImageNodeAdded(KisNodeSP)),
            Qt::DirectConnection);

    // executed in a context of the gui thread
    connect(this, SIGNAL(sigContinueAddNode(KisNodeSP)),
            SLOT(slotContinueAddNode(KisNodeSP)),
            Qt::AutoConnection);

    // executed in a context of an image thread
    connect(image(), SIGNAL(sigRemoveNodeAsync(KisNodeSP)),
            SLOT(slotImageNodeRemoved(KisNodeSP)),
            Qt::DirectConnection);

    // executed in a context of the gui thread
    connect(this, SIGNAL(sigContinueRemoveNode(KisNodeSP)),
            SLOT(slotContinueRemoveNode(KisNodeSP)),
            Qt::AutoConnection);

    d->viewManager->updateGUI();

    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");
}

KisViewManager* KisView::viewManager() const
{
    return d->viewManager;
}

void KisView::slotImageNodeAdded(KisNodeSP node)
{
    emit sigContinueAddNode(node);
}

void KisView::slotContinueAddNode(KisNodeSP newActiveNode)
{
    /**
     * When deleting the last layer, root node got selected. We should
     * fix it when the first layer is added back.
     *
     * Here we basically reimplement what Qt's view/model do. But
     * since they are not connected, we should do it manually.
     */

    if (!d->isCurrent &&
            (!d->currentNode || !d->currentNode->parent())) {

        d->currentNode = newActiveNode;
    }
}


void KisView::slotImageNodeRemoved(KisNodeSP node)
{
    emit sigContinueRemoveNode(KritaUtils::nearestNodeAfterRemoval(node));
}

void KisView::slotContinueRemoveNode(KisNodeSP newActiveNode)
{
    if (!d->isCurrent) {
        d->currentNode = newActiveNode;
    }
}

KoZoomController *KisView::zoomController() const
{
    return d->zoomManager.zoomController();
}

KisZoomManager *KisView::zoomManager() const
{
    return &d->zoomManager;
}

KisCanvasController *KisView::canvasController() const
{
    return &d->canvasController;
}

KisCanvasResourceProvider *KisView::resourceProvider() const
{
    if (d->viewManager) {
        return d->viewManager->canvasResourceProvider();
    }
    return 0;
}

KisInputManager* KisView::globalInputManager() const
{
    return d->viewManager ? d->viewManager->inputManager() : 0;
}

KisCanvas2 *KisView::canvasBase() const
{
    return &d->canvas;
}

KisImageWSP KisView::image() const
{
    if (d->document) {
        return d->document->image();
    }
    return 0;
}


KisCoordinatesConverter *KisView::viewConverter() const
{
    return &d->viewConverter;
}

void KisView::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug() << "KisView::dragEnterEvent formats" << event->mimeData()->formats() << "urls" << event->mimeData()->urls() << "has images" << event->mimeData()->hasImage();
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node")
            || event->mimeData()->hasFormat("krita/x-colorsetentry")
            || event->mimeData()->hasColor()) {
        event->accept();

        // activate view if it should accept the drop
        this->setFocus();
    } else {
        event->ignore();
    }
}

void KisView::dropEvent(QDropEvent *event)
{
    KisImageWSP kisimage = image();
    Q_ASSERT(kisimage);

    QPoint cursorPos = canvasBase()->coordinatesConverter()->widgetToImage(event->pos()).toPoint();
    QRect imageBounds = kisimage->bounds();
    QPoint pasteCenter;
    bool forceRecenter;

    if (event->keyboardModifiers() & Qt::ShiftModifier &&
            imageBounds.contains(cursorPos)) {

        pasteCenter = cursorPos;
        forceRecenter = true;
    } else {
        pasteCenter = imageBounds.center();
        forceRecenter = false;
    }

    //qDebug() << "KisView::dropEvent() formats" << event->mimeData()->formats() << "urls" << event->mimeData()->urls() << "has images" << event->mimeData()->hasImage();

    if (event->mimeData()->hasFormat("application/x-krita-node") ||
            event->mimeData()->hasImage())
    {
        KisShapeController *kritaShapeController =
                dynamic_cast<KisShapeController*>(d->document->shapeController());

        QList<KisNodeSP> nodes =
                KisMimeData::loadNodes(event->mimeData(), imageBounds,
                                       pasteCenter, forceRecenter,
                                       kisimage, kritaShapeController);

        Q_FOREACH (KisNodeSP node, nodes) {
            if (node) {
                KisNodeCommandsAdapter adapter(viewManager());
                if (!viewManager()->nodeManager()->activeLayer()) {
                    adapter.addNode(node, kisimage->rootLayer() , 0);
                } else {
                    adapter.addNode(node,
                                    viewManager()->nodeManager()->activeLayer()->parent(),
                                    viewManager()->nodeManager()->activeLayer());
                }
            }
        }
    }
    else if (event->mimeData()->hasUrls()) {

        KisClipboardUtil::clipboardHasUrlsAction(this, event->mimeData());

    }
    else if (event->mimeData()->hasColor() || event->mimeData()->hasFormat("krita/x-colorsetentry")) {
        if (image() && d->viewManager->activeDevice()) {
            KisProcessingApplicator applicator(image(), d->viewManager->activeNode(),
                                               KisProcessingApplicator::NONE,
                                               KisImageSignalVector(),
                                               kundo2_i18n("Flood Fill Layer"));

            KisResourcesSnapshotSP resources =
                new KisResourcesSnapshot(image(), d->viewManager->activeNode(), d->viewManager->canvasResourceProvider()->resourceManager());

            if (event->mimeData()->hasColor()) {
                resources->setFGColorOverride(KoColor(event->mimeData()->colorData().value<QColor>(), image()->colorSpace()));
            } else {
                QByteArray byteData = event->mimeData()->data("krita/x-colorsetentry");
                KisSwatch s = KisSwatch::fromByteArray(byteData);
                resources->setFGColorOverride(s.color());
            }

            KisProcessingVisitorSP visitor =
                new FillProcessingVisitor(resources->image()->projection(),
                                          QPoint(0, 0), // start position
                                          selection(),
                                          resources,
                                          false, // fast mode
                                          false,
                                          true, // fill only selection,
                                          false,
                                          0, // feathering radius
                                          0, // sizemod
                                          80, // threshold,
                                          false, // use unmerged
                                          false // use bg
                                          );

            applicator.applyVisitor(visitor,
                                    KisStrokeJobData::SEQUENTIAL,
                                    KisStrokeJobData::EXCLUSIVE);

            applicator.end();
        }
    }
}

void KisView::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << "KisView::dragMoveEvent";
    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {

        event->accept();
    }

}

KisDocument *KisView::document() const
{
    return d->document;
}

KisView *KisView::replaceBy(KisDocument *document)
{
    KisMainWindow *window = mainWindow();
    QMdiSubWindow *subWindow = d->subWindow;
    delete this;
    return window->newView(document, subWindow);
}

KisMainWindow * KisView::mainWindow() const
{
    return d->viewManager->mainWindow();
}

void KisView::setSubWindow(QMdiSubWindow *subWindow)
{
    d->subWindow = subWindow;
}

QStatusBar * KisView::statusBar() const
{
    KisMainWindow *mw = mainWindow();
    return mw ? mw->statusBar() : 0;
}

void KisView::slotSavingStatusMessage(const QString &text, int timeout, bool isAutoSaving)
{
    QStatusBar *sb = statusBar();
    if (sb) {
        sb->showMessage(text, timeout);
    }

    KisConfig cfg(true);

    if (!sb || sb->isHidden() ||
        (!isAutoSaving && cfg.forceShowSaveMessages()) ||
        (cfg.forceShowAutosaveMessages() && isAutoSaving)) {

        viewManager()->showFloatingMessage(text, QIcon());
    }
}

void KisView::slotClearStatusText()
{
    QStatusBar *sb = statusBar();
    if (sb) {
        sb->clearMessage();
    }
}

QList<QAction*> KisView::createChangeUnitActions(bool addPixelUnit)
{
    UnitActionGroup* unitActions = new UnitActionGroup(d->document, addPixelUnit, this);
    return unitActions->actions();
}

void KisView::closeEvent(QCloseEvent *event)
{
    // Check whether we're the last (user visible) view
    int viewCount = KisPart::instance()->viewCount(document());
    if (viewCount > 1 || !isVisible()) {
        // there are others still, so don't bother the user
        event->accept();
        return;
    }

    if (queryClose()) {
        event->accept();
        return;
    }

    event->ignore();

}

bool KisView::queryClose()
{
    if (!document())
        return true;

    document()->waitForSavingToComplete();

    if (document()->isModified()) {
        QString name;
        name = QFileInfo(document()->path()).fileName();

        if (name.isEmpty())
            name = i18n("Untitled");

        int res = QMessageBox::warning(this,
                                       i18nc("@title:window", "Krita"),
                                       i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);

        switch (res) {
        case QMessageBox::Yes : {
            bool isNative = (document()->mimeType() == document()->nativeFormatMimeType());
            if (!viewManager()->mainWindow()->saveDocument(document(), !isNative, false))
                return false;
            break;
        }
        case QMessageBox::No : {
            KisImageSP image = document()->image();
            image->requestStrokeCancellation();
            viewManager()->blockUntilOperationsFinishedForced(image);

            document()->removeAutoSaveFiles(document()->localFilePath(), document()->isRecovered());
            document()->setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        }
        default : // case QMessageBox::Cancel :
            return false;
        }
    }

    return true;

}

void KisView::slotScreenChanged()
{
    d->zoomManager.updateScreenResolution(this);
}

void KisView::slotThemeChanged(QPalette pal)
{
    this->setPalette(pal);
    for (int i=0; i<this->children().size();i++) {
        QWidget *w = qobject_cast<QWidget*> ( this->children().at(i));
        if (w) {
            w->setPalette(pal);
        }
    }
    if (canvasBase()) {
        canvasBase()->canvasWidget()->setPalette(pal);
    }
    if (canvasController()) {
        canvasController()->setPalette(pal);
    }
}

void KisView::resetImageSizeAndScroll(bool changeCentering,
                                      const QPointF &oldImageStillPoint,
                                      const QPointF &newImageStillPoint)
{
    const KisCoordinatesConverter *converter = d->canvas.coordinatesConverter();

    QPointF oldPreferredCenter = d->canvasController.preferredCenter();

    /**
     * Calculating the still point in old coordinates depending on the
     * parameters given
     */

    QPointF oldStillPoint;

    if (changeCentering) {
        oldStillPoint =
                converter->imageToWidget(oldImageStillPoint) +
                converter->documentOffset();
    } else {
        QSizeF oldDocumentSize = d->canvasController.documentSize();
        oldStillPoint = QPointF(0.5 * oldDocumentSize.width(), 0.5 * oldDocumentSize.height());
    }

    /**
     * Updating the document size
     */

    QSizeF size(image()->width() / image()->xRes(), image()->height() / image()->yRes());
    KoZoomController *zc = d->zoomManager.zoomController();
    zc->setZoom(KoZoomMode::ZOOM_CONSTANT, zc->zoomAction()->effectiveZoom(),
                d->zoomManager.resolutionX(), d->zoomManager.resolutionY());
    zc->setPageSize(size);
    zc->setDocumentSize(size, true);

    /**
     * Calculating the still point in new coordinates depending on the
     * parameters given
     */

    QPointF newStillPoint;

    if (changeCentering) {
        newStillPoint =
                converter->imageToWidget(newImageStillPoint) +
                converter->documentOffset();
    } else {
        QSizeF newDocumentSize = d->canvasController.documentSize();
        newStillPoint = QPointF(0.5 * newDocumentSize.width(), 0.5 * newDocumentSize.height());
    }

    d->canvasController.setPreferredCenter(oldPreferredCenter - oldStillPoint + newStillPoint);
}

void KisView::syncLastActiveNodeToDocument()
{
    KisDocument *doc = document();
    if (doc) {
        doc->setPreActivatedNode(d->currentNode);
    }
}

void KisView::saveViewState(KisPropertiesConfiguration &config) const
{
    config.setProperty("file", d->document->path());
    config.setProperty("window", mainWindow()->windowStateConfig().name());

    if (d->subWindow) {
        config.setProperty("geometry", d->subWindow->saveGeometry().toBase64());
    }

    config.setProperty("zoomMode", (int)zoomController()->zoomMode());
    config.setProperty("zoom", d->canvas.coordinatesConverter()->zoom());

    d->canvasController.saveCanvasState(config);
}

void KisView::restoreViewState(const KisPropertiesConfiguration &config)
{
    if (d->subWindow) {
        QByteArray geometry = QByteArray::fromBase64(config.getString("geometry", "").toLatin1());
        d->subWindow->restoreGeometry(QByteArray::fromBase64(geometry));
    }

    qreal zoom = config.getFloat("zoom", 1.0f);
    int zoomMode = config.getInt("zoomMode", (int)KoZoomMode::ZOOM_PAGE);
    d->zoomManager.zoomController()->setZoom((KoZoomMode::Mode)zoomMode, zoom);
    d->canvasController.restoreCanvasState(config);
}

void KisView::setCurrentNode(KisNodeSP node)
{
    d->currentNode = node;
    d->canvas.slotTrySwitchShapeManager();

    syncLastActiveNodeToDocument();
}

KisNodeSP KisView::currentNode() const
{
    return d->currentNode;
}

KisLayerSP KisView::currentLayer() const
{
    KisNodeSP node;
    KisMaskSP mask = currentMask();
    if (mask) {
        node = mask->parent();
    }
    else {
        node = d->currentNode;
    }
    return qobject_cast<KisLayer*>(node.data());
}

KisMaskSP KisView::currentMask() const
{
    return dynamic_cast<KisMask*>(d->currentNode.data());
}

KisSelectionSP KisView::selection()
{
    KisLayerSP layer = currentLayer();
    if (layer)
        return layer->selection(); // falls through to the global
    // selection, or 0 in the end
    if (image()) {
        return image()->globalSelection();
    }
    return 0;
}

void KisView::slotSoftProofing(bool softProofing)
{
    d->softProofing = softProofing;
    QString message;
    if (canvasBase()->image()->colorSpace()->colorDepthId().id().contains("F"))
    {
        message = i18n("Soft Proofing doesn't work in floating point.");
        viewManager()->showFloatingMessage(message,QIcon());
        return;
    }
    if (softProofing){
        message = i18n("Soft Proofing turned on.");
    } else {
        message = i18n("Soft Proofing turned off.");
    }
    viewManager()->showFloatingMessage(message,QIcon());
    canvasBase()->slotSoftProofing(softProofing);
}

void KisView::slotGamutCheck(bool gamutCheck)
{
    d->gamutCheck = gamutCheck;
    QString message;
    if (canvasBase()->image()->colorSpace()->colorDepthId().id().contains("F"))
    {
        message = i18n("Gamut Warnings don't work in floating point.");
        viewManager()->showFloatingMessage(message,QIcon());
        return;
    }

    if (gamutCheck){
        message = i18n("Gamut Warnings turned on.");
        if (!d->softProofing){
            message += "\n "+i18n("But Soft Proofing is still off.");
        }
    } else {
        message = i18n("Gamut Warnings turned off.");
    }
    viewManager()->showFloatingMessage(message,QIcon());
    canvasBase()->slotGamutCheck(gamutCheck);
}

bool KisView::softProofing()
{
    return d->softProofing;
}

bool KisView::gamutCheck()
{
    return d->gamutCheck;
}

void KisView::slotLoadingFinished()
{
    if (!document()) return;

    /**
     * Cold-start of image size/resolution signals
     */
    slotImageResolutionChanged();

    if (image()->locked()) {
        // If this is the first view on the image, the image will have been locked
        // so unlock it.
        image()->blockSignals(false);
        image()->unlock();
    }

    canvasBase()->initializeImage();

    /**
     * Dirty hack alert
     */
    d->zoomManager.zoomController()->setAspectMode(true);

    if (viewConverter()) {
        viewConverter()->setZoomMode(KoZoomMode::ZOOM_PAGE);
    }
    connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), this, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)));
    connect(image(), SIGNAL(sigProfileChanged(const KoColorProfile*)), this, SIGNAL(sigProfileChanged(const KoColorProfile*)));
    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SIGNAL(sigSizeChanged(QPointF,QPointF)));

    KisNodeSP activeNode = document()->preActivatedNode();

    if (!activeNode) {
        activeNode = image()->rootLayer()->lastChild();
    }

    while (activeNode && !activeNode->inherits("KisLayer")) {
        activeNode = activeNode->prevSibling();
    }

    setCurrentNode(activeNode);
    connect(d->viewManager->mainWindow(), SIGNAL(screenChanged()), SLOT(slotScreenChanged()));
    zoomManager()->updateImageBoundsSnapping();
}

void KisView::slotSavingFinished()
{
    if (d->viewManager && d->viewManager->mainWindow()) {
        d->viewManager->mainWindow()->updateCaption();
    }
}

void KisView::slotImageResolutionChanged()
{
    resetImageSizeAndScroll(false);
    zoomManager()->updateImageBoundsSnapping();
    zoomManager()->updateGuiAfterDocumentSize();

    // update KoUnit value for the document
    if (resourceProvider()) {
        resourceProvider()->resourceManager()->
                setResource(KoCanvasResource::Unit, d->canvas.unit());
    }
}

void KisView::slotImageSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint)
{
    resetImageSizeAndScroll(true, oldStillPoint, newStillPoint);
    zoomManager()->updateImageBoundsSnapping();
    zoomManager()->updateGuiAfterDocumentSize();
}

void KisView::closeView()
{
    d->subWindow->close();
}
