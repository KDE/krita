/* This file is part of the KDE project
 * Copyright (C) 2012 Boudewijn Rempt <boud@kogmbh.com>
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
#include "KisSketchView.h"

#include <QApplication>
#include <QScrollBar>
#include <QFileInfo>

#include <kactioncollection.h>

#include <KoZoomController.h>
#include <KoToolManager.h>

#include "KisDocument.h"
#include "kis_canvas2.h"
#include <kis_canvas_controller.h>
#include "KisViewManager.h"
#include <kis_image_signal_router.h>
#include <input/kis_input_manager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_selection_manager.h>
#include <KisPart.h>
#include <kis_tool_freehand.h>
#include <kis_paintop_box.h>
#include "KisSelectionExtras.h"

#include "ProgressProxy.h"
#include "DocumentManager.h"

class KisSketchView::Private
{
public:
    Private( KisSketchView* qq)
        : q(qq)
        , actionCollection(0)
        , doc(0)
        , viewManager(0)
        , view(0)
        , canvas(0)
        , canvasWidget(0)
        , selectionExtras(0)
        , undoAction(0)
        , redoAction(0)
        , tabletEventCount(0)
    { }
    ~Private() {
        delete selectionExtras;
    }

    void imageUpdated(const QRect &updated);
    void documentOffsetMoved();
    void zoomChanged();
    void resetDocumentPosition();
    void removeNodeAsync(KisNodeSP removedNode);

    KisSketchView* q;

    KActionCollection *actionCollection;

    QPointer<KisDocument> doc;
    QPointer<KisViewManager> viewManager;
    QPointer<KisView> view;

    QPointer<KisCanvas2> canvas;
    KUndo2Stack* undoStack;

    QWidget *canvasWidget;

    QString file;

    KisSelectionExtras *selectionExtras;

    QTimer *timer;

    QTimer *loadedTimer;
    QTimer *savedTimer;
    QAction* undoAction;
    QAction* redoAction;

    unsigned char tabletEventCount;
};

KisSketchView::KisSketchView(QQuickItem* parent)
    : QQuickItem(parent)
    , d(new Private(this))
{
    // this is just an interaction overlay, the contents are painted on the sceneview background
    setFlag(QQuickItem::ItemHasContents, false);
    // QT5TODO
//     setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
    setAcceptHoverEvents(true);

    d->actionCollection = new KActionCollection(this, "krita");
    d->viewManager = 0; //new KisViewManager(qApp->activeWindow(), d->actionCollection);

    // QT5TODO
//     grabGesture(Qt::PanGesture);
    //grabGesture(Qt::PinchGesture);

    KoZoomMode::setMinimumZoom(0.1);
    KoZoomMode::setMaximumZoom(16.0);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(resetDocumentPosition()));

    d->loadedTimer = new QTimer(this);
    d->loadedTimer->setSingleShot(true);
    d->loadedTimer->setInterval(100);
    connect(d->loadedTimer, SIGNAL(timeout()), SIGNAL(loadingFinished()));

    d->savedTimer = new QTimer(this);
    d->savedTimer->setSingleShot(true);
    d->savedTimer->setInterval(100);
    connect(d->savedTimer, SIGNAL(timeout()), SIGNAL(savingFinished()));

    connect(DocumentManager::instance(), SIGNAL(aboutToDeleteDocument()), SLOT(documentAboutToBeDeleted()));
    connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(documentChanged()));
    connect(DocumentManager::instance()->progressProxy(), SIGNAL(valueChanged(int)), SIGNAL(progress(int)));
    connect(DocumentManager::instance(), SIGNAL(documentSaved()), d->savedTimer, SLOT(start()));

    if (DocumentManager::instance()->document()) {
        documentChanged();
    }
}

KisSketchView::~KisSketchView()
{
    if (d->doc) {
        DocumentManager::instance()->closeDocument();
    }
    if (d->canvasWidget) {
    // QT5TODO
//         SketchDeclarativeView *v = qobject_cast<SketchDeclarativeView*>(scene()->views().at(0));
//         if (v) {
//             v->setCanvasWidget(0);
//             v->setDrawCanvas(false);
//         }
    }

    delete d;
}

QObject* KisSketchView::selectionManager() const
{
    if (!d->viewManager)
        return 0;
    return d->viewManager->selectionManager();
}

QObject* KisSketchView::selectionExtras() const
{
    if (!d->selectionExtras) {
        d->selectionExtras = new KisSelectionExtras(d->viewManager);
    }
    return d->selectionExtras;
}

QObject* KisSketchView::doc() const
{
    return d->doc;
}

QObject* KisSketchView::view() const
{
    return d->viewManager;
}

QString KisSketchView::file() const
{
    return d->file;
}

QString KisSketchView::fileTitle() const
{
    QFileInfo file(d->file);
    return file.fileName();
}

bool KisSketchView::isModified() const
{
    if(d->doc)
        return d->doc->isModified();

    return false;
}

void KisSketchView::setFile(const QString& file)
{
    if (!file.isEmpty() && file != d->file) {
        d->file = file;
        emit fileChanged();

        if (!file.startsWith("temp://")) {
            DocumentManager::instance()->openDocument(file);
        }
    }
}

void KisSketchView::componentComplete()
{
}

bool KisSketchView::canUndo() const
{
    if (d->undoAction)
        return d->undoAction->isEnabled();
    return false;
}

bool KisSketchView::canRedo() const
{
    if (d->redoAction)
        return d->redoAction->isEnabled();
    return false;
}

int KisSketchView::imageHeight() const
{
    if (d->doc)
        return d->doc->image()->height();
    return 0;
}

int KisSketchView::imageWidth() const
{
    if (d->doc)
        return d->doc->image()->width();
    return 0;
}

void KisSketchView::undo()
{
    d->undoAction->trigger();
}

void KisSketchView::redo()
{
    d->redoAction->trigger();
}

void KisSketchView::zoomIn()
{
    d->viewManager->actionCollection()->action("zoom_in")->trigger();
}

void KisSketchView::zoomOut()
{
    d->viewManager->actionCollection()->action("zoom_out")->trigger();
}

void KisSketchView::save()
{
    DocumentManager::instance()->save();
}

void KisSketchView::saveAs(const QString& fileName, const QString& mimeType)
{
    DocumentManager::instance()->saveAs(fileName, mimeType);
}

void KisSketchView::documentAboutToBeDeleted()
{
    if (d->undoAction)
        d->undoAction->disconnect(this);

    if (d->redoAction)
        d->redoAction->disconnect(this);

    delete d->view;
    d->view = 0;

    emit viewChanged();

    d->canvas = 0;
    d->canvasWidget = 0;
}

void KisSketchView::documentChanged()
{
    d->doc = DocumentManager::instance()->document();
    if (!d->doc) return;
    if (!d->viewManager) return;

    connect(d->doc, SIGNAL(modified(bool)), SIGNAL(modifiedChanged()));

    QPointer<KisView> view = qobject_cast<KisView*>(KisPart::instance()->createView(d->doc,
                                                                                    d->viewManager->canvasResourceProvider()->resourceManager(),
                                                                                    d->viewManager->actionCollection(),
                                                                                    QApplication::activeWindow()));
    view->setViewManager(d->viewManager);
    view->canvasBase()->setFavoriteResourceManager(d->viewManager->paintOpBox()->favoriteResourcesManager());
    view->slotLoadingFinished();

    d->view = view;
    d->canvas = d->view->canvasBase();
    d->view->setShowFloatingMessage(false);
    d->viewManager->setCurrentView(view);
    KisCanvasController *controller = static_cast<KisCanvasController*>(d->canvas->canvasController());

    connect(d->viewManager, SIGNAL(floatingMessageRequested(QString,QString)), this, SIGNAL(floatingMessageRequested(QString,QString)));

    controller->setGeometry(x(), y(), width(), height());
    d->view->hide();

    d->undoStack = d->doc->undoStack();
    d->undoAction = d->viewManager->actionCollection()->action("edit_undo");
    connect(d->undoAction, SIGNAL(changed()), this, SIGNAL(canUndoChanged()));

    d->redoAction = d->viewManager->actionCollection()->action("edit_redo");
    connect(d->redoAction, SIGNAL(changed()), this, SIGNAL(canRedoChanged()));

    KoToolManager::instance()->switchToolRequested( "KritaShape/KisToolBrush" );

    d->canvasWidget = d->canvas->canvasWidget();



    connect(d->doc->image(), SIGNAL(sigImageUpdated(QRect)), SLOT(imageUpdated(QRect)));
    connect(controller->proxyObject, SIGNAL(moveDocumentOffset(QPoint)), SLOT(documentOffsetMoved()));
    connect(d->view->zoomController(), SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)), SLOT(zoomChanged()));
    connect(d->canvas, SIGNAL(updateCanvasRequested(QRect)), SLOT(imageUpdated(QRect)));
    connect(d->doc->image()->signalRouter(), SIGNAL(sigRemoveNodeAsync(KisNodeSP)), SLOT(removeNodeAsync(KisNodeSP)));
    connect(d->doc->image()->signalRouter(), SIGNAL(sigSizeChanged(QPointF,QPointF)), SIGNAL(imageSizeChanged()));

    // QT5TODO
//     if(scene()) {
//         SketchDeclarativeView *v = qobject_cast<SketchDeclarativeView*>(scene()->views().at(0));
//         if (v) {
//             v->setCanvasWidget(d->canvasWidget);
//             v->setDrawCanvas(true);
//         }
//     }

    d->imageUpdated(d->canvas->image()->bounds());

    static_cast<KoZoomHandler*>(d->canvas->viewConverter())->setResolution(d->doc->image()->xRes(), d->doc->image()->yRes());
    d->view->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);
    controller->setScrollBarValue(QPoint(0, 0));
    controller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    controller->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    geometryChanged(QRectF(x(), y(), width(), height()), QRectF());

    d->loadedTimer->start(100);

    d->viewManager->actionCollection()->action("zoom_to_100pct")->trigger();
    d->resetDocumentPosition();

    emit viewChanged();

}

bool KisSketchView::event( QEvent* event )
{
    if (!d->viewManager) return false;
    if (!d->viewManager->canvasBase()) return false;

    KisCanvasController *controller = dynamic_cast<KisCanvasController*>(d->viewManager->canvasBase()->canvasController());
    if (!controller) return false;

//    switch(static_cast<int>(event->type())) {
//        case ViewModeSwitchEvent::AboutToSwitchViewModeEvent: {
//            ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();

//            if (d->view && d->viewManager && d->viewManager->canvasBase()) {

//                controller->setFocus();
//                qApp->processEvents();

//                KisCanvasResourceProvider* provider = d->view->resourceProvider();
//                syncObject->backgroundColor = provider->bgColor();
//                syncObject->foregroundColor = provider->fgColor();
//                syncObject->exposure = provider->HDRExposure();
//                syncObject->gamma = provider->HDRGamma();
//                syncObject->compositeOp = provider->currentCompositeOp();
//                syncObject->pattern = provider->currentPattern();
//                syncObject->gradient = provider->currentGradient();
//                syncObject->node = provider->currentNode();
//                syncObject->paintOp = provider->currentPreset();
//                syncObject->opacity = provider->opacity();
//                syncObject->globalAlphaLock = provider->globalAlphaLock();

//                syncObject->documentOffset = controller->scrollBarValue();
//                syncObject->zoomLevel = d->view->zoomController()->zoomAction()->effectiveZoom();
//                syncObject->rotationAngle = d->view->canvasBase()->rotationAngle();

//                syncObject->activeToolId = KoToolManager::instance()->activeToolId();

//                syncObject->gridConfig = d->view->document()->gridConfig();

//                syncObject->mirrorHorizontal = provider->mirrorHorizontal();
//                syncObject->mirrorVertical = provider->mirrorVertical();
//                //syncObject->mirrorAxesCenter = provider->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF();

//                KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(d->view->canvasBase(), syncObject->activeToolId));
//                if(tool) {
//                    syncObject->smoothingOptions = tool->smoothingOptions();
//                }

//                syncObject->initialized = true;
//            }

//            return true;
//        }
//        case ViewModeSwitchEvent::SwitchedToSketchModeEvent: {
//            ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();

//            if (d->view && syncObject->initialized) {
//                controller->setFocus();
//                qApp->processEvents();

//                KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(d->view->canvasBase(), syncObject->activeToolId));
//                if(tool && syncObject->smoothingOptions) {
//                    tool->smoothingOptions()->setSmoothingType(syncObject->smoothingOptions->smoothingType());
//                    tool->smoothingOptions()->setSmoothPressure(syncObject->smoothingOptions->smoothPressure());
//                    tool->smoothingOptions()->setTailAggressiveness(syncObject->smoothingOptions->tailAggressiveness());
//                    tool->smoothingOptions()->setUseScalableDistance(syncObject->smoothingOptions->useScalableDistance());
//                    tool->smoothingOptions()->setSmoothnessDistance(syncObject->smoothingOptions->smoothnessDistance());
//                    tool->smoothingOptions()->setUseDelayDistance(syncObject->smoothingOptions->useDelayDistance());
//                    tool->smoothingOptions()->setDelayDistance(syncObject->smoothingOptions->delayDistance());
//                    tool->smoothingOptions()->setFinishStabilizedCurve(syncObject->smoothingOptions->finishStabilizedCurve());
//                    tool->smoothingOptions()->setStabilizeSensors(syncObject->smoothingOptions->stabilizeSensors());
//                    tool->updateSettingsViews();
//                }

//                KisCanvasResourceProvider* provider = d->view->resourceProvider();

//                provider->setMirrorHorizontal(syncObject->mirrorHorizontal);
//                provider->setMirrorVertical(syncObject->mirrorVertical);
//                //provider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, syncObject->mirrorAxesCenter);

//                provider->setPaintOpPreset(syncObject->paintOp);
//                qApp->processEvents();

//                KoToolManager::instance()->switchToolRequested("InteractionTool");
//                qApp->processEvents();
//                KoToolManager::instance()->switchToolRequested(syncObject->activeToolId);
//                qApp->processEvents();

//                provider->setBGColor(syncObject->backgroundColor);
//                provider->setFGColor(syncObject->foregroundColor);
//                provider->setHDRExposure(syncObject->exposure);
//                provider->setHDRGamma(syncObject->gamma);
//                provider->slotPatternActivated(syncObject->pattern);
//                provider->slotGradientActivated(syncObject->gradient);
//                provider->slotNodeActivated(syncObject->node);
//                provider->setOpacity(syncObject->opacity);
//                provider->setGlobalAlphaLock(syncObject->globalAlphaLock);
//                provider->setCurrentCompositeOp(syncObject->compositeOp);

//                d->view->document()->setGridConfig(syncObject->gridConfig);

//                zoomIn();
//                qApp->processEvents();

//                d->view->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, syncObject->zoomLevel);
//                controller->rotateCanvas(syncObject->rotationAngle - d->view->canvasBase()->rotationAngle());

//                qApp->processEvents();
//                QPoint newOffset = syncObject->documentOffset;
//                controller->setScrollBarValue(newOffset);
//            }

//            return true;
//        }
//#ifdef Q_OS_X11
//            if(d->tabletEventCount % 2 == 0)
//#endif
//                d->canvas->globalInputManager()->eventFilter(this, event);
//            return true;
//        case QEvent::KeyPress:
//        case QEvent::KeyRelease:
//            emit interactionStarted();
//            QApplication::sendEvent(d->view, event);
//            break;
//        default:
//            break;
//    }

    return QQuickItem::event( event );
}

    // QT5TODO
#if 0
bool KisSketchView::sceneEvent(QEvent* event)
{
    if (d->canvas && d->canvasWidget) {
        switch(event->type()) {
        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseButtonPress, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneMouseMove: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseMove, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            update();
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *gsmevent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QMouseEvent mevent(QMouseEvent::MouseButtonRelease, gsmevent->pos().toPoint(), gsmevent->button(), gsmevent->buttons(), gsmevent->modifiers());
            QApplication::sendEvent(d->canvasWidget, &mevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneWheel: {
            QGraphicsSceneWheelEvent *gswevent = static_cast<QGraphicsSceneWheelEvent*>(event);
            QWheelEvent wevent(gswevent->pos().toPoint(), gswevent->delta(), gswevent->buttons(), gswevent->modifiers(), gswevent->orientation());
            QApplication::sendEvent(d->canvasWidget, &wevent);
            emit interactionStarted();
            return true;
        }
        case QEvent::GraphicsSceneHoverEnter: {
            QGraphicsSceneHoverEvent *hevent = static_cast<QGraphicsSceneHoverEvent*>(event);
            QHoverEvent e(QEvent::Enter, hevent->screenPos(), hevent->lastScreenPos());
            QApplication::sendEvent(d->canvasWidget, &e);
            return true;
        }
        case QEvent::GraphicsSceneHoverLeave: {
            QGraphicsSceneHoverEvent *hevent = static_cast<QGraphicsSceneHoverEvent*>(event);
            QHoverEvent e(QEvent::Leave, hevent->screenPos(), hevent->lastScreenPos());
            QApplication::sendEvent(d->canvasWidget, &e);
            return true;
        }
        case QEvent::TouchBegin: {
            QApplication::sendEvent(d->canvasWidget, event);
            event->accept();
            emit interactionStarted();
            return true;
        }
        case QEvent::TabletPress:
        case QEvent::TabletMove:
        case QEvent::TabletRelease:
            d->canvas->globalInputManager()->stopIgnoringEvents();
            QApplication::sendEvent(d->canvasWidget, event);
            return true;
        default:
            if (QApplication::sendEvent(d->canvasWidget, event)) {
                emit interactionStarted();
                return true;
            }
        }
    }
    return QQuickItem::sceneEvent(event);
}
#endif
void KisSketchView::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (d->canvasWidget && !newGeometry.isEmpty()) {
        d->view->resize(newGeometry.toRect().size());
        // If we don't ask for this event to be sent, the view does not actually handle
        // the resize, and we're stuck with a very oddly sized viewport
        QResizeEvent *event = new QResizeEvent(newGeometry.toRect().size(), d->view->size());
        QApplication::sendEvent(d->view, event);
        // This is a touch on the hackish side - i'm sure there's a better way of doing it
        // but it's taking a long time to work it out. Problem: When switching orientation,
        // the canvas is rendered wrong, in what looks like an off-by-one ish kind of fashion.
        if (oldGeometry.height() == oldGeometry.width() && oldGeometry.height() == newGeometry.width()) {
            // in this case, we've just rotated the display... do something useful!
            // Turns out we get /two/ resize events per rotation, one one per setting each height and width.
            // So we can't just check it normally. Annoying, but there you go.
            QTimer::singleShot(100, this, SLOT(centerDoc()));
            QTimer::singleShot(150, this, SLOT(zoomOut()));
        }
        if (oldGeometry.height() == oldGeometry.width() && oldGeometry.width() == newGeometry.height()) {
            // in this case, we've just rotated the display... do something useful!
            // Turns out we get /two/ resize events per rotation, one one per setting each height and width.
            // So we can't just check it normally. Annoying, but there you go.
            QTimer::singleShot(100, this, SLOT(centerDoc()));
            QTimer::singleShot(150, this, SLOT(zoomOut()));
        }
    }
}

void KisSketchView::centerDoc()
{
    d->viewManager->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
}

void KisSketchView::Private::imageUpdated(const QRect &/*updated*/)
{
    // QT5TODO
//     if (q->scene()) {
//         q->scene()->views().at(0)->update(updated);
//         q->scene()->invalidate( 0, 0, q->width(), q->height() );
//     }
}

