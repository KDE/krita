/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoShapeFactory.h" // for the SHAPE mimetypes
#include "KoShapeRegistry.h"
#include "KoShapeController.h"
#include "KoShapeManager.h"
#include "KoSelection.h"

#include <KoProperties.h>

#include <kdebug.h>
#include <ksharedconfig.h>

#include <QUndoCommand>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <QSize>
#include <QPoint>

#include <limits.h>

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
    : QAbstractScrollArea(parent)
{
    m_d = new Private();
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
    m_d->margin = cfg->readEntry("canvasmargin",  30);

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

    // calculate the differance to the viewport centerpoint
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

    // XXX: The singlestep should be configurable per app?
    //int fontheight = QFontMetrics(KGlobalSettings::generalFont()).height() * 3;
    int fontheight = 40;
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


// ********** Viewport **********
Viewport::Viewport(KoCanvasController* parent)
    : QWidget(parent)
    , m_draggedShape(0)
    , m_canvas( 0 )
    , m_documentOffset( QPoint( 0, 0 ) )
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setAcceptDrops(true);
    setMouseTracking( true );
    m_parent = parent;

    KSharedConfig::Ptr cfg = KGlobal::config();
    cfg->setGroup("");
    m_margin = cfg->readEntry("canvasmargin",  30);

}

void Viewport::setCanvas(QWidget *canvas)
{
    if ( !canvas ) return;
    // XXX: Should we delete the old canvas if we set a new one, or
    // leave that to the owning application?
    if ( m_canvas ) {
        m_canvas->hide();
        delete m_canvas;
    }
    m_canvas = canvas;
    m_canvas->setParent( this );
    resetLayout();
}

void Viewport::setDocumentSize( QSize size )
{
    m_documentSize = size;
    resetLayout();
}

void Viewport::documentOffsetMoved( QPoint pt )
{
    m_documentOffset = pt;
    resetLayout();
}


void Viewport::handleDragEnterEvent(QDragEnterEvent *event)
{
    // if not a canvas set then ignore this, makes it possible to assume
    // we have a canvas in all the support methods.
    if(! (m_parent->canvas() && m_parent->canvas()->canvasWidget()))
        return;
    if (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE) ||
            event->mimeData()->hasFormat(SHAPEID_MIMETYPE)) {

        QByteArray itemData;
        bool isTemplate = true;
        if (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE))
            itemData = event->mimeData()->data(SHAPETEMPLATE_MIMETYPE);
        else {
            isTemplate = false;
            itemData = event->mimeData()->data(SHAPEID_MIMETYPE);
        }
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QString id;
        dataStream >> id;
        QString properties;
        if(isTemplate)
            dataStream >> properties;

        // and finally, there is a point.
        QPointF offset;
        dataStream >> offset;

        // The rest of this method is mostly a copy paste from the KoCreateShapeStrategy
        // So, lets remove this again when Zagge adds his new class that does this kind of thing. (KoLoadSave)
        KoShapeFactory *factory = KoShapeRegistry::instance()->get(id);
        if(! factory) {
            kWarning(30006) << "Application requested a shape that is not registered '" <<
                id << "', Ignoring" << endl;
            event->ignore();
            return;
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();

        if(isTemplate) {
            KoProperties props;
            props.load(properties);
            m_draggedShape = factory->createShape(&props);
        }
        else
            m_draggedShape = factory->createDefaultShape();
        if( m_draggedShape->shapeId().isEmpty() )
            m_draggedShape->setShapeId(factory->shapeId());
        m_draggedShape->setZIndex(INT_MAX);

        m_parent->canvas()->shapeManager()->add(m_draggedShape);
    }
}

void Viewport::handleDropEvent(QDropEvent *event) {
    m_draggedShape->setAbsolutePosition( correctPosition(event->pos()) );
    m_parent->canvas()->shapeManager()->remove(m_draggedShape); // remove it to not interfere with z-index calc.
    QUndoCommand * cmd = m_parent->canvas()->shapeController()->addShape( m_draggedShape );
    if(cmd) {
        m_parent->canvas()->addCommand(cmd);
        KoSelection *selection = m_parent->canvas()->shapeManager()->selection();
        selection->deselectAll();
        selection->select(m_draggedShape);
    }
    else
        delete m_draggedShape;
    m_draggedShape = 0;
}

QPointF Viewport::correctPosition(const QPoint &point) const {
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    QPoint correctedPos(point.x() - canvasWidget->x(), point.y() - canvasWidget->y());
    correctedPos -= m_parent->canvas()->documentOrigin();
    return m_parent->canvas()->viewConverter()->viewToDocument(correctedPos);
}

void Viewport::handleDragMoveEvent (QDragMoveEvent *event) {
    if(m_draggedShape == 0)
        return;
    m_draggedShape->repaint();
    repaint(m_draggedShape);
    m_draggedShape->setAbsolutePosition( correctPosition(event->pos()) );
    m_draggedShape->repaint();
    repaint(m_draggedShape);
}

