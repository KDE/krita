/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thoresten Zachmann <zachmann@kde.org>
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

#include <KoProperties.h>

#include <kdebug.h>
#include <kcommand.h>
#include <QMouseEvent>
#include <QGridLayout>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <limits.h>

KoCanvasController::KoCanvasController(QWidget *parent)
    : QScrollArea(parent)
    , m_canvas(0)
    , m_canvasWidget(0)
    , m_toolOptionDocker(0)
{
    m_viewport = new Viewport(this);
    setWidget(m_viewport);
    setWidgetResizable(true);
    setAutoFillBackground(false);
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));

    setMouseTracking( true );

}

void KoCanvasController::setCanvas(KoCanvasBase *canvas) {
    Q_ASSERT(canvas); // param is not null
    if(m_canvas) {
        emit canvasRemoved(this);
        m_viewport->removeCanvas(m_canvas->canvasWidget());
    }
    m_viewport->setCanvas(canvas->canvasWidget());
    m_canvas = canvas;
    m_canvas->canvasWidget()->installEventFilter(this);
    m_canvas->canvasWidget()->setMouseTracking( true );

    emit canvasSet(this);
}

KoCanvasBase* KoCanvasController::canvas() const {
    return m_canvas;
}

int KoCanvasController::visibleHeight() const {
    int height1;
    if(m_canvasWidget == 0)
        height1 = m_viewport->height();
    else
        height1 = qMin(m_viewport->height(), m_canvasWidget->height());
    int height2 = height();
    if(horizontalScrollBar() && horizontalScrollBar()->isVisible())
        height2 -= horizontalScrollBar()->height();
    return qMin(height1, height2);
}

int KoCanvasController::visibleWidth() const {
    int width1;
    if(m_canvasWidget == 0)
        width1 = m_viewport->width();
    else
        width1 = qMin(m_viewport->width(), m_canvasWidget->width());
    int width2 = width();
    if(verticalScrollBar() && verticalScrollBar()->isVisible())
        width2 -= verticalScrollBar()->width();
    return qMin(width1, width2);
}

void KoCanvasController::centerCanvas(bool centered) {
    m_centerCanvas = centered;
    m_viewport->centerCanvas(centered);
}

bool KoCanvasController::isCanvasCentered() const {
    return m_centerCanvas;
}

int KoCanvasController::canvasOffsetX() const {
    int offset = 0;

    if(m_canvas) {
        offset = m_canvas->canvasWidget()->x() + frameWidth();
    }

    if(horizontalScrollBar()) {
        offset -= horizontalScrollBar()->value();
    }

    return offset;
}

int KoCanvasController::canvasOffsetY() const {
    int offset = 0;

    if(m_canvas) {
        offset = m_canvas->canvasWidget()->y() + frameWidth();
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
    if(m_canvas && m_canvas->canvasWidget() && (watched == m_canvas->canvasWidget())) {
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
    QRect viewRect = m_canvas->viewConverter()->documentToView( rect ).toRect();

    // calculate position of the centerpoint of the rect we want to make visible
    QPoint cp = viewRect.center() + m_canvas->documentOrigin();
    cp.rx() += m_canvas->canvasWidget()->x() + frameWidth();
    cp.ry() += m_canvas->canvasWidget()->y() + frameWidth();

    // calculate the differance to the viewport centerpoint
    QPoint centerDiff = cp - 0.5 * QPoint( m_viewport->width(), m_viewport->height() );

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


// ********** Viewport **********
KoCanvasController::Viewport::Viewport(KoCanvasController* parent)
: QWidget(parent)
, m_draggedShape(0)
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(false);
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    centerCanvas(true);
    setAcceptDrops(true);
    m_parent = parent;
}

void KoCanvasController::Viewport::setCanvas(QWidget *canvas) {
    m_layout->addWidget(canvas, 1, 1, Qt::AlignHCenter | Qt::AlignVCenter);
}

void KoCanvasController::Viewport::removeCanvas(QWidget *canvas) {
    m_layout->removeWidget(canvas);
}

void KoCanvasController::Viewport::centerCanvas(bool centered) {
    m_layout->setColumnStretch(0,centered?1:0);
    m_layout->setColumnStretch(1,1);
    m_layout->setColumnStretch(2,centered?1:2);
    m_layout->setRowStretch(0,centered?1:0);
    m_layout->setRowStretch(1,1);
    m_layout->setRowStretch(2,centered?1:2);
}

void KoCanvasController::Viewport::dragEnterEvent(QDragEnterEvent *event) {
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
            kWarning(30001) << "Application requested a shape that is not registered '" <<
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

void KoCanvasController::Viewport::dropEvent(QDropEvent *event) {
    m_draggedShape->setAbsolutePosition( corrrectPosition(event->pos()) );
    m_parent->canvas()->shapeManager()->remove(m_draggedShape); // remove it to not interfere with z-index calc.
    KCommand * cmd = m_parent->canvas()->shapeController()->addShape( m_draggedShape );
    if(cmd) {
        cmd->execute();
        m_parent->canvas()->addCommand(cmd);
    }
    m_draggedShape = 0;
}

QPointF KoCanvasController::Viewport::corrrectPosition(const QPoint &point) const {
    QPoint correctedPos(point.x() - m_parent->canvasOffsetX(), point.y() - m_parent->canvasOffsetY());
    return m_parent->canvas()->viewConverter()->viewToDocument(correctedPos);
}

void KoCanvasController::Viewport::dragMoveEvent (QDragMoveEvent *event) {
    if(m_draggedShape == 0)
        return;
    m_draggedShape->repaint();
    repaint(m_draggedShape);
    m_draggedShape->setAbsolutePosition( corrrectPosition(event->pos()) );
    m_draggedShape->repaint();
    repaint(m_draggedShape);
}

void KoCanvasController::Viewport::repaint(KoShape *shape) {
    QRect rect = m_parent->canvas()->viewConverter()->documentToView(shape->boundingRect()).toRect();
    rect.moveLeft(rect.left() + m_parent->canvasOffsetX());
    rect.moveTop(rect.top() + m_parent->canvasOffsetY());
    rect.adjust(-2, -2, 2, 2); // update for antialias
    update(rect);
}

void KoCanvasController::Viewport::dragLeaveEvent(QDragLeaveEvent *) {
    if(m_draggedShape) {
        m_parent->canvas()->shapeManager()->remove(m_draggedShape);
        delete m_draggedShape;
        m_draggedShape = 0;
    }
}

void KoCanvasController::Viewport::paintEvent(QPaintEvent *event) {
    if(m_draggedShape) {
        KoViewConverter *vc = m_parent->canvas()->viewConverter();

        QPainter painter( this );
        painter.setClipRect(event->rect());
        painter.translate(m_parent->canvasOffsetX(), m_parent->canvasOffsetY());
        QPointF offset = vc->documentToView(m_draggedShape->position());
        painter.translate(offset.x(), offset.y());
        m_draggedShape->paint(painter, *vc);
        painter.end();
    }
}


#include "KoCanvasController.moc"

// TODO add a paintEvent here and optionally paint a nice shadow to the
// bottom/right of the canvas.
