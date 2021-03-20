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
#include "KoCanvasBase.h"
#include "KoCanvasObserverBase.h"
#include "KoCanvasSupervisor.h"
#include "KoToolManager_p.h"

#include <FlakeDebug.h>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <kddockwidgets/DockWidget.h>
#include <QTimer>
#include <QPointer>

#include <QOpenGLWidget>

#include <math.h>

void KoCanvasControllerWidget::Private::setDocumentOffset()
{
    // The margins scroll the canvas widget inside the viewport, not
    // the document. The documentOffset is meant to be the value that
    // the canvas must add to the update rect in its paint event, to
    // compensate.

    QPoint pt(q->horizontalScrollBar()->value(), q->verticalScrollBar()->value());
    q->proxyObject->emitMoveDocumentOffset(pt);

    QWidget *canvasWidget = canvas->canvasWidget();

    if (canvasWidget) {
        // If it isn't an OpenGL canvas
        if (qobject_cast<QOpenGLWidget*>(canvasWidget) == 0) {
            QPoint diff = q->documentOffset() - pt;
            canvasWidget->scroll(diff.x(), diff.y(), canvasWidget->rect());
        }
    }

    q->setDocumentOffset(pt);
}

void KoCanvasControllerWidget::Private::resetScrollBars()
{
    // The scrollbar value always points at the top-left corner of the
    // bit of image we paint.

    int docH = (int)q->documentSize().height() + q->margin();
    int docW = (int)q->documentSize().width() + q->margin();
    int drawH = viewportWidget->height();
    int drawW = viewportWidget->width();

    QScrollBar *hScroll = q->horizontalScrollBar();
    QScrollBar *vScroll = q->verticalScrollBar();

    int horizontalReserve = vastScrollingFactor * drawW;
    int verticalReserve = vastScrollingFactor * drawH;

    int xMin = -horizontalReserve;
    int yMin = -verticalReserve;

    int xMax = docW - drawW + horizontalReserve;
    int yMax = docH - drawH + verticalReserve;

    hScroll->setRange(xMin, xMax);
    vScroll->setRange(yMin, yMax);

    int fontheight = QFontMetrics(q->font()).height();

    vScroll->setPageStep(drawH);
    vScroll->setSingleStep(fontheight);
    hScroll->setPageStep(drawW);
    hScroll->setSingleStep(fontheight);

}

void KoCanvasControllerWidget::Private::emitPointerPositionChangedSignals(QEvent *event)
{
    if (!canvas) return;
    if (!canvas->viewConverter()) return;

    QPoint pointerPos;
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent) {
        pointerPos = mouseEvent->pos();
    } else {
        QTabletEvent *tabletEvent = dynamic_cast<QTabletEvent*>(event);
        if (tabletEvent) {
            pointerPos = tabletEvent->pos();
        }
    }

    QPoint pixelPos = (pointerPos - canvas->documentOrigin()) + q->documentOffset();
    QPointF documentPos = canvas->viewConverter()->viewToDocument(pixelPos);

    q->proxyObject->emitDocumentMousePositionChanged(documentPos);
    q->proxyObject->emitCanvasMousePositionChanged(pointerPos);
}


#include <QTime>

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
KoCanvasControllerWidget::KoCanvasControllerWidget(KActionCollection * actionCollection, KoCanvasSupervisor *observerProvider, QWidget *parent)
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

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));
    connect(d->viewportWidget, SIGNAL(sizeChanged()), this, SLOT(updateCanvasOffsetX()));
    connect(proxyObject, SIGNAL(moveDocumentOffset(QPoint)), d->viewportWidget, SLOT(documentOffsetMoved(QPoint)));
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
    d->setDocumentOffset();
}

QSizeF KoCanvasControllerWidget::viewportSize() const
{
    // Calculate viewport size aligned to device pixels to match KisOpenGLCanvas2.
    qreal dpr = viewport()->devicePixelRatioF();
    int viewportWidth = static_cast<int>(viewport()->width() * dpr);
    int viewportHeight = static_cast<int>(viewport()->height() * dpr);
    return QSizeF(viewportWidth / dpr, viewportHeight / dpr);
}

