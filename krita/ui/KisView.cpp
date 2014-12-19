/*
 * Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisView.h"

#include "KisView_p.h"

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>
#include <KoDocumentInfo.h>
#include "KoDocumentInfo.h"
#include "KoPageLayout.h"
#include <KoToolManager.h>

#include <KoIcon.h>

#include <kactioncollection.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <kselectaction.h>
#include <kconfiggroup.h>
#include <kdeprintdialog.h>
#include <kmenu.h>
#include <kactioncollection.h>
#include <kmessagebox.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>
#include <QList>
#include <QPrintDialog>
#include <QToolBar>
#include <QUrl>

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
#include "KisPrintJob.h"
#include "kis_shape_controller.h"
#include "kis_tool_freehand.h"
#include "KisUndoStackAction.h"
#include "KisViewManager.h"
#include "kis_zoom_manager.h"
#include "kis_composite_progress_proxy.h"
#include "kis_statusbar.h"
#include "kis_painting_assistants_decoration.h"
#include "kis_progress_widget.h"

#include "krita/gemini/ViewModeSwitchEvent.h"


//static
QString KisView::newObjectName()
{
    static int s_viewIFNumber = 0;
    QString name; name.setNum(s_viewIFNumber++); name.prepend("view_");
    return name;
}

bool KisView::s_firstView = true;

class KisView::Private
{
public:
    Private()
        : undo(0)
        , redo(0)
        , viewConverter(0)
        , canvasController(0)
        , canvas(0)
        , zoomManager(0)
        , viewManager(0)
        , mirrorAxis(0)
        , actionCollection(0)
        , paintingAssistantsDecoration(0)
    {
        tempActiveWidget = 0;
        documentDeleted = false;
    }

    ~Private() {
        if (canvasController) {
            KoToolManager::instance()->removeCanvasController(canvasController);
        }

        delete zoomManager;
        delete canvasController;
        delete canvas;
        delete viewConverter;
        delete mirrorAxis;
    }

    KisUndoStackAction *undo;
    KisUndoStackAction *redo;

    class StatusBarItem;

    QList<StatusBarItem> statusBarItems; // Our statusbar items
    bool inOperation; //in the middle of an operation (no screen refreshing)?

    QPointer<KisDocument> document; // our KisDocument
    QWidget *tempActiveWidget;
    bool documentDeleted; // true when document gets deleted [can't use document==0
    // since this only happens in ~QObject, and views
    // get deleted by ~KisDocument].

    KisCoordinatesConverter *viewConverter;
    KisCanvasController *canvasController;
    KisCanvas2 *canvas;
    KisZoomManager *zoomManager;
    KisViewManager *viewManager;
    KisNodeSP currentNode;
    KisMirrorAxis* mirrorAxis;
    KActionCollection* actionCollection;
    KisPaintingAssistantsDecoration *paintingAssistantsDecoration;

    // Hmm sorry for polluting the private class with such a big inner class.
    // At the beginning it was a little struct :)
    class StatusBarItem
    {
    public:
        StatusBarItem() // for QValueList
            : m_widget(0),
              m_connected(false),
              m_hidden(false) {}

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

        void ensureItemShown(KStatusBar * sb) {
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
        void ensureItemHidden(KStatusBar * sb) {
            if (m_connected) {
                m_hidden = m_widget->isHidden();
                sb->removeWidget(m_widget);
                m_widget->hide();
                m_connected = false;
            }
        }
    private:
        QWidget * m_widget;
        int m_stretch;
        bool m_permanent;
        bool m_connected;
        bool m_hidden;

    };

};

KisView::KisView(KisDocument *document, KoCanvasResourceManager *resourceManager, KActionCollection *actionCollection, QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    Q_ASSERT(document);

    setObjectName(newObjectName());

    d->document = document;
    d->actionCollection = actionCollection;

    setFocusPolicy(Qt::StrongFocus);

    d->undo = new KisUndoStackAction(d->document->undoStack(), KisUndoStackAction::UNDO);
    d->redo = new KisUndoStackAction(d->document->undoStack(), KisUndoStackAction::RED0);

    KStatusBar * sb = statusBar();
    if (sb) { // No statusbar in e.g. konqueror
        connect(d->document, SIGNAL(statusBarMessage(const QString&)),
                this, SLOT(slotActionStatusText(const QString&)));
        connect(d->document, SIGNAL(clearStatusBarMessage()),
                this, SLOT(slotClearStatusText()));
    }

    d->viewConverter = new KisCoordinatesConverter();

    KisConfig cfg;

    d->canvasController = new KisCanvasController(this, d->actionCollection);

    d->canvasController->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->canvasController->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->canvasController->setDrawShadow(false);
    d->canvasController->setCanvasMode(KoCanvasController::Infinite);
    d->canvasController->setVastScrolling(cfg.vastScrolling());

    KConfigGroup grp(KGlobal::config(), "krita/crashprevention");
    if (grp.readEntry("CreatingCanvas", false)) {
        cfg.setUseOpenGL(false);
    }
    if (cfg.canvasState() == "OPENGL_FAILED") {
        cfg.setUseOpenGL(false);
    }
    grp.writeEntry("CreatingCanvas", true);
    grp.sync();
    d->canvas = new KisCanvas2(d->viewConverter, resourceManager, this, document->shapeController());

    grp.writeEntry("CreatingCanvas", false);
    grp.sync();

    d->canvasController->setCanvas(d->canvas);

    Q_ASSERT(d->canvasController);

    d->zoomManager = new KisZoomManager(this, d->viewConverter, d->canvasController);
    d->zoomManager->setup(d->actionCollection);

    connect(d->canvasController, SIGNAL(documentSizeChanged()), d->zoomManager, SLOT(slotScrollAreaSizeChanged()));
    setAcceptDrops(true);

    connect(d->document, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    connect(d->document, SIGNAL(sigSavingFinished()), this, SLOT(slotSavingFinished()));

    d->paintingAssistantsDecoration = new KisPaintingAssistantsDecoration(this);
    d->canvas->addDecoration(d->paintingAssistantsDecoration);

}

KisView::~KisView()
{
    KisPart::instance()->removeView(this);
    delete d;
}

void KisView::setViewManager(KisViewManager *view)
{
    d->viewManager = view;

    connect(canvasController(), SIGNAL(toolOptionWidgetsChanged(QList<QPointer<QWidget> >)), d->viewManager->mainWindow(), SLOT(newOptionWidgets(QList<QPointer<QWidget> >)));

    KoToolManager::instance()->addController(d->canvasController);
    KoToolManager::instance()->registerTools(d->actionCollection, d->canvasController);
    dynamic_cast<KisShapeController*>(d->document->shapeController())->setInitialShapeForCanvas(d->canvas);
    KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");

    if (s_firstView) {
        // Restore the tool shortcuts from the config file
        if(qApp->applicationName() == QLatin1String("kritagemini")) {
            KConfigGroup group(KGlobal::config(), "krita/shortcuts");
            foreach(KActionCollection *collection, KActionCollection::allCollections()) {
                const QObject* obj = dynamic_cast<const QObject*>(collection->parentGUIClient());
                if(obj && qobject_cast<const KisViewManager*>(obj) && !obj->objectName().startsWith("view_0"))
                    break;
                collection->setConfigGroup("krita/shortcuts");
                collection->readSettings(&group);
            }
        }
        else {
            KConfigGroup group(KGlobal::config(), "krita/shortcuts");
            foreach(KActionCollection *collection, KActionCollection::allCollections()) {
                collection->setConfigGroup("krita/shortcuts");
                collection->readSettings(&group);
            }
        }
        s_firstView = false;
    }


    if (resourceProvider()) {
        resourceProvider()->slotImageSizeChanged();
    }

    if (d->viewManager && d->viewManager->nodeManager()) {
        d->viewManager->nodeManager()->nodesUpdated();
    }

    connect(image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), resourceProvider(), SLOT(slotImageSizeChanged()));
    connect(image(), SIGNAL(sigResolutionChanged(double,double)), resourceProvider(), SLOT(slotOnScreenResolutionChanged()));
    connect(image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), this, SLOT(slotImageSizeChanged(const QPointF&, const QPointF&)));
    connect(image(), SIGNAL(sigResolutionChanged(double,double)), this, SLOT(slotImageResolutionChanged()));
    connect(image(), SIGNAL(sigNodeChanged(KisNodeSP)), d->viewManager, SLOT(updateGUI()));

    connect(zoomManager()->zoomController(), SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)), resourceProvider(), SLOT(slotOnScreenResolutionChanged()));

    /*
     * WARNING: Currently we access the global progress bar in two ways:
     * connecting to composite progress proxy (strokes) and creating
     * progress updaters. The latter way should be deprecated in favour
     * of displaying the status of the global strokes queue
     */
    image()->compositeProgressProxy()->addProxy(d->viewManager->statusBar()->progress()->progressProxy());
    connect(d->viewManager->statusBar()->progress(), SIGNAL(sigCancellationRequested()), image(), SLOT(requestStrokeCancellation()));

    d->viewManager->updateGUI();
}