void Viewport::repaint(KoShape *shape) {
    QRect rect = m_parent->canvas()->viewConverter()->documentToView(shape->boundingRect()).toRect();
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    rect.moveLeft(rect.left() + canvasWidget->x());
    rect.moveTop(rect.top() + canvasWidget->y());
    rect.adjust(-2, -2, 2, 2); // adjust for antialias
    update();
}

void Viewport::handleDragLeaveEvent(QDragLeaveEvent *) {
    if(m_draggedShape) {
        repaint(m_draggedShape);
        m_parent->canvas()->shapeManager()->remove(m_draggedShape);
        delete m_draggedShape;
        m_draggedShape = 0;
    }
}

void Viewport::handlePaintEvent(QPainter & painter, QPaintEvent *event)
{

    // XXX: This is the shadow, right?
    if(false && m_parent->canvas() && m_parent->canvas()->canvasWidget()) {
        QWidget *canvas = m_parent->canvas()->canvasWidget();
        painter.setPen(Qt::black);
        QRect rect(canvas->x(), canvas->y(), canvas->width(), canvas->height());
        rect.adjust(-1, -1, 0, 0);
        painter.drawRect(rect);
        painter.drawLine(rect.right()+2, rect.top()+2, rect.right()+2, rect.bottom()+2);
        painter.drawLine(rect.left()+2, rect.bottom()+2, rect.right()+2, rect.bottom()+2);
    }
    if(m_draggedShape) {
        KoViewConverter *vc = m_parent->canvas()->viewConverter();

        painter.save();
        QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
        Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
        painter.translate(canvasWidget->x(), canvasWidget->y());
        QPointF offset = vc->documentToView(m_draggedShape->position());
        painter.setOpacity(0.6);
        painter.translate(offset.x(), offset.y());
        painter.setRenderHint(QPainter::Antialiasing);
        m_draggedShape->paint(painter, *vc);
        painter.restore();
    }

}

void Viewport::resetLayout()
{
    // Determine the area we have to show
    QRect viewRect( m_documentOffset, size() );

    int viewH = viewRect.height();
    int viewW = viewRect.width();

    int docH = m_documentSize.height();
    int docW = m_documentSize.width();

    int moveX = 0;
    int moveY = 0;

    int resizeW = viewW;
    int resizeH = viewH;

//     kDebug() << "viewH: " << viewH << endl
//              << "docH: " << docH << endl
//              << "viewW: " << viewW << endl
//              << "docW: " << docW << endl;

    if ( viewH == docH && viewW == docW )
    {
        // Do nothing
        resizeW = docW;
        resizeH = docH;
    }
    else if ( viewH > docH && viewW > docW )
    {
        // Show entire canvas centered
        moveX = ( viewW - docW ) / 2;
        moveY = ( viewH - docH ) / 2;
        resizeW = docW;
        resizeH = docH;
    }
    else  if ( viewH < docH && viewW > docW ) {
        // Center canvas horizontally
        moveX = ( viewW - docW ) / 2;
        resizeW = docW;

        int marginTop = m_margin - m_documentOffset.y();
        int marginBottom = viewH  - ( m_documentSize.height() - m_documentOffset.y() );

        if ( marginTop > 0 ) moveY = marginTop;
        if ( marginTop > 0 ) resizeH = viewH - marginTop;
        if ( marginBottom > 0 ) resizeH = viewH - marginBottom;
    }
    else  if ( viewW < docW && viewH > docH ) {
        // Center canvas vertically
        moveY = ( viewH - docH ) / 2;
        resizeH = docH;

        int marginLeft = m_margin - m_documentOffset.x();
        int marginRight = viewW - ( m_documentSize.width() - m_documentOffset.x() );

        if ( marginLeft > 0 ) moveX = marginLeft;
        if ( marginLeft > 0 ) resizeW = viewW - marginLeft;
        if ( marginRight > 0 ) resizeW = viewW - marginRight;

    }
    else {
        // Take care of the margin around the canvas
        int marginTop = m_margin - m_documentOffset.y();
        int marginLeft = m_margin - m_documentOffset.x();
        int marginRight = viewW - ( m_documentSize.width() - m_documentOffset.x() );
        int marginBottom = viewH  - ( m_documentSize.height() - m_documentOffset.y() );

        if ( marginTop > 0 ) moveY = marginTop;
        if ( marginLeft > 0 ) moveX = marginLeft;

        if ( marginTop > 0 ) resizeH = viewH - marginTop;
        if ( marginLeft > 0 ) resizeW = viewW - marginLeft;
        if ( marginRight > 0 ) resizeW = viewW - marginRight;
        if ( marginBottom > 0 ) resizeH = viewH - marginBottom;

    }
    m_canvas->setGeometry( moveX, moveY, resizeW, resizeH );

//     kDebug() << "View port geom: " << geometry() << endl;
//     kDebug() << "Canvas widget geom: " << m_canvas->geometry() << endl;
}

#include "KoCanvasController.moc"