void KoCanvasControllerWidget::resizeEvent(QResizeEvent *resizeEvent)
{
    proxyObject->emitSizeChanged(resizeEvent->size());

    // XXX: When resizing, keep the area we're looking at now in the
    // center of the resized view.
    resetScrollBars();
    d->setDocumentOffset();
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

        setPreferredCenterFractionX(0);
        setPreferredCenterFractionY(0);
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

int KoCanvasControllerWidget::visibleHeight() const
{
    if (d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int height1;
    if (canvasWidget == 0)
        height1 = viewport()->height();
    else
        height1 = qMin(viewport()->height(), canvasWidget->height());
    int height2 = height();
    return qMin(height1, height2);
}

int KoCanvasControllerWidget::visibleWidth() const
{
    if (d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int width1;
    if (canvasWidget == 0)
        width1 = viewport()->width();
    else
        width1 = qMin(viewport()->width(), canvasWidget->width());
    int width2 = width();
    return qMin(width1, width2);
}

int KoCanvasControllerWidget::canvasOffsetX() const
{
    int offset = -horizontalScrollBar()->value();

    if (d->canvas) {
        offset += d->canvas->canvasWidget()->x() + frameWidth();
    }

    return offset;
}

int KoCanvasControllerWidget::canvasOffsetY() const
{
    int offset = -verticalScrollBar()->value();

    if (d->canvas) {
        offset += d->canvas->canvasWidget()->y() + frameWidth();
    }

    return offset;
}

void KoCanvasControllerWidget::updateCanvasOffsetX()
{
    proxyObject->emitCanvasOffsetXChanged(canvasOffsetX());
    if (d->ignoreScrollSignals)
        return;

    setPreferredCenterFractionX((horizontalScrollBar()->value()
                                 + viewport()->width() / 2.0) / documentSize().width());
}

void KoCanvasControllerWidget::updateCanvasOffsetY()
{
    proxyObject->emitCanvasOffsetYChanged(canvasOffsetY());
    if (d->ignoreScrollSignals)
        return;

    setPreferredCenterFractionY((verticalScrollBar()->value()
                                 + verticalScrollBar()->pageStep() / 2.0) / documentSize().height());
}

void KoCanvasControllerWidget::ensureVisible(KoShape *shape)
{
    Q_ASSERT(shape);
    ensureVisible(d->canvas->viewConverter()->documentToView(shape->boundingRect()));
}

void KoCanvasControllerWidget::ensureVisible(const QRectF &rect, bool smooth)
{
    QRect currentVisible(-canvasOffsetX(), -canvasOffsetY(), visibleWidth(), visibleHeight());

    QRect viewRect = rect.toRect();
    viewRect.translate(d->canvas->documentOrigin());
    if (!viewRect.isValid() || currentVisible.contains(viewRect))
        return; // its visible. Nothing to do.

    // if we move, we move a little more so the amount of times we have to move is less.
    int jumpWidth = smooth ? 0 : currentVisible.width() / 5;
    int jumpHeight = smooth ? 0 : currentVisible.height() / 5;
    if (!smooth && viewRect.width() + jumpWidth > currentVisible.width())
        jumpWidth = 0;
    if (!smooth && viewRect.height() + jumpHeight > currentVisible.height())
        jumpHeight = 0;

    int horizontalMove = 0;
    if (currentVisible.width() <= viewRect.width())      // center view
        horizontalMove = viewRect.center().x() - currentVisible.center().x();
    else if (currentVisible.x() > viewRect.x())          // move left
        horizontalMove = viewRect.x() - currentVisible.x() - jumpWidth;
    else if (currentVisible.right() < viewRect.right())  // move right
        horizontalMove = viewRect.right() - qMax(0, currentVisible.right() - jumpWidth);

    int verticalMove = 0;
    if (currentVisible.height() <= viewRect.height())       // center view
        verticalMove = viewRect.center().y() - currentVisible.center().y();
    if (currentVisible.y() > viewRect.y())               // move up
        verticalMove = viewRect.y() - currentVisible.y() - jumpHeight;
    else if (currentVisible.bottom() < viewRect.bottom()) // move down
        verticalMove = viewRect.bottom() - qMax(0, currentVisible.bottom() - jumpHeight);

    pan(QPoint(horizontalMove, verticalMove));
}

void KoCanvasControllerWidget::recenterPreferred()
{
    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;

    QPointF center = preferredCenter();

    // convert into a viewport based point
    center.rx() += d->canvas->canvasWidget()->x() + frameWidth();
    center.ry() += d->canvas->canvasWidget()->y() + frameWidth();

    // scroll to a new center point
    QPointF topLeft = center - 0.5 * QPointF(viewport()->width(), viewport()->height());
    setScrollBarValue(topLeft.toPoint());

    d->ignoreScrollSignals = oldIgnoreScrollSignals;
}

void KoCanvasControllerWidget::zoomIn(const QPoint &center)
{
    zoomBy(center, sqrt(2.0));
}

void KoCanvasControllerWidget::zoomOut(const QPoint &center)
{
    zoomBy(center, sqrt(0.5));
}


void KoCanvasControllerWidget::zoomBy(const QPoint &center, qreal zoom)
{
    const QPointF oldCenter = preferredCenter();
    const QPointF newCenter = center + scrollBarValue();
    const QPointF stillPoint = (zoom * newCenter - oldCenter) / (zoom - 1.0);

    proxyObject->emitZoomRelative(zoom, stillPoint);
}

void KoCanvasControllerWidget::zoomTo(const QRect &viewRect)
{
    qreal scale;

    if (1.0 * viewport()->width() / viewRect.width() > 1.0 * viewport()->height() / viewRect.height())
        scale = 1.0 * viewport()->height() / viewRect.height();
    else
        scale = 1.0 * viewport()->width() / viewRect.width();

    zoomBy(viewRect.center(), scale);
}

void KoCanvasControllerWidget::updateDocumentSize(const QSizeF &sz, bool recalculateCenter)
{
    // Don't update if the document-size didn't changed to prevent infinite loops and unneeded updates.
    if (KoCanvasController::documentSize() == sz)
        return;

    if (!recalculateCenter) {
        // assume the distance from the top stays equal and recalculate the center.
        setPreferredCenterFractionX(documentSize().width() * preferredCenterFractionX() / sz.width());
        setPreferredCenterFractionY(documentSize().height() * preferredCenterFractionY() / sz.height());
    }

    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;
    KoCanvasController::setDocumentSize(sz);
    d->viewportWidget->setDocumentSize(sz);
    resetScrollBars();

    // Always emit the new offset.
    updateCanvasOffsetX();
    updateCanvasOffsetY();

    d->ignoreScrollSignals = oldIgnoreScrollSignals;
}

void KoCanvasControllerWidget::setZoomWithWheel(bool zoom)
{
    d->zoomWithWheel = zoom;
}

void KoCanvasControllerWidget::setVastScrolling(qreal factor)
{
    d->vastScrollingFactor = factor;
}

QPointF KoCanvasControllerWidget::currentCursorPosition() const
{
    QWidget *canvasWidget = d->canvas->canvasWidget();
    const KoViewConverter *converter = d->canvas->viewConverter();
    return converter->viewToDocument(canvasWidget->mapFromGlobal(QCursor::pos()) + d->canvas->canvasController()->documentOffset() - canvasWidget->pos());
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

void KoCanvasControllerWidget::setPreferredCenter(const QPointF &viewPoint)
{
    setPreferredCenterFractionX(viewPoint.x() / documentSize().width());
    setPreferredCenterFractionY(viewPoint.y() / documentSize().height());
    recenterPreferred();
}

QPointF KoCanvasControllerWidget::preferredCenter() const
{
    QPointF center;
    center.setX(preferredCenterFractionX() * documentSize().width());
    center.setY(preferredCenterFractionY() * documentSize().height());
    return center;
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

void KoCanvasControllerWidget::wheelEvent(QWheelEvent *event)
{
    if (d->zoomWithWheel != ((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)) {
        const qreal zoomCoeff = event->delta() > 0 ? sqrt(2.0) : sqrt(0.5);
        zoomRelativeToPoint(event->pos(), zoomCoeff);

        event->accept();
    } else
        QAbstractScrollArea::wheelEvent(event);
}

void KoCanvasControllerWidget::zoomRelativeToPoint(const QPoint &widgetPoint, qreal zoomCoeff)
{
    const QPoint offset = scrollBarValue();
    const QPoint mousePos(widgetPoint + offset);

    const bool oldIgnoreScrollSignals = d->ignoreScrollSignals;
    d->ignoreScrollSignals = true;
    proxyObject->emitZoomRelative(zoomCoeff, mousePos);
    d->ignoreScrollSignals = oldIgnoreScrollSignals;
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

void KoCanvasControllerWidget::setMargin(int margin)
{
    KoCanvasController::setMargin(margin);
    Q_ASSERT(d->viewportWidget);
    d->viewportWidget->setMargin(margin);
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

void KoCanvasControllerWidget::resetScrollBars()
{
    d->resetScrollBars();
}

qreal KoCanvasControllerWidget::vastScrollingFactor() const
{
    return d->vastScrollingFactor;
}

KoCanvasControllerWidget::Private *KoCanvasControllerWidget::priv()
{
    return d;
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoCanvasControllerWidget.cpp"
