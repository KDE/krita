/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
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

#include <QMessageBox>
#include <QUrl>
#include <QTemporaryFile>
#include <QApplication>
#include <QScreen>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include <QPrintDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QMoveEvent>
#include <QMdiSubWindow>
#include <QFileInfo>
#include <QScreen>

#include <kis_image.h>
#include <kis_node.h>

#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_mask.h>
#include <kis_selection.h>

#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisMainWindow.h"
#include "KisMimeDatabase.h"
#include "KisPart.h"
#include "KisReferenceImagesDecoration.h"
#include "KisRemoteFileFetcher.h"
#include "KisSynchronizedConnection.h"
#include "KisViewManager.h"
#include "input/kis_input_manager.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "kis_file_layer.h"
#include "kis_fill_painter.h"
#include "kis_filter_manager.h"
#include "kis_image_manager.h"
#include "kis_import_catcher.h"
#include "kis_mimedata.h"
#include "kis_node_commands_adapter.h"
#include "kis_node_manager.h"
#include "kis_paint_layer.h"
#include "kis_painting_assistants_decoration.h"
#include "kis_resources_snapshot.h"
#include "kis_selection_manager.h"
#include "kis_shape_controller.h"
#include "kis_signal_compressor.h"
#include "kis_zoom_manager.h"
#include "krita_utils.h"
#include "processing/fill_processing_visitor.h"
#include "widgets/kis_canvas_drop.h"
#include <commands_new/KisMergeLabeledLayersCommand.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/kis_processing_command.h>
#include <commands_new/kis_update_command.h>
#include <kis_command_utils.h>
#include <KisScreenMigrationTracker.h>
#include "kis_memory_statistics_server.h"
#include "kformat.h"


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
        , screenMigrationTracker(_q)
    {
    }

    bool inOperation {false}; //in the middle of an operation (no screen refreshing)?

    QPointer<KisDocument> document; // our KisDocument
    QWidget *tempActiveWidget {nullptr};

    KisKActionCollection* actionCollection {nullptr};
    KisCoordinatesConverter viewConverter;
    KisCanvasController canvasController;
    KisCanvas2 canvas;
    KisZoomManager zoomManager;
    KisViewManager *viewManager {nullptr};
    KisNodeSP currentNode;
    KisPaintingAssistantsDecorationSP paintingAssistantsDecoration;
    KisReferenceImagesDecorationSP referenceImagesDecoration;
    bool isCurrent {false};
    bool showFloatingMessage {true};
    QPointer<KisFloatingMessage> savedFloatingMessage;
    KisSignalCompressor floatingMessageCompressor;
    QMdiSubWindow *subWindow {nullptr};

    bool softProofing {false};
    bool gamutCheck {false};

    KisSynchronizedConnection<KisNodeSP, KisNodeAdditionFlags> addNodeConnection;
    KisSynchronizedConnection<KisNodeSP> removeNodeConnection;

    KisScreenMigrationTracker screenMigrationTracker;

    // Hmm sorry for polluting the private class with such a big inner class.
    // At the beginning it was a little struct :)
    class StatusBarItem : public boost::equality_comparable<StatusBarItem>
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
    d->canvasController.setCanvas(&d->canvas);

    d->zoomManager.setup(d->actionCollection);

    setAcceptDrops(true);

    connect(d->document, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));

    d->referenceImagesDecoration = new KisReferenceImagesDecoration(this, document, /* viewReady = */ false);
    d->canvas.addDecoration(d->referenceImagesDecoration);
    d->referenceImagesDecoration->setVisible(true);

    d->paintingAssistantsDecoration = new KisPaintingAssistantsDecoration(this);
    d->canvas.addDecoration(d->paintingAssistantsDecoration);
    d->paintingAssistantsDecoration->setVisible(true);

    d->showFloatingMessage = cfg.showCanvasMessages();
    slotScreenOrResolutionChanged();

    connect(document, SIGNAL(sigReadWriteChanged(bool)), this, SLOT(slotUpdateDocumentTitle()));
    connect(document, SIGNAL(sigRecoveredChanged(bool)), this, SLOT(slotUpdateDocumentTitle()));
    connect(document, SIGNAL(sigPathChanged(QString)), this, SLOT(slotUpdateDocumentTitle()));
    connect(KisMemoryStatisticsServer::instance(),
            SIGNAL(sigUpdateMemoryStatistics()),
            SLOT(slotUpdateDocumentTitle()));
    connect(document, SIGNAL(modified(bool)), this, SLOT(setWindowModified(bool)));
    slotUpdateDocumentTitle();
    setWindowModified(document->isModified());
}