KisViewManager* KisView::viewManager() const
{
    return d->viewManager;
}


KAction *KisView::undoAction() const
{
    return d->undo;
}

KAction *KisView::redoAction() const
{
    return d->redo;
}

KoZoomController *KisView::zoomController() const
{
    return d->zoomManager->zoomController();
}

KisZoomManager *KisView::zoomManager() const
{
    return d->zoomManager;
}

KisCanvasController *KisView::canvasController() const
{
    return d->canvasController;
}

KisCanvasResourceProvider *KisView::resourceProvider() const
{
    if (d->viewManager) {
        return d->viewManager->resourceProvider();
    }
    return 0;
}

KisInputManager* KisView::globalInputManager() const
{
    return d->viewManager ? d->viewManager->inputManager() : 0;
}

KisCanvas2 *KisView::canvasBase() const
{
    return d->canvas;
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
    return d->viewConverter;
}

void KisView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node")) {
        event->accept();

        // activate view if it should accept the drop
        this->setFocus();
    } else {
        event->ignore();
    }
}

void KisView::dropEvent(QDropEvent *event)
{
    KisImageSP kisimage = image();
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
    if (event->mimeData()->hasFormat("application/x-krita-node") ||
            event->mimeData()->hasImage())
    {
        KisShapeController *kritaShapeController =
                dynamic_cast<KisShapeController*>(d->document->shapeController());

        QList<KisNodeSP> nodes =
                KisMimeData::loadNodes(event->mimeData(), imageBounds,
                                       pasteCenter, forceRecenter,
                                       kisimage, kritaShapeController);

        foreach(KisNodeSP node, nodes) {
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

        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.length() > 0) {

            KMenu popup;
            popup.setObjectName("drop_popup");

            QAction *insertAsNewLayer = new KAction(i18n("Insert as New Layer"), &popup);
            QAction *insertManyLayers = new KAction(i18n("Insert Many Layers"), &popup);

            QAction *openInNewDocument = new KAction(i18n("Open in New Document"), &popup);
            QAction *openManyDocuments = new KAction(i18n("Open Many Documents"), &popup);

            QAction *cancel = new KAction(i18n("Cancel"), &popup);

            popup.addAction(insertAsNewLayer);
            popup.addAction(openInNewDocument);
            popup.addAction(insertManyLayers);
            popup.addAction(openManyDocuments);

            insertAsNewLayer->setEnabled(image() && urls.count() == 1);
            openInNewDocument->setEnabled(urls.count() == 1);
            insertManyLayers->setEnabled(image() && urls.count() > 1);
            openManyDocuments->setEnabled(urls.count() > 1);

            popup.addSeparator();
            popup.addAction(cancel);

            QAction *action = popup.exec(QCursor::pos());

            if (action != 0 && action != cancel) {
                foreach(const QUrl &url, urls) {

                    if (action == insertAsNewLayer || action == insertManyLayers) {
                        d->viewManager->imageManager()->importImage(KUrl(url));
                        activateWindow();
                    }
                    else {
                        Q_ASSERT(action == openInNewDocument || action == openManyDocuments);
                        if (mainWindow()) {
                            mainWindow()->openDocument(url);
                        }
                    }
                }
            }
        }
    }

}

