/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
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

class KoCanvasController::Private
{
public:
    KoCanvasBase * canvas;
    bool centerCanvas;
    QWidget * toolOptionWidget;
    int margin; // The viewport margin around the document
    QSize documentSize;
    QPoint documentOffset;
    Viewport * viewportWidget;
};

KoCanvasController::KoCanvasController(QWidget *parent)
    : QAbstractScrollArea(parent),
    m_d(new Private())
{
    m_d->canvas = 0;
    m_d->toolOptionWidget = 0;

    setFrameShape(NoFrame);
    m_d->viewportWidget = new Viewport( this );
    setViewport(m_d->viewportWidget);

    setAutoFillBackground(false);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));

    setMouseTracking( true );

    KSharedConfig::Ptr cfg = KGlobal::config();
    cfg->setGroup("");
    m_d->margin = cfg->readEntry("canvasmargin",  0);

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
    }
    m_d->viewportWidget->setCanvas(canvas->canvasWidget());
    m_d->canvas = canvas;
    m_d->canvas->canvasWidget()->installEventFilter(this);
    m_d->canvas->canvasWidget()->setMouseTracking( true );

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

bool KoCanvasController::isCanvasCentered() const {
    return m_d->centerCanvas;
}

void KoCanvasController::centerCanvas(bool centered)
{
    // XXX: Move the document so it's midpoint is in the middle
    Q_UNUSED( centered );
    m_d->centerCanvas = true;
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
    emit canvasOffsetXChanged(canvasOffsetX());
}

void KoCanvasController::updateCanvasOffsetY() {
    emit canvasOffsetYChanged(canvasOffsetY());
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

void KoCanvasController::ensureVisible( const QRectF &rect ) {
    // convert the document based rect into a canvas based rect
    QRect viewRect = m_d->canvas->viewConverter()->documentToView( rect ).toRect();

    // calculate position of the centerpoint of the rect we want to make visible
    QPoint cp = viewRect.center() + m_d->canvas->documentOrigin();
    cp.rx() += m_d->canvas->canvasWidget()->x() + frameWidth();
    cp.ry() += m_d->canvas->canvasWidget()->y() + frameWidth();

    // calculate the difference to the viewport centerpoint
    QPoint centerDiff = cp - 0.5 * QPoint( viewport()->width(), viewport()->height() );

    QScrollBar *hBar = horizontalScrollBar();
    // try to centralize the centerpoint of the rect which we want to make visible
    if( hBar && hBar->isVisible() ) {
        centerDiff.rx() += int( 0.5 * (float)hBar->maximum() );
        centerDiff.rx() = qMax( centerDiff.x(), hBar->minimum() );
        centerDiff.rx() = qMin( centerDiff.x(), hBar->maximum() );
        hBar->setValue( centerDiff.x() );
    }
    QScrollBar *vBar = verticalScrollBar();
    if( vBar && vBar->isVisible() ) {
        centerDiff.ry() += int( 0.5 * (float)vBar->maximum() );
        centerDiff.ry() = qMax( centerDiff.y(), vBar->minimum() );
        centerDiff.ry() = qMin( centerDiff.y(), vBar->maximum() );
        vBar->setValue( centerDiff.y() );
    }
}

void KoCanvasController::zoomIn(const QPointF &center)
{
    // convert the document based point into a canvas based point
    QPoint cp = m_d->canvas->viewConverter()->documentToView( center ).toPoint() + m_d->canvas->documentOrigin();
    cp.rx() += m_d->canvas->canvasWidget()->x() + frameWidth();
    cp.ry() += m_d->canvas->canvasWidget()->y() + frameWidth();

    // calculate the difference to the viewport centerpoint
    QPoint centerDiff = cp - 0.5 * QPoint( viewport()->width(), viewport()->height() );

    QScrollBar *hBar = horizontalScrollBar();
    // try to centralize the centerpoint which we want to make visible
    if( hBar && hBar->isVisible() ) {
        centerDiff.rx() += int( 0.5 * (float)hBar->maximum() );
        centerDiff.rx() = qMax( centerDiff.x(), hBar->minimum() );
        centerDiff.rx() = qMin( centerDiff.x(), hBar->maximum() );
        hBar->setValue( centerDiff.x() );
    }
    QScrollBar *vBar = verticalScrollBar();
    if( vBar && vBar->isVisible() ) {
        centerDiff.ry() += int( 0.5 * (float)vBar->maximum() );
        centerDiff.ry() = qMax( centerDiff.y(), vBar->minimum() );
        centerDiff.ry() = qMin( centerDiff.y(), vBar->maximum() );
        vBar->setValue( centerDiff.y() );
    }
}

void KoCanvasController::zoomOut(const QPointF &center)
{
}

void KoCanvasController::zoomTo(const QRectF &rect)
{
}

void KoCanvasController::setToolOptionWidget(QWidget *widget) {
//   if(m_toolOptionWidget)
//       m_toolOptionWidget->deleteLater();
    m_d->toolOptionWidget = widget;
    emit toolOptionWidgetChanged(m_d->toolOptionWidget);
}


void KoCanvasController::setDocumentSize( const QSize & sz )
{
    m_d->documentSize = sz;
    m_d->viewportWidget->setDocumentSize( sz );
    resetScrollBars();
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

    if(m_d->canvas->canvasWidget()) {
        QPoint diff = m_d->documentOffset - pt;
        m_d->canvas->canvasWidget()->scroll(diff.x(), diff.y());
    }

    m_d->documentOffset = pt;
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