KisView::~KisView()
{
    if (d->viewManager) {
        if (d->viewManager->filterManager()->isStrokeRunning()) {
            d->viewManager->filterManager()->cancelDialog();
        }

        d->viewManager->mainWindow()->notifyChildViewDestroyed(this);
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
    KisShapeController* shapeController = dynamic_cast<KisShapeController*>(d->document->shapeController());
    shapeController->setInitialShapeForCanvas(&d->canvas);

    if (d->viewManager && d->viewManager->nodeManager()) {
        d->viewManager->nodeManager()->nodesUpdated();
    }

    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SLOT(slotImageSizeChanged(QPointF,QPointF)));
    connect(image(), SIGNAL(sigResolutionChanged(double,double)), this, SLOT(slotImageResolutionChanged()));

    d->addNodeConnection.connectSync(image(), &KisImage::sigNodeAddedAsync,
                                     this, &KisView::slotContinueAddNode);

    // executed in a context of an image thread
    connect(image(), SIGNAL(sigRemoveNodeAsync(KisNodeSP)),
            SLOT(slotImageNodeRemoved(KisNodeSP)),
            Qt::DirectConnection);

    d->removeNodeConnection.connectOutputSlot(this, &KisView::slotContinueRemoveNode);

    d->viewManager->updateGUI();

    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");
}

KisViewManager* KisView::viewManager() const
{
    return d->viewManager;
}

void KisView::slotContinueAddNode(KisNodeSP newActiveNode, KisNodeAdditionFlags flags)
{
    Q_UNUSED(flags)

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
    d->removeNodeConnection.start(KritaUtils::nearestNodeAfterRemoval(node));
}

