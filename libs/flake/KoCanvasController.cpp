/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoCanvasController.h"
#include "KoCanvasController_p.h"
#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoCanvasBase.h"

#include <ksharedconfig.h>

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>

#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <QGLWidget>
#endif

class KoCanvasController::Private
{
public:
    Private() : canvas(0), canvasMode( Centered ), toolOptionWidget(0), margin(0)
    ,ignoreScrollSignals(false) {}
    KoCanvasBase * canvas;
    CanvasMode canvasMode;
    QWidget * toolOptionWidget;
    int margin; // The viewport margin around the document
    QSize documentSize;
    QPoint documentOffset;
    Viewport * viewportWidget;
    double preferredCenterFractionX;
    double preferredCenterFractionY;
    bool ignoreScrollSignals;
};

KoCanvasController::KoCanvasController(QWidget *parent)
    : QAbstractScrollArea(parent),
    m_d(new Private())
{
    setFrameShape(NoFrame);
    m_d->viewportWidget = new Viewport( this );
    setViewport(m_d->viewportWidget);

    setAutoFillBackground(false);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));

    setMouseTracking( true );

    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->margin = cfg.readEntry("canvasmargin",  0);

    connect( this, SIGNAL( moveDocumentOffset( QPoint ) ), m_d->viewportWidget, SLOT( documentOffsetMoved( QPoint ) ) );
}

KoCanvasController::~KoCanvasController()
{
    delete m_d;
}


void KoCanvasController::scrollContentsBy ( int dx, int dy )
{
    Q_UNUSED( dx );
    Q_UNUSED( dy );
    setDocumentOffset();
}

void KoCanvasController::resizeEvent(QResizeEvent * resizeEvent)
{
    emit sizeChanged(resizeEvent->size());

    // XXX: When resizing, keep the area we're looking at now in the
    // center of the resized view.
    resetScrollBars();
    setDocumentOffset();
}

void KoCanvasController::setCanvas(KoCanvasBase *canvas) {
    Q_ASSERT(canvas); // param is not null
    if(m_d->canvas) {
        emit canvasRemoved(this);
        canvas->setCanvasController(0);
    }
    m_d->viewportWidget->setCanvas(canvas->canvasWidget());
    m_d->canvas = canvas;
    m_d->canvas->canvasWidget()->installEventFilter(this);
    m_d->canvas->canvasWidget()->setMouseTracking( true );
    canvas->setCanvasController(this);

    emit canvasSet(this);
}

KoCanvasBase* KoCanvasController::canvas() const {
    return m_d->canvas;
}