KisDocument *KisView::document() const
{
    return d->document;
}

void KisView::setDocument(KisDocument *document)
{
    d->document->disconnect(this);
    d->document = document;
    KStatusBar *sb = statusBar();
    if (sb) { // No statusbar in e.g. konqueror
        connect(d->document, SIGNAL(statusBarMessage(const QString&)),
                this, SLOT(slotActionStatusText(const QString&)));
        connect(d->document, SIGNAL(clearStatusBarMessage()),
                this, SLOT(slotClearStatusText()));
    }
}

void KisView::setDocumentDeleted()
{
    d->documentDeleted = true;
}

void KisView::addStatusBarItem(QWidget * widget, int stretch, bool permanent)
{
    Private::StatusBarItem item(widget, stretch, permanent);
    KStatusBar * sb = statusBar();
    if (sb) {
        item.ensureItemShown(sb);
    }
    d->statusBarItems.append(item);
}

void KisView::removeStatusBarItem(QWidget *widget)
{
    KStatusBar *sb = statusBar();

    int itemCount = d->statusBarItems.count();
    for (int i = itemCount-1; i >= 0; --i) {
        Private::StatusBarItem &sbItem = d->statusBarItems[i];
        if (sbItem.widget() == widget) {
            if (sb) {
                sbItem.ensureItemHidden(sb);
            }
            d->statusBarItems.removeOne(sbItem);
            break;
        }
    }
}