void KisView::slotContinueRemoveNode(KisNodeSP newActiveNode)
{
    if (!d->isCurrent) {
        d->currentNode = newActiveNode;
    }
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
    dbgUI << Q_FUNC_INFO
          << "Formats: " << event->mimeData()->formats()
          << "Urls: " << event->mimeData()->urls()
          << "Has images: " << event->mimeData()->hasImage();
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node-internal-pointer")
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

    QPoint imgCursorPos = canvasBase()->coordinatesConverter()->widgetToImage(event->pos()).toPoint();
    QRect imageBounds = kisimage->bounds();
    boost::optional<QPoint> forcedCenter;

    if (event->keyboardModifiers() & Qt::ShiftModifier && imageBounds.contains(imgCursorPos)) {
        forcedCenter = imgCursorPos;
    }

    dbgUI << Q_FUNC_INFO;
    dbgUI << "\t Formats: " << event->mimeData()->formats();
    dbgUI << "\t Urls: " << event->mimeData()->urls();
    dbgUI << "\t Has images: " << event->mimeData()->hasImage();

    if (event->mimeData()->hasFormat("application/x-krita-node-internal-pointer")) {
        KisShapeController *kritaShapeController =
                dynamic_cast<KisShapeController*>(d->document->shapeController());

        bool copyNode = true;
        QList<KisNodeSP> nodes;

        if (forcedCenter) {
            nodes = KisMimeData::loadNodesFastAndRecenter(*forcedCenter, event->mimeData(), kisimage, kritaShapeController, copyNode);
        } else {
            nodes = KisMimeData::loadNodesFast(event->mimeData(), kisimage, kritaShapeController, copyNode);
        }

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
    } else if (event->mimeData()->hasImage() || event->mimeData()->hasUrls()) {
        const auto *data = event->mimeData();

        KisCanvasDrop dlgAction;

        const auto callPos = QCursor::pos();

        const KisCanvasDrop::Action action = dlgAction.dropAs(*data, callPos);

        if (action == KisCanvasDrop::INSERT_AS_NEW_LAYER) {
            const QPair<bool, KisClipboard::PasteFormatBehaviour> source =
                KisClipboard::instance()->askUserForSource(data);

            if (!source.first) {
                dbgUI << "Paste event cancelled";
                return;
            }

            if (source.second != KisClipboard::PASTE_FORMAT_CLIP) {
                const QList<QUrl> &urls = data->urls();
                const auto url = std::find_if(
                    urls.constBegin(),
                    urls.constEnd(),
                    [&](const QUrl &url) {
                        if (source.second
                            == KisClipboard::PASTE_FORMAT_DOWNLOAD) {
                            return !url.isLocalFile();
                        } else if (source.second
                                   == KisClipboard::PASTE_FORMAT_LOCAL) {
                            return url.isLocalFile();
                        } else {
                            return false;
                        }
                    });

                if (url != urls.constEnd()) {
                    QScopedPointer<QTemporaryFile> tmp(new QTemporaryFile());
                    tmp->setAutoRemove(true);

                    const QUrl localUrl = [&]() -> QUrl {
                        if (!url->isLocalFile()) {
                            // download the file and substitute the url
                            KisRemoteFileFetcher fetcher;
                            tmp->setFileName(url->fileName());

                            if (!fetcher.fetchFile(*url, tmp.data())) {
                                warnUI << "Fetching" << *url << "failed";
                                return {};
                            }
                            return QUrl::fromLocalFile(tmp->fileName());
                        }
                        return *url;
                    }();

                    if (localUrl.isLocalFile()) {
                        this->mainWindow()
                            ->viewManager()
                            ->imageManager()
                            ->importImage(localUrl);
                        this->activateWindow();
                        return;
                    }
                }
            }

            KisPaintDeviceSP clip =
                KisClipboard::instance()->clipFromBoardContents(data,
                                                                QRect(),
                                                                true,
                                                                -1,
                                                                false,
                                                                source);
            if (clip) {
                const auto pos = this->viewConverter()
                                     ->imageToDocument(imgCursorPos)
                                     .toPoint();

                clip->moveTo(pos.x(), pos.y());

                KisImportCatcher::adaptClipToImageColorSpace(clip,
                                                             this->image());

                KisPaintLayerSP layer = new KisPaintLayer(
                    this->image(),
                    this->image()->nextLayerName() + " " + i18n("(pasted)"),
                    OPACITY_OPAQUE_U8,
                    clip);
                KisNodeCommandsAdapter adapter(
                    this->mainWindow()->viewManager());
                adapter.addNode(
                    layer,
                    this->mainWindow()->viewManager()->activeNode()->parent(),
                    this->mainWindow()->viewManager()->activeNode());
                this->activateWindow();
                return;
            }
        } else if (action == KisCanvasDrop::INSERT_AS_REFERENCE_IMAGE) {
            KisPaintDeviceSP clip =
                KisClipboard::instance()->clipFromMimeData(data, QRect(), true);
            if (clip) {
                KisImportCatcher::adaptClipToImageColorSpace(clip,
                                                             this->image());

                auto *reference =
                    KisReferenceImage::fromPaintDevice(clip,
                                                       *this->viewConverter(),
                                                       this);

                if (reference) {
                    if (data->hasUrls()) {
                        const auto &urls = data->urls();
                        const auto url = std::find_if(urls.constBegin(), urls.constEnd(), std::mem_fn(&QUrl::isLocalFile));
                        if (url != urls.constEnd()) {
                            reference->setFilename((*url).toLocalFile());
                        }
                    }
                    const auto pos = this->canvasBase()
                                         ->coordinatesConverter()
                                         ->widgetToImage(event->pos());
                    reference->setPosition(
                        (*this->viewConverter()).imageToDocument(pos));
                    this->canvasBase()
                        ->referenceImagesDecoration()
                        ->addReferenceImage(reference);
                    KoToolManager::instance()->switchToolRequested(
                        "ToolReferenceImages");
                    return;
                }
            }
        } else if (action != KisCanvasDrop::NONE) {
            // multiple URLs detected OR about to open a document

            for (QUrl url : data->urls()) { // do copy it
                QScopedPointer<QTemporaryFile> tmp(new QTemporaryFile());
                tmp->setAutoRemove(true);

                if (!url.isLocalFile()) {
                    // download the file and substitute the url
                    KisRemoteFileFetcher fetcher;
                    tmp->setFileName(url.fileName());

                    if (!fetcher.fetchFile(url, tmp.data())) {
                        qWarning() << "Fetching" << url << "failed";
                        continue;
                    }
                    url = QUrl::fromLocalFile(tmp->fileName());
                }

                if (url.isLocalFile()) {
                    if (action == KisCanvasDrop::INSERT_MANY_LAYERS) {
                        this->mainWindow()
                            ->viewManager()
                            ->imageManager()
                            ->importImage(url);
                        this->activateWindow();
                    } else if (action == KisCanvasDrop::INSERT_MANY_FILE_LAYERS
                               || action
                                   == KisCanvasDrop::INSERT_AS_NEW_FILE_LAYER) {
                        KisNodeCommandsAdapter adapter(
                            this->mainWindow()->viewManager());
                        QFileInfo fileInfo(url.toLocalFile());

                        QString type =
                            KisMimeDatabase::mimeTypeForFile(url.toLocalFile());
                        QStringList mimes =
                            KisImportExportManager::supportedMimeTypes(
                                KisImportExportManager::Import);

                        if (!mimes.contains(type)) {
                            QString msg =
                                KisImportExportErrorCode(
                                    ImportExportCodes::FileFormatNotSupported)
                                    .errorMessage();
                            QMessageBox::warning(
                                this,
                                i18nc("@title:window", "Krita"),
                                i18n("Could not open %2.\nReason: %1.",
                                     msg,
                                     url.toDisplayString()));
                            continue;
                        }

                        KisFileLayer *fileLayer =
                            new KisFileLayer(this->image(),
                                             "",
                                             url.toLocalFile(),
                                             KisFileLayer::None,
                                             "Bicubic",
                                             fileInfo.fileName(),
                                             OPACITY_OPAQUE_U8);

                        KisLayerSP above =
                            this->mainWindow()->viewManager()->activeLayer();
                        KisNodeSP parent = above ? above->parent()
                                                 : this->mainWindow()
                                                       ->viewManager()
                                                       ->image()
                                                       ->root();

                        adapter.addNode(fileLayer, parent, above);
                    } else if (action == KisCanvasDrop::OPEN_IN_NEW_DOCUMENT
                               || action
                                   == KisCanvasDrop::OPEN_MANY_DOCUMENTS) {
                        if (this->mainWindow()) {
                            this->mainWindow()->openDocument(
                                url.toLocalFile(),
                                KisMainWindow::None);
                        }
                    } else if (action
                                   == KisCanvasDrop::INSERT_AS_REFERENCE_IMAGES
                               || action
                                   == KisCanvasDrop::
                                       INSERT_AS_REFERENCE_IMAGE) {
                        auto *reference =
                            KisReferenceImage::fromFile(url.toLocalFile(),
                                                        *this->viewConverter(),
                                                        this);

                        if (reference) {
                            const auto pos = this->canvasBase()
                                                 ->coordinatesConverter()
                                                 ->widgetToImage(event->pos());
                            reference->setPosition(
                                (*this->viewConverter()).imageToDocument(pos));
                            this->canvasBase()
                                ->referenceImagesDecoration()
                                ->addReferenceImage(reference);

                            KoToolManager::instance()->switchToolRequested(
                                "ToolReferenceImages");
                        }
                    }
                }
            }
        }
    } else if (event->mimeData()->hasColor() || event->mimeData()->hasFormat("krita/x-colorsetentry")) {
        if (!image()) {
            return;
        }

        // Cannot fill on non-painting layers (vector layer, clone layer, file layer, group layer)
        if (d->viewManager->activeNode().isNull() ||
            d->viewManager->activeNode()->inherits("KisShapeLayer") ||
            d->viewManager->activeNode()->inherits("KisCloneLayer") ||
            !d->viewManager->activeDevice()) {
            showFloatingMessage(i18n("You cannot drag and drop colors on the selected layer type."), QIcon());
            return;
        }

        // Cannot fill if the layer is not editable
        if (!d->viewManager->activeNode()->isEditable()) {
            QString message;
            if (!d->viewManager->activeNode()->visible() && d->viewManager->activeNode()->userLocked()) {
                message = i18n("Layer is locked and invisible.");
            } else if (d->viewManager->activeNode()->userLocked()) {
                message = i18n("Layer is locked.");
            } else if(!d->viewManager->activeNode()->visible()) {
                message = i18n("Layer is invisible.");
            }
            showFloatingMessage(message, KisIconUtils::loadIcon("object-locked"));
            return;
        }

        // The cursor is outside the image
        if (!image()->wrapAroundModePermitted() && !image()->bounds().contains(imgCursorPos)) {
            return;
        }
            
        KisStrokeStrategyUndoCommandBased *strategy =
                new KisStrokeStrategyUndoCommandBased(
                    kundo2_i18n("Flood Fill Layer"), false, image().data()
                );
        strategy->setSupportsWrapAroundMode(true);
        KisStrokeId fillStrokeId = image()->startStroke(strategy);
        KIS_SAFE_ASSERT_RECOVER_RETURN(fillStrokeId);

        QSharedPointer<QRect> dirtyRect = QSharedPointer<QRect>(new QRect);

        KisResourcesSnapshotSP resources =
            new KisResourcesSnapshot(image(), d->viewManager->activeNode(), d->viewManager->canvasResourceProvider()->resourceManager());

        if (event->mimeData()->hasColor()) {
            resources->setFGColorOverride(KoColor(event->mimeData()->colorData().value<QColor>(), image()->colorSpace()));
        } else {
            QByteArray byteData = event->mimeData()->data("krita/x-colorsetentry");
            KisSwatch s = KisSwatch::fromByteArray(byteData);
            resources->setFGColorOverride(s.color());
        }

        // Use same options as the fill tool
        KConfigGroup configGroup = KSharedConfig::openConfig()->group("KritaFill/KisToolFill");
        QString fillMode = configGroup.readEntry<QString>("whatToFill", "");
        if (fillMode.isEmpty()) {
            if (configGroup.readEntry<bool>("fillSelection", false)) {
                fillMode = "fillSelection";
            } else {
                fillMode = "fillContiguousRegion";
            }
        }
        const bool useCustomBlendingOptions = configGroup.readEntry<bool>("useCustomBlendingOptions", false);
        const qreal customOpacity =
            qBound(0, configGroup.readEntry<int>("customOpacity", 100), 100) / 100.0;
        QString customCompositeOp = configGroup.readEntry<QString>("customCompositeOp", COMPOSITE_OVER);
        if (KoCompositeOpRegistry::instance().getKoID(customCompositeOp).id().isNull()) {
            customCompositeOp = COMPOSITE_OVER;
        }
            
        if (event->keyboardModifiers() == Qt::ShiftModifier) {
            if (fillMode == "fillSimilarRegions") {
                fillMode = "fillSelection";
            } else {
                fillMode = "fillSimilarRegions";
            }
        } else if (event->keyboardModifiers() == Qt::AltModifier) {
            if (fillMode == "fillContiguousRegion") {
                fillMode = "fillSelection";
            } else {
                fillMode = "fillContiguousRegion";
            }
        }

        if (fillMode == "fillSelection") {
            FillProcessingVisitor *visitor =  new FillProcessingVisitor(nullptr,
                                                                        selection(),
                                                                        resources);
            visitor->setSeedPoint(imgCursorPos);
            visitor->setSelectionOnly(true);
            visitor->setUseCustomBlendingOptions(useCustomBlendingOptions);
            if (useCustomBlendingOptions) {
                visitor->setCustomOpacity(customOpacity);
                visitor->setCustomCompositeOp(customCompositeOp);
            }
            visitor->setOutDirtyRect(dirtyRect);

            image()->addJob(
                fillStrokeId,
                new KisStrokeStrategyUndoCommandBased::Data(
                    KUndo2CommandSP(new KisProcessingCommand(visitor, d->viewManager->activeNode())),
                    false,
                    KisStrokeJobData::SEQUENTIAL,
                    KisStrokeJobData::EXCLUSIVE
                )
            );
        } else {
            const int threshold = configGroup.readEntry("thresholdAmount", 8);
            const int opacitySpread = configGroup.readEntry("opacitySpread", 100);
            const bool antiAlias = configGroup.readEntry("antiAlias", true);
            const int grow = configGroup.readEntry("growSelection", 0);
            const bool stopGrowingAtDarkestPixel = configGroup.readEntry<bool>("stopGrowingAtDarkestPixel", false);
            const int feather = configGroup.readEntry("featherAmount", 0);
            const int closeGap = configGroup.readEntry("closeGapAmount", 0);
            QString sampleLayersMode = configGroup.readEntry("sampleLayersMode", "");
            if (sampleLayersMode.isEmpty()) {
                if (configGroup.readEntry("sampleMerged", false)) {
                    sampleLayersMode = "allLayers";
                } else {
                    sampleLayersMode = "currentLayer";
                }
            }
            QList<int> colorLabels;
            {
                const QStringList colorLabelsStr = configGroup.readEntry<QString>("colorLabels", "").split(',', Qt::SkipEmptyParts);

                for (const QString &colorLabelStr : colorLabelsStr) {
                    bool ok;
                    const int colorLabel = colorLabelStr.toInt(&ok);
                    if (ok) {
                        colorLabels << colorLabel;
                    }
                }
            }
            
            KisPaintDeviceSP referencePaintDevice = nullptr;
            if (sampleLayersMode == "allLayers") {
                referencePaintDevice = image()->projection();
            } else if (sampleLayersMode == "currentLayer") {
                referencePaintDevice = d->viewManager->activeNode()->paintDevice();
            } else if (sampleLayersMode == "colorLabeledLayers") {
                referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
                image()->addJob(
                    fillStrokeId,
                    new KisStrokeStrategyUndoCommandBased::Data(
                        KUndo2CommandSP(new KisMergeLabeledLayersCommand(image(),
                                                                         referencePaintDevice,
                                                                         colorLabels,
                                                                         KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled)),
                        false,
                        KisStrokeJobData::SEQUENTIAL,
                        KisStrokeJobData::EXCLUSIVE
                    )
                );
            }

            QSharedPointer<KoColor> referenceColor(new KoColor);
            if (sampleLayersMode == "colorLabeledLayers") {
                // We need to obtain the reference color from the reference paint
                // device, but it is produced in a stroke, so we must get the color
                // after the device is ready. So we get it in the stroke
                image()->addJob(
                    fillStrokeId,
                    new KisStrokeStrategyUndoCommandBased::Data(
                        KUndo2CommandSP(new KisCommandUtils::LambdaCommand(
                            [referenceColor, referencePaintDevice, imgCursorPos]() -> KUndo2Command*
                            {
                                *referenceColor = referencePaintDevice->pixel(imgCursorPos);
                                return 0;
                            }
                        )),
                        false,
                        KisStrokeJobData::SEQUENTIAL,
                        KisStrokeJobData::EXCLUSIVE
                    )
                );
            } else {
                // Here the reference device is already ready, so we obtain the
                // reference color directly
                *referenceColor = referencePaintDevice->pixel(imgCursorPos);
            }

            if (fillMode == "fillContiguousRegion") {
                const KisFillPainter::RegionFillingMode regionFillingMode =
                    configGroup.readEntry("contiguousFillMode", "") == "boundaryFill"
                    ? KisFillPainter::RegionFillingMode_BoundaryFill
                    : KisFillPainter::RegionFillingMode_FloodFill;
                KoColor regionFillingBoundaryColor;
                if (regionFillingMode == KisFillPainter::RegionFillingMode_BoundaryFill) {
                    const QString xmlColor = configGroup.readEntry("contiguousFillBoundaryColor", QString());
                    QDomDocument doc;
                    if (doc.setContent(xmlColor)) {
                        QDomElement e = doc.documentElement().firstChild().toElement();
                        QString channelDepthID = doc.documentElement().attribute("channeldepth", Integer16BitsColorDepthID.id());
                        bool ok;
                        if (e.hasAttribute("space") || e.tagName().toLower() == "srgb") {
                            regionFillingBoundaryColor = KoColor::fromXML(e, channelDepthID, &ok);
                        } else if (doc.documentElement().hasAttribute("space") || doc.documentElement().tagName().toLower() == "srgb"){
                            regionFillingBoundaryColor = KoColor::fromXML(doc.documentElement(), channelDepthID, &ok);
                        }
                    }
                }
                const bool useSelectionAsBoundary = configGroup.readEntry("useSelectionAsBoundary", false);
                const bool blendingOptionsAreNoOp = useCustomBlendingOptions
                                                    ? (qFuzzyCompare(customOpacity, OPACITY_OPAQUE_F) &&
                                                       customCompositeOp == COMPOSITE_OVER)
                                                    : (qFuzzyCompare(resources->opacity(), OPACITY_OPAQUE_F) &&
                                                       resources->compositeOpId() == COMPOSITE_OVER);
                const bool useFastMode = !resources->activeSelection() &&
                                         blendingOptionsAreNoOp &&
                                         opacitySpread == 100 &&
                                         useSelectionAsBoundary == false &&
                                         !antiAlias && grow == 0 && feather == 0 &&
                                         closeGap == 0 &&
                                         sampleLayersMode == "currentLayer";

                FillProcessingVisitor *visitor = new FillProcessingVisitor(referencePaintDevice,
                                                                           selection(),
                                                                           resources);
                visitor->setSeedPoint(imgCursorPos);
                visitor->setUseFastMode(useFastMode);
                visitor->setUseSelectionAsBoundary(useSelectionAsBoundary);
                visitor->setFeather(feather);
                visitor->setSizeMod(grow);
                visitor->setStopGrowingAtDarkestPixel(stopGrowingAtDarkestPixel);
                visitor->setRegionFillingMode(regionFillingMode);
                if (regionFillingMode == KisFillPainter::RegionFillingMode_BoundaryFill) {
                    visitor->setRegionFillingBoundaryColor(regionFillingBoundaryColor);
                }
                visitor->setFillThreshold(threshold);
                visitor->setOpacitySpread(opacitySpread);
                visitor->setCloseGap(closeGap);
                visitor->setAntiAlias(antiAlias);
                visitor->setUseCustomBlendingOptions(useCustomBlendingOptions);
                if (useCustomBlendingOptions) {
                    visitor->setCustomOpacity(customOpacity);
                    visitor->setCustomCompositeOp(customCompositeOp);
                }
                visitor->setOutDirtyRect(dirtyRect);
                
                image()->addJob(
                    fillStrokeId,
                    new KisStrokeStrategyUndoCommandBased::Data(
                        KUndo2CommandSP(new KisProcessingCommand(visitor, d->viewManager->activeNode())),
                        false,
                        KisStrokeJobData::SEQUENTIAL,
                        KisStrokeJobData::EXCLUSIVE
                    )
                );
            } else {
                KisSelectionSP fillMask = new KisSelection;
                QSharedPointer<KisProcessingVisitor::ProgressHelper>
                    progressHelper(new KisProcessingVisitor::ProgressHelper(currentNode()));

                {
                    KisSelectionSP selection = this->selection();
                    KisFillPainter painter;
                    QRect bounds = image()->bounds();
                    if (selection) {
                        bounds = bounds.intersected(selection->projection()->selectedRect());
                    }

                    painter.setFillThreshold(threshold);
                    painter.setOpacitySpread(opacitySpread);
                    painter.setAntiAlias(antiAlias);
                    painter.setSizemod(grow);
                    painter.setStopGrowingAtDarkestPixel(stopGrowingAtDarkestPixel);
                    painter.setFeather(feather);

                    QVector<KisStrokeJobData*> jobs =
                        painter.createSimilarColorsSelectionJobs(
                            fillMask->pixelSelection(), referenceColor, referencePaintDevice,
                            bounds, selection ? selection->projection() : nullptr, progressHelper
                        );

                    for (KisStrokeJobData *job : jobs) {
                        image()->addJob(fillStrokeId, job);
                    }
                }

                {
                    FillProcessingVisitor *visitor =  new FillProcessingVisitor(nullptr,
                                                                                fillMask,
                                                                                resources);

                    visitor->setSeedPoint(imgCursorPos);
                    visitor->setSelectionOnly(true);
                    visitor->setProgressHelper(progressHelper);
                    visitor->setOutDirtyRect(dirtyRect);

                    image()->addJob(
                        fillStrokeId,
                        new KisStrokeStrategyUndoCommandBased::Data(
                            KUndo2CommandSP(new KisProcessingCommand(visitor, currentNode())),
                            false,
                            KisStrokeJobData::SEQUENTIAL,
                            KisStrokeJobData::EXCLUSIVE
                        )
                    );
                }
            }
        }

        image()->addJob(
            fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisUpdateCommand(d->viewManager->activeNode(), dirtyRect, image().data())),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );

        image()->endStroke(fillStrokeId);
    }
}