int KoCanvasController::visibleHeight() const {
    if(m_d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int height1;
    if(canvasWidget == 0)
        height1 = viewport()->height();
    else
        height1 = qMin(viewport()->height(), canvasWidget->height());
    int height2 = height();
    return qMin(height1, height2);
}

int KoCanvasController::visibleWidth() const {
    if(m_d->canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int width1;
    if(canvasWidget == 0)
        width1 = viewport()->width();
    else
        width1 = qMin(viewport()->width(), canvasWidget->width());
    int width2 = width();
    return qMin(width1, width2);
}

void KoCanvasController::setCanvasMode( CanvasMode mode )
{
    m_d->canvasMode = mode;
}

KoCanvasController::CanvasMode KoCanvasController::canvasMode() const
{
    return m_d->canvasMode;
}

int KoCanvasController::canvasOffsetX() const {
    int offset = 0;

    if(m_d->canvas) {
        offset = m_d->canvas->canvasWidget()->x() + frameWidth();
    }

    if(horizontalScrollBar()) {
        offset -= horizontalScrollBar()->value();
    }

    return offset;
}

int KoCanvasController::canvasOffsetY() const {
    int offset = 0;

    if(m_d->canvas) {
        offset = m_d->canvas->canvasWidget()->y() + frameWidth();
    }

    if(verticalScrollBar()) {
        offset -= verticalScrollBar()->value();
    }

    return offset;
}

void KoCanvasController::updateCanvasOffsetX() {
    if(m_d->ignoreScrollSignals)
        return;
    // save new preferred x-center
    m_d->preferredCenterFractionX = (0.5 * viewport()->width() - canvasOffsetX() ) / m_d->documentSize.width();
    emit canvasOffsetXChanged(canvasOffsetX());
    m_d->viewportWidget->canvas()->setFocus(); // workaround ugly bug in Qt that the focus is transferred to the sliders
}

void KoCanvasController::updateCanvasOffsetY() {
    if(m_d->ignoreScrollSignals)
        return;
    // save new preferred y-center
    m_d->preferredCenterFractionY = (0.5 * viewport()->height() - canvasOffsetY() ) / m_d->documentSize.height();
    emit canvasOffsetYChanged(canvasOffsetY());
    m_d->viewportWidget->canvas()->setFocus(); // workaround ugly bug in Qt that the focus is transferred to the sliders
}



bool KoCanvasController::eventFilter(QObject* watched, QEvent* event) {
    if(m_d->canvas && m_d->canvas->canvasWidget() && (watched == m_d->canvas->canvasWidget())) {
        if((event->type() == QEvent::Resize) || event->type() == QEvent::Move) {
            updateCanvasOffsetX();
            updateCanvasOffsetY();
        }
        else if ( event->type() == QEvent::MouseMove ) {
            QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent*>( event );
            if ( mouseEvent )
                emit canvasMousePositionChanged( mouseEvent->pos() );
        }
    }
    return false;
}

void KoCanvasController::ensureVisible( KoShape *shape ) {
    Q_ASSERT(shape);
    ensureVisible( shape->boundingRect() );
}

void KoCanvasController::ensureVisible( const QRectF &rect, bool smooth ) {
    QRect currentVisible (qMax(0, -canvasOffsetX()), qMax(0, -canvasOffsetY()), visibleWidth(), visibleHeight());

    // convert the document based rect into a canvas based rect
    QRect viewRect = m_d->canvas->viewConverter()->documentToView( rect ).toRect();
    viewRect.translate( m_d->canvas->documentOrigin() );
    if(! viewRect.isValid() || currentVisible.contains(viewRect))
        return; // its visible. Nothing to do.

    // if we move, we move a little more so the amount of times we have to move is less.
    int jumpWidth = smooth ? 0 : currentVisible.width() / 5;
    int jumpHeight = smooth ? 0 : currentVisible.height() / 5;
    if(!smooth && viewRect.width() + jumpWidth > currentVisible.width())
        jumpWidth = 0;
    if(!smooth && viewRect.height() + jumpHeight > currentVisible.height())
        jumpHeight = 0;

    int horizontalMove = 0;
    if(currentVisible.width() <= viewRect.width())      // center view
        horizontalMove = viewRect.center().x() - currentVisible.center().x();
    else if(currentVisible.x() > viewRect.x())          // move left
        horizontalMove = viewRect.x() - currentVisible.x() - jumpWidth;
    else if(currentVisible.right() < viewRect.right())  // move right
        horizontalMove = viewRect.right() - qMax(0, currentVisible.right() - jumpWidth);

    int verticalMove = 0;
    if(currentVisible.height() <= viewRect.height())       // center view
        verticalMove = viewRect.center().y() - currentVisible.center().y();
    if(currentVisible.y() > viewRect.y())               // move up
        verticalMove = viewRect.y() - currentVisible.y() - jumpHeight;
    else if(currentVisible.bottom() < viewRect.bottom()) // move down
        verticalMove = viewRect.bottom() - qMax(0, currentVisible.bottom() - jumpHeight);

    pan(QPoint(horizontalMove, verticalMove));
}

void KoCanvasController::recenterPreferred()
{
    if(viewport()->width() >= m_d->documentSize.width()
                && viewport()->height() >= m_d->documentSize.height())
        return; // no need to center when image is smaller than viewport

    QPoint center = QPoint(int(m_d->documentSize.width() * m_d->preferredCenterFractionX),
                                        int(m_d->documentSize.height() * m_d->preferredCenterFractionY));

    // convert into a viewport based point
    center.rx() += m_d->canvas->canvasWidget()->x() + frameWidth();
    center.ry() += m_d->canvas->canvasWidget()->y() + frameWidth();

    // calculate the difference to the viewport centerpoint
    QPoint topLeft = center - 0.5 * QPoint( viewport()->width(), viewport()->height() );

    QScrollBar *hBar = horizontalScrollBar();
    // try to centralize the centerpoint which we want to make visible
    {//if( hBar && hBar->isVisible() ) {
        topLeft.rx() = qMax( topLeft.x(), hBar->minimum() );
        topLeft.rx() = qMin( topLeft.x(), hBar->maximum() );
        hBar->setValue( topLeft.x() );
    }
    QScrollBar *vBar = verticalScrollBar();
    {//if( vBar && vBar->isVisible() ) {
        topLeft.ry() = qMax( topLeft.y(), vBar->minimum() );
        topLeft.ry() = qMin( topLeft.y(), vBar->maximum() );
        vBar->setValue( topLeft.y() );
    }
}

void KoCanvasController::zoomIn(const QPoint &center)
{
    m_d->preferredCenterFractionX = 1.0 * center.x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * center.y() / m_d->documentSize.height();

    emit zoomBy(sqrt(2.0));
    recenterPreferred();
}

void KoCanvasController::zoomOut(const QPoint &center)
{
    m_d->preferredCenterFractionX = 1.0 * center.x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * center.y() / m_d->documentSize.height();

    emit zoomBy(sqrt(0.5));
    recenterPreferred();
}

void KoCanvasController::zoomTo(const QRect &viewRect)
{
    double scale;

    if(1.0 * viewport()->width() / viewRect.width() > 1.0 * viewport()->height() / viewRect.height())
        scale = 1.0 * viewport()->height() / viewRect.height();
    else
        scale = 1.0 * viewport()->width() / viewRect.width();

    m_d->preferredCenterFractionX = 1.0 * viewRect.center().x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * viewRect.center().y() / m_d->documentSize.height();

    emit zoomBy(scale);
    recenterPreferred();
}

void KoCanvasController::setToolOptionWidget(QWidget *widget) {
//   if(m_toolOptionWidget)
//       m_toolOptionWidget->deleteLater();
    m_d->toolOptionWidget = widget;
    emit toolOptionWidgetChanged(m_d->toolOptionWidget);
}


void KoCanvasController::setDocumentSize( const QSize & sz )
{
    m_d->ignoreScrollSignals = true;
    m_d->documentSize = sz;
    m_d->viewportWidget->setDocumentSize( sz );
    resetScrollBars();
    m_d->ignoreScrollSignals = false;
}

void KoCanvasController::setDocumentOffset()
{
    // The margins scroll the canvas widget inside the viewport, not
    // the document. The documentOffset is meant the be the value that
    // the canvas must add to the update rect in its paint event, to
    // compensate.

    QPoint pt( horizontalScrollBar()->value(), verticalScrollBar()->value() );
    if ( pt.x() < m_d->margin ) pt.setX( 0 );
    if ( pt.y() < m_d->margin ) pt.setY( 0 );
    if ( pt.x() > m_d->documentSize.width() ) pt.setX( m_d->documentSize.width() );
    if ( pt.y() > m_d->documentSize.height() ) pt.setY( m_d->documentSize.height() );
    emit( moveDocumentOffset( pt ) );

    QWidget *canvasWidget = m_d->canvas->canvasWidget();

    if (canvasWidget) {
        if (!canvasIsOpenGL()) {
            QPoint diff = m_d->documentOffset - pt;
            canvasWidget->scroll(diff.x(), diff.y());
        }
    }

    m_d->documentOffset = pt;
}

bool KoCanvasController::canvasIsOpenGL() const
{
    QWidget *canvasWidget = m_d->canvas->canvasWidget();

    if (canvasWidget) {
#ifdef HAVE_OPENGL
        if (qobject_cast<QGLWidget*>(canvasWidget) != 0) {
            return true;
        }
#endif
    }

    return false;
}

void KoCanvasController::resetScrollBars()
{
    // The scrollbar value always points at the top-left corner of the
    // bit of image we paint.

    int docH = m_d->documentSize.height() + m_d->margin;
    int docW = m_d->documentSize.width() + m_d->margin;
    int drawH = m_d->viewportWidget->height();
    int drawW = m_d->viewportWidget->width();

    QScrollBar * hScroll = horizontalScrollBar();
    QScrollBar * vScroll = verticalScrollBar();

    if (docH <= drawH && docW <= drawW) {
        // we need no scrollbars
        vScroll->setRange( 0, 0 );
        hScroll->setRange( 0, 0 );
    } else if (docH <= drawH) {
        // we need a horizontal scrollbar only
        vScroll->setRange( 0, 0 );
        hScroll->setRange(0, docW - drawW);
    } else if(docW <= drawW) {
        // we need a vertical scrollbar only
        hScroll->setRange( 0, 0 );
        vScroll->setRange(0, docH - drawH);
    } else {
        // we need both scrollbars
        vScroll->setRange(0, docH - drawH);
        hScroll->setRange(0, docW - drawW);
    }

    int fontheight = QFontMetrics(font()).height();
    vScroll->setPageStep(drawH);
    vScroll->setSingleStep(fontheight);
    hScroll->setPageStep(drawW);
    hScroll->setSingleStep(fontheight);

}

void KoCanvasController::pan(const QPoint distance) {
    QScrollBar *hBar = horizontalScrollBar();
    if( hBar && hBar->isVisible() )
        hBar->setValue( hBar->value() + distance.x());
    QScrollBar *vBar = verticalScrollBar();
    if( vBar && vBar->isVisible() )
        vBar->setValue( vBar->value() + distance.y());
}

void KoCanvasController::setPreferredCenter( const QPoint &viewPoint )
{
    m_d->preferredCenterFractionX = 1.0 * viewPoint.x() / m_d->documentSize.width();
    m_d->preferredCenterFractionY = 1.0 * viewPoint.y() / m_d->documentSize.height();
    recenterPreferred();
}

// XXX: Apparently events are not propagated to the viewport widget by
// QAbstractScrollArea

void KoCanvasController::paintEvent( QPaintEvent * event )
{
    QPainter gc( viewport() );
    m_d->viewportWidget->handlePaintEvent( gc, event );
}

void KoCanvasController::dragEnterEvent( QDragEnterEvent * event )
{
    m_d->viewportWidget->handleDragEnterEvent( event );
}

void KoCanvasController::dropEvent( QDropEvent *event )
{
    m_d->viewportWidget->handleDropEvent( event );
}

void KoCanvasController::dragMoveEvent ( QDragMoveEvent *event )
{
    m_d->viewportWidget->handleDragMoveEvent( event );
}

void KoCanvasController::dragLeaveEvent( QDragLeaveEvent *event )
{
    m_d->viewportWidget->handleDragLeaveEvent( event );
}

#include "KoCanvasController.moc"