KoPageLayout KisView::pageLayout() const
{
    return document()->pageLayout();
}

QPrintDialog *KisView::createPrintDialog(KisPrintJob *printJob, QWidget *parent)
{
    QPrintDialog *printDialog = KdePrint::createPrintDialog(&printJob->printer(),
                                                            printJob->createOptionWidgets(), parent);
    printDialog->setMinMax(printJob->printer().fromPage(), printJob->printer().toPage());
    printDialog->setEnabledOptions(printJob->printDialogOptions());
    return printDialog;
}


KisMainWindow * KisView::mainWindow() const
{
    return dynamic_cast<KisMainWindow *>(window());
}

KStatusBar * KisView::statusBar() const
{
    KisMainWindow *mw = mainWindow();
    return mw ? mw->statusBar() : 0;
}

void KisView::slotActionStatusText(const QString &text)
{
    KStatusBar *sb = statusBar();
    if (sb)
        sb->showMessage(text);
}

void KisView::slotClearStatusText()
{
    KStatusBar *sb = statusBar();
    if (sb)
        sb->clearMessage();
}

QList<QAction*> KisView::createChangeUnitActions(bool addPixelUnit)
{
    UnitActionGroup* unitActions = new UnitActionGroup(d->document, addPixelUnit, this);
    return unitActions->actions();
}

