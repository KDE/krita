/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006, 2008-2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * SPDX-FileCopyrightText: 2006, 2009 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2007 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2006-2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoCanvasControllerWidget.h"
#include "KoCanvasControllerWidget_p.h"

#include "KoCanvasControllerWidgetViewport_p.h"
#include "KoShape.h"
#include "KoViewConverter.h"
#include <KoZoomHandler.h>
#include "KoCanvasBase.h"
#include "KoCanvasObserverBase.h"
#include "KoCanvasSupervisor.h"
#include "KoToolManager_p.h"

#include <FlakeDebug.h>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <QTimer>

#include <QOpenGLWidget>
#include <KisCanvasState.h>

#include <math.h>
#include <kis_debug.h>

void KoCanvasControllerWidget::Private::activate()
{
    if (!observerProvider) {
        return;
    }
    KoCanvasBase *canvas = q->canvas();
    Q_FOREACH (KoCanvasObserverBase *docker, observerProvider->canvasObservers()) {
        KoCanvasObserverBase *observer = dynamic_cast<KoCanvasObserverBase*>(docker);
        if (observer) {
            observer->setObservedCanvas(canvas);
        }
    }
}

void KoCanvasControllerWidget::Private::unsetCanvas()
{
    if (!observerProvider) {
        return;
    }
    Q_FOREACH (KoCanvasObserverBase *docker, observerProvider->canvasObservers()) {
        KoCanvasObserverBase *observer = dynamic_cast<KoCanvasObserverBase*>(docker);
       if (observer) {
            if (observer->observedCanvas() == q->canvas()) {
                observer->unsetObservedCanvas();
            }
        }
    }
}

////////////
KoCanvasControllerWidget::KoCanvasControllerWidget(KisKActionCollection * actionCollection, KoCanvasSupervisor *observerProvider, QWidget *parent)
    : QAbstractScrollArea(parent)
    , KoCanvasController(actionCollection)
    , d(new Private(this, observerProvider))
{
    // We need to set this as QDeclarativeView sets them a bit different from QAbstractScrollArea
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // And then our own Viewport
    d->viewportWidget = new Viewport(this);
    setViewport(d->viewportWidget);
    d->viewportWidget->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::NoFocus);
    setFrameStyle(0);

    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAutoFillBackground(false);
    /*
      Fixes:   apps starting at zero zoom.
      Details: Since the document is set on the mainwindow before loading commences the inial show/layout can choose
          to set the document to be very small, even to be zero pixels tall.  Setting a sane minimum size on the
          widget means we no longer get rounding errors in zooming and we no longer end up with zero-zoom.
      Note: KoPage apps should probably startup with a sane document size; for Krita that's impossible
     */
    setMinimumSize(QSize(50, 50));
    setMouseTracking(true);
}

KoCanvasControllerWidget::~KoCanvasControllerWidget()
{
    delete d;
}

void KoCanvasControllerWidget::activate()
{
    d->activate();
}

void KoCanvasControllerWidget::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx);
    Q_UNUSED(dy);

    const KisCanvasState oldCanvasState = canvasState();

    const QPoint pt(horizontalScrollBar()->value(), verticalScrollBar()->value());
    if (oldCanvasState.documentOffset == pt) return;

    updateCanvasOffsetInternal(pt);

    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

void KoCanvasControllerWidget::resizeEvent(QResizeEvent *event)
{
    const KisCanvasState oldCanvasState = canvasState();

    updateCanvasWidgetSizeInternal(event->size());

    const KisCanvasState newCanvasState = canvasState();

    // jcall resize on the subordinate widget
    d->viewportWidget->resetLayout();

    emitSignals(oldCanvasState, newCanvasState);
}

void KoCanvasControllerWidget::setCanvas(KoCanvasBase *canvas)
{
    if (d->canvas) {
        d->unsetCanvas();
        proxyObject->emitCanvasRemoved(this);
        d->canvas->setCanvasController(0);
        d->canvas->canvasWidget()->removeEventFilter(this);
    }

    d->canvas = canvas;

    if (d->canvas) {
        d->canvas->setCanvasController(this);
        changeCanvasWidget(d->canvas->canvasWidget());

        proxyObject->emitCanvasSet(this);
        QTimer::singleShot(0, this, SLOT(activate()));
    }
}

KoCanvasBase* KoCanvasControllerWidget::canvas() const
{
    if (d->canvas.isNull()) return 0;
    return d->canvas;
}

void KoCanvasControllerWidget::changeCanvasWidget(QWidget *widget)
{
    if (d->viewportWidget->canvas()) {
        widget->setCursor(d->viewportWidget->canvas()->cursor());
        d->viewportWidget->canvas()->removeEventFilter(this);
    }

    d->viewportWidget->setCanvas(widget);
    setFocusProxy(d->canvas->canvasWidget());
}