void KisView::dragMoveEvent(QDragMoveEvent *event)
{
    dbgUI << Q_FUNC_INFO
          << "Formats: " << event->mimeData()->formats()
          << "Urls: " << event->mimeData()->urls()
          << "Has images: " << event->mimeData()->hasImage();
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node-internal-pointer")
            || event->mimeData()->hasFormat("krita/x-colorsetentry")
            || event->mimeData()->hasColor()) {
        return event->accept();
    }

    return event->ignore();
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
    // Check whether we're the last view
    int viewCount = KisPart::instance()->viewCount(document());
    if (viewCount > 1) {
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

void KisView::slotMigratedToScreen(QScreen *screen)
{
    d->canvas.slotScreenChanged(screen);
}

void KisView::slotScreenOrResolutionChanged()
{
    /**
     * slotScreenOrResolutionChanged() is guaranteed to come after
     * slotMigratedToScreen() when a migration happens
     */
    d->canvasController.updateScreenResolution(this);

    if (d->canvas.resourceManager() && d->screenMigrationTracker.currentScreen()) {
        int penWidth = qRound(d->screenMigrationTracker.currentScreen()->devicePixelRatio());
        d->canvas.resourceManager()->setDecorationThickness(qMax(penWidth, 1));
    }
}

QScreen* KisView::currentScreen() const
{
    return d->screenMigrationTracker.currentScreen();
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

void KisView::slotUpdateDocumentTitle()
{
    QString title = d->document->caption();

    if (!d->document->isReadWrite()) {
        title += " " + i18n("Write Protected");
    }

    if (d->document->isRecovered()) {
        title += " " + i18n("Recovered");
    }

    // show the file size for the document
    KisMemoryStatisticsServer::Statistics fileSizeStats = KisMemoryStatisticsServer::instance()->fetchMemoryStatistics(d->document->image());

    if (fileSizeStats.imageSize) {
        title += QString(" (").append( KFormat().formatByteSize(qreal(fileSizeStats.imageSize))).append( ") ");
    }

    title += "[*]";

    this->setWindowTitle(title);
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

    const KoZoomState zoomState = d->canvasController.zoomState();

    config.setProperty("zoomMode", zoomState.mode);
    config.setProperty("zoom", zoomState.zoom);

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
    d->canvasController.setZoom((KoZoomMode::Mode)zoomMode, zoom);
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
    canvasBase()->slotSoftProofing();
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
    canvasBase()->slotGamutCheck();
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

    connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), this, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)));
    connect(image(), SIGNAL(sigProfileChanged(const KoColorProfile*)), this, SIGNAL(sigProfileChanged(const KoColorProfile*)));
    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SIGNAL(sigSizeChanged(QPointF,QPointF)));

    connect(&d->screenMigrationTracker, SIGNAL(sigScreenChanged(QScreen*)), this, SLOT(slotMigratedToScreen(QScreen*)));
    connect(&d->screenMigrationTracker, SIGNAL(sigScreenOrResolutionChanged(QScreen*)), this, SLOT(slotScreenOrResolutionChanged()));
    zoomManager()->updateImageBoundsSnapping();
}

void KisView::slotImageResolutionChanged()
{
    d->canvasController.syncOnImageResolutionChange();
    d->zoomManager.syncOnImageResolutionChange();
    zoomManager()->updateImageBoundsSnapping();
}

void KisView::slotImageSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint)
{
    d->canvasController.syncOnImageSizeChange(oldStillPoint, newStillPoint);
    zoomManager()->updateImageBoundsSnapping();
}

void KisView::closeView()
{
    d->subWindow->close();
}