bool KisView::event(QEvent *event)
{
    switch(static_cast<int>(event->type()))
    {
    case ViewModeSwitchEvent::AboutToSwitchViewModeEvent: {
        ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();

        d->canvasController->setFocus();
        qApp->processEvents();

        KisCanvasResourceProvider* provider = resourceProvider();
        syncObject->backgroundColor = provider->bgColor();
        syncObject->foregroundColor = provider->fgColor();
        syncObject->exposure = provider->HDRExposure();
        syncObject->gamma = provider->HDRGamma();
        syncObject->compositeOp = provider->currentCompositeOp();
        syncObject->pattern = provider->currentPattern();
        syncObject->gradient = provider->currentGradient();
        syncObject->node = provider->currentNode();
        syncObject->paintOp = provider->currentPreset();
        syncObject->opacity = provider->opacity();
        syncObject->globalAlphaLock = provider->globalAlphaLock();

        syncObject->documentOffset = d->canvasController->scrollBarValue() - pos();
        syncObject->zoomLevel = zoomController()->zoomAction()->effectiveZoom();
        syncObject->rotationAngle = canvasBase()->rotationAngle();

        syncObject->activeToolId = KoToolManager::instance()->activeToolId();

        syncObject->gridData = &document()->gridData();

        syncObject->mirrorHorizontal = provider->mirrorHorizontal();
        syncObject->mirrorVertical = provider->mirrorVertical();
        syncObject->mirrorAxesCenter = provider->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF();

        KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(canvasBase(), syncObject->activeToolId));
        if(tool) {
            syncObject->smoothingOptions = tool->smoothingOptions();
        }

        syncObject->initialized = true;

        QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow());
        if(mainWindow) {
            QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
            foreach(QDockWidget* widget, dockWidgets) {
                if (widget->isFloating()) {
                    widget->hide();
                }
            }
        }

        return true;
    }
    case ViewModeSwitchEvent::SwitchedToDesktopModeEvent: {
        ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();
        d->canvasController->setFocus();
        qApp->processEvents();

        if(syncObject->initialized) {
            KisCanvasResourceProvider* provider = resourceProvider();

            provider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, syncObject->mirrorAxesCenter);
            if (provider->mirrorHorizontal() != syncObject->mirrorHorizontal) {
                QAction* mirrorAction = d->actionCollection->action("hmirror_action");
                mirrorAction->setChecked(syncObject->mirrorHorizontal);
                provider->setMirrorHorizontal(syncObject->mirrorHorizontal);
            }
            if (provider->mirrorVertical() != syncObject->mirrorVertical) {
                QAction* mirrorAction = d->actionCollection->action("vmirror_action");
                mirrorAction->setChecked(syncObject->mirrorVertical);
                provider->setMirrorVertical(syncObject->mirrorVertical);
            }

            provider->setPaintOpPreset(syncObject->paintOp);
            qApp->processEvents();

            KoToolManager::instance()->switchToolRequested(syncObject->activeToolId);
            qApp->processEvents();

            KisPaintOpPresetSP preset = canvasBase()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
            preset->settings()->setProperty("CompositeOp", syncObject->compositeOp);
            if(preset->settings()->hasProperty("OpacityValue"))
                preset->settings()->setProperty("OpacityValue", syncObject->opacity);
            provider->setPaintOpPreset(preset);

            provider->setBGColor(syncObject->backgroundColor);
            provider->setFGColor(syncObject->foregroundColor);
            provider->setHDRExposure(syncObject->exposure);
            provider->setHDRGamma(syncObject->gamma);
            provider->slotPatternActivated(syncObject->pattern);
            provider->slotGradientActivated(syncObject->gradient);
            provider->slotNodeActivated(syncObject->node);
            provider->setOpacity(syncObject->opacity);
            provider->setGlobalAlphaLock(syncObject->globalAlphaLock);
            provider->setCurrentCompositeOp(syncObject->compositeOp);

            document()->gridData().setGrid(syncObject->gridData->gridX(), syncObject->gridData->gridY());
            document()->gridData().setGridColor(syncObject->gridData->gridColor());
            document()->gridData().setPaintGridInBackground(syncObject->gridData->paintGridInBackground());
            document()->gridData().setShowGrid(syncObject->gridData->showGrid());
            document()->gridData().setSnapToGrid(syncObject->gridData->snapToGrid());

            d->actionCollection->action("zoom_in")->trigger();
            qApp->processEvents();


            QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow());
            if(mainWindow) {
                QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
                foreach(QDockWidget* widget, dockWidgets) {
                    if (widget->isFloating()) {
                        widget->show();
                    }
                }
            }

            zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, syncObject->zoomLevel);
            d->canvasController->rotateCanvas(syncObject->rotationAngle - canvasBase()->rotationAngle());

            QPoint newOffset = syncObject->documentOffset + pos();
            qApp->processEvents();
            d->canvasController->setScrollBarValue(newOffset);

            KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(canvasBase(), syncObject->activeToolId));
            if(tool && syncObject->smoothingOptions) {
                tool->smoothingOptions()->setSmoothingType(syncObject->smoothingOptions->smoothingType());
                tool->smoothingOptions()->setSmoothPressure(syncObject->smoothingOptions->smoothPressure());
                tool->smoothingOptions()->setTailAggressiveness(syncObject->smoothingOptions->tailAggressiveness());
                tool->smoothingOptions()->setUseScalableDistance(syncObject->smoothingOptions->useScalableDistance());
                tool->smoothingOptions()->setSmoothnessDistance(syncObject->smoothingOptions->smoothnessDistance());
                tool->smoothingOptions()->setUseDelayDistance(syncObject->smoothingOptions->useDelayDistance());
                tool->smoothingOptions()->setDelayDistance(syncObject->smoothingOptions->delayDistance());
                tool->smoothingOptions()->setFinishStabilizedCurve(syncObject->smoothingOptions->finishStabilizedCurve());
                tool->smoothingOptions()->setStabilizeSensors(syncObject->smoothingOptions->stabilizeSensors());
                tool->updateSettingsViews();
            }
        }

        return true;
    }
    default:
        break;
    }

    return QWidget::event( event );
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
        d->viewManager->removeStatusBarItem(zoomManager()->zoomActionWidget());
        event->accept();
        return;
    }

    event->ignore();

}