void KisSketchView::Private::documentOffsetMoved()
{
    // QT5TODO
//     if (q->scene()) {
//         q->scene()->views().at(0)->update();
//         q->scene()->invalidate( 0, 0, q->width(), q->height() );
//     }
}

void KisSketchView::Private::resetDocumentPosition()
{
    viewManager->zoomController()->setZoomMode(KoZoomMode::ZOOM_PAGE);

    QPoint pos;
    KisCanvasController *controller = dynamic_cast<KisCanvasController*>(viewManager->canvasBase()->canvasController());

    if (!controller) return;

    QScrollBar *sb = controller->horizontalScrollBar();
    pos.rx() = sb->minimum() + (sb->maximum() - sb->minimum()) / 2;

    sb = controller->verticalScrollBar();
    pos.ry() = sb->minimum() + (sb->maximum() - sb->minimum()) / 2;

    controller->setScrollBarValue(pos);

}


void KisSketchView::Private::removeNodeAsync(KisNodeSP removedNode)
{
    if (removedNode) {
        imageUpdated(removedNode->extent());
    }
}

void KisSketchView::Private::zoomChanged()
{
    // QT5TODO
//     if (q->scene()) {
//         q->scene()->views().at(0)->update();
//         q->scene()->invalidate( 0, 0, q->width(), q->height() );
//     }
}

void KisSketchView::activate()
{
    if (d->canvasWidget != d->canvas->canvasWidget()) {
        d->canvasWidget = d->canvas->canvasWidget();
    // QT5TODO
// 		SketchDeclarativeView *v = qobject_cast<SketchDeclarativeView*>(scene()->views().at(0));
// 		if (v) {
// 			v->setCanvasWidget(d->canvasWidget);
// 			v->setDrawCanvas(true);
// 		}
    }
    d->canvasWidget->setFocus();
    Q_ASSERT(d->viewManager);
    KisCanvasController *controller = dynamic_cast<KisCanvasController*>(d->viewManager->canvasBase()->canvasController());
    Q_ASSERT(controller);
    controller->activate();
}

// for private slots
#include "moc_KisSketchView.cpp"