void KoCanvasControllerWidget::emitSignals(const KisCanvasState &oldState, const KisCanvasState &newState)
{
    if (oldState.documentOffsetF != newState.documentOffsetF) {
        proxyObject->emitCanvasOffsetChanged();
    }

    if (oldState.viewportOffsetF != newState.viewportOffsetF) {
        qDebug() << "emitting viewport offset changed"
                 << "old:" << oldState.viewportOffsetF
                 << "new:" << newState.viewportOffsetF;
        proxyObject->emitMoveViewportOffset(oldState.viewportOffsetF, newState.viewportOffsetF);
    }

    if (oldState.documentOffsetF != newState.documentOffsetF) {
        proxyObject->emitMoveDocumentOffset(oldState.documentOffsetF, newState.documentOffsetF);
    }

    if (oldState.canvasSize != newState.canvasSize) {
        // TODO: should we remove this signal?
        proxyObject->emitSizeChanged(newState.canvasSize.toSize());
    }

    if (oldState.documentOffset != newState.documentOffset ||
        oldState.minimumOffset != newState.minimumOffset ||
        oldState.maximumOffset != newState.maximumOffset) {

        resetScrollBars();
    }

    if (!qFuzzyCompare(oldState.effectiveZoom, newState.effectiveZoom)) {
        proxyObject->emitEffectiveZoomChanged(newState.effectiveZoom);
    }

    if (oldState.zoomState() != newState.zoomState()) {
        proxyObject->emitZoomStateChanged(newState.zoomState());
    }

    if (!qFuzzyCompare(oldState.rotation, newState.rotation)) {
        proxyObject->emitDocumentRotationChanged(newState.rotation);
    }

    if (oldState.mirrorHorizontally != newState.mirrorHorizontally
        || oldState.mirrorVertically != newState.mirrorVertically) {

        proxyObject->emitDocumentMirrorStatusChanged(newState.mirrorHorizontally, newState.mirrorVertically);
    }

    if (oldState.imageRectInWidgetPixels != newState.imageRectInWidgetPixels) {
        proxyObject->emitDocumentRectInWidgetPixelsChanged(newState.imageRectInWidgetPixels);
    }
}

void KoCanvasControllerWidget::zoomTo(const QRect &viewRect)
{
    const KisCanvasState oldCanvasState = canvasState();

    zoomToInternal(viewRect);

    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

void KoCanvasControllerWidget::pan(const QPoint &distance)
{
    QPoint sourcePoint = scrollBarValue();
    setScrollBarValue(sourcePoint + distance);
}

void KoCanvasControllerWidget::panUp()
{
    pan(QPoint(0, verticalScrollBar()->singleStep()));
}

void KoCanvasControllerWidget::panDown()
{
    pan(QPoint(0, -verticalScrollBar()->singleStep()));
}

void KoCanvasControllerWidget::panLeft()
{
    pan(QPoint(horizontalScrollBar()->singleStep(), 0));
}

void KoCanvasControllerWidget::panRight()
{
    pan(QPoint(-horizontalScrollBar()->singleStep(), 0));
}

void KoCanvasControllerWidget::paintEvent(QPaintEvent *event)
{
    QPainter gc(viewport());
    d->viewportWidget->handlePaintEvent(gc, event);
}

void KoCanvasControllerWidget::dragEnterEvent(QDragEnterEvent *event)
{
    d->viewportWidget->handleDragEnterEvent(event);
}

void KoCanvasControllerWidget::dropEvent(QDropEvent *event)
{
    d->viewportWidget->handleDropEvent(event);
}

void KoCanvasControllerWidget::dragMoveEvent(QDragMoveEvent *event)
{
    d->viewportWidget->handleDragMoveEvent(event);
}

void KoCanvasControllerWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->viewportWidget->handleDragLeaveEvent(event);
}

void KoCanvasControllerWidget::zoomRelativeToPoint(const QPointF &widgetPoint, qreal zoomCoeff)
{
    const qreal newZoom = d->canvas->viewConverter()->zoom() * zoomCoeff;
    setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, widgetPoint);
}

void KoCanvasControllerWidget::setZoom(KoZoomMode::Mode mode, qreal zoom)
{
    setZoom(mode, zoom, QRectF(QPointF(), canvasState().canvasSize).center());
}

void KoCanvasControllerWidget::setZoom(KoZoomMode::Mode mode, qreal zoom, const QPointF &stillPoint)
{

    KoZoomHandler *zoomHandler = dynamic_cast<KoZoomHandler*>(d->canvas->viewConverter());
    KIS_SAFE_ASSERT_RECOVER_RETURN(zoomHandler);
    setZoom(mode, zoom, zoomHandler->resolutionX(), zoomHandler->resolutionY(), stillPoint);
}

void KoCanvasControllerWidget::setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY)
{
    setZoom(mode, zoom, resolutionX, resolutionY, QRectF(QPointF(), canvasState().canvasSize).center());
}

void KoCanvasControllerWidget::setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const QPointF &stillPoint)
{
    const KisCanvasState oldCanvasState = canvasState();

    updateCanvasZoomInternal(mode, zoom, resolutionX, resolutionY, stillPoint);

    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

bool KoCanvasControllerWidget::focusNextPrevChild(bool)
{
    // we always return false meaning the canvas takes keyboard focus, but never gives it away.
    return false;
}

bool KoCanvasControllerWidget::viewportEvent(QEvent *event) {
    // Workaround: Don't let QAbstractScrollArea handle Gesture events. Because
    // Qt's detection of touch point positions is a bit buggy, it is handled
    // with custom algorithms in the KisInputManager. But we must also not let
    // the corresponding event propagate to the parent QAbstractScrollArea.
    if (event->type() == QEvent::Gesture) {
        return false;
    }
    return QAbstractScrollArea::viewportEvent(event);
}

QPoint KoCanvasControllerWidget::scrollBarValue() const
{
    QScrollBar * hBar = horizontalScrollBar();
    QScrollBar * vBar = verticalScrollBar();

    return QPoint(hBar->value(), vBar->value());
}

void KoCanvasControllerWidget::setScrollBarValue(const QPoint &value)
{
    QScrollBar * hBar = horizontalScrollBar();
    QScrollBar * vBar = verticalScrollBar();

    hBar->setValue(value.x());
    vBar->setValue(value.y());
}

KoCanvasControllerWidget::Private *KoCanvasControllerWidget::priv()
{
    return d;
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoCanvasControllerWidget.cpp"