bool KisView::queryClose()
{
    if (!document())
        return true;

    if (document()->isModified()) {
        QString name;
        if (document()->documentInfo()) {
            name = document()->documentInfo()->aboutInfo("title");
        }
        if (name.isEmpty())
            name = document()->url().fileName();

        if (name.isEmpty())
            name = i18n("Untitled");

        int res = KMessageBox::warningYesNoCancel(this,
                                                  i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name),
                                                  QString(),
                                                  KStandardGuiItem::save(),
                                                  KStandardGuiItem::discard());

        switch (res) {
        case KMessageBox::Yes : {
            bool isNative = (document()->outputMimeType() == document()->nativeFormatMimeType());
            if (!viewManager()->mainWindow()->saveDocument(document(), !isNative))
                return false;
            break;
        }
        case KMessageBox::No :
            document()->removeAutoSaveFiles();
            document()->setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        default : // case KMessageBox::Cancel :
            return false;
        }
    }

    return true;

}

void KisView::resetImageSizeAndScroll(bool changeCentering,
                                      const QPointF oldImageStillPoint,
                                      const QPointF newImageStillPoint)
{
    const KisCoordinatesConverter *converter = d->canvas->coordinatesConverter();

    QPointF oldPreferredCenter = d->canvasController->preferredCenter();

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
        QSize oldDocumentSize = d->canvasController->documentSize();
        oldStillPoint = QPointF(0.5 * oldDocumentSize.width(), 0.5 * oldDocumentSize.height());
    }

    /**
     * Updating the document size
     */

    QSizeF size(image()->width() / image()->xRes(), image()->height() / image()->yRes());
    KoZoomController *zc = d->zoomManager->zoomController();
    zc->setZoom(KoZoomMode::ZOOM_CONSTANT, zc->zoomAction()->effectiveZoom());
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
        QSize newDocumentSize = d->canvasController->documentSize();
        newStillPoint = QPointF(0.5 * newDocumentSize.width(), 0.5 * newDocumentSize.height());
    }

    d->canvasController->setPreferredCenter(oldPreferredCenter - oldStillPoint + newStillPoint);
}

void KisView::setCurrentNode(KisNodeSP node)
{
    d->currentNode = node;
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
    return dynamic_cast<KisLayer*>(node.data());
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

    if (d->paintingAssistantsDecoration){
        foreach(KisPaintingAssistant* assist, document()->preLoadedAssistants()){
            d->paintingAssistantsDecoration->addAssistant(assist);
        }
        d->paintingAssistantsDecoration->setVisible(true);
    }

    canvasBase()->initializeImage();

    /**
     * Dirty hack alert
     */
    d->zoomManager->zoomController()->setAspectMode(true);

    if (viewConverter()) {
        viewConverter()->setZoomMode(KoZoomMode::ZOOM_PAGE);
    }
    connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), this, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)));
    connect(image(), SIGNAL(sigProfileChanged(const KoColorProfile*)), this, SIGNAL(sigProfileChanged(const KoColorProfile*)));
    connect(image(), SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SIGNAL(sigSizeChanged(QPointF,QPointF)));

    KisNodeSP activeNode = document()->preActivatedNode();
    document()->setPreActivatedNode(0); // to make sure that we don't keep a reference to a layer the user can later delete.

    if (!activeNode) {
        activeNode = image()->rootLayer()->lastChild();
    }

    while (activeNode && !activeNode->inherits("KisLayer")) {
        activeNode = activeNode->prevSibling();
    }

    setCurrentNode(activeNode);
}

void KisView::slotSavingFinished()
{
    if (d->viewManager && d->viewManager->mainWindow()) {
        d->viewManager->mainWindow()->updateCaption();
    }
}

KisPrintJob * KisView::createPrintJob()
{
    return new KisPrintJob(image());
}

void KisView::slotImageResolutionChanged()
{
    resetImageSizeAndScroll(false);
    zoomManager()->updateGUI();

    // update KoUnit value for the document
    if (resourceProvider()) {
        resourceProvider()->resourceManager()->
                setResource(KoCanvasResourceManager::Unit, d->canvas->unit());

    }

}

void KisView::slotImageSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint)
{
    resetImageSizeAndScroll(true, oldStillPoint, newStillPoint);
    zoomManager()->updateGUI();
}


#include <KisView_p.moc>
#include <KisView.moc>
