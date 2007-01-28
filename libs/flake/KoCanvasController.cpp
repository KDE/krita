/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#include <QUndoCommand>
#include <QMouseEvent>
#include <QGridLayout>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <limits.h>

KoCanvasController::KoCanvasController(QWidget *parent)
    : QScrollArea(parent)
    , m_canvas(0)
    , m_toolOptionDocker(0)
{
    setFrameShape(NoFrame);
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
    if(m_canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int height1;
    if(canvasWidget == 0)
        height1 = m_viewport->height();
    else
        height1 = qMin(m_viewport->height(), canvasWidget->height());
    int height2 = height();
    return qMin(height1, height2);
}

int KoCanvasController::visibleWidth() const {
    if(m_canvas == 0)
        return 0;
    QWidget *canvasWidget = canvas()->canvasWidget();

    int width1;
    if(canvasWidget == 0)
        width1 = m_viewport->width();
    else
        width1 = qMin(m_viewport->width(), canvasWidget->width());
    int width2 = width();
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

void KoCanvasController::resizeEvent(QResizeEvent * resizeEvent) {
    QScrollArea::resizeEvent(resizeEvent);
    emit sizeChanged(resizeEvent->size());
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

QPointF KoCanvasController::Viewport::correctPosition(const QPoint &point) const {
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    QPoint correctedPos(point.x() - canvasWidget->x(), point.y() - canvasWidget->y());
    correctedPos -= m_parent->canvas()->documentOrigin();
    return m_parent->canvas()->viewConverter()->viewToDocument(correctedPos);
}

void KoCanvasController::Viewport::dragMoveEvent (QDragMoveEvent *event) {
    if(m_draggedShape == 0)
        return;
    m_draggedShape->repaint();
    repaint(m_draggedShape);
    m_draggedShape->setAbsolutePosition( correctPosition(event->pos()) );
    m_draggedShape->repaint();
    repaint(m_draggedShape);
}

void KoCanvasController::Viewport::repaint(KoShape *shape) {
    QRect rect = m_parent->canvas()->viewConverter()->documentToView(shape->boundingRect()).toRect();
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    rect.moveLeft(rect.left() + canvasWidget->x());
    rect.moveTop(rect.top() + canvasWidget->y());
    rect.adjust(-2, -2, 2, 2); // adjust for antialias
    update(rect);
}

void KoCanvasController::Viewport::dragLeaveEvent(QDragLeaveEvent *) {
    if(m_draggedShape) {
        repaint(m_draggedShape);
        m_parent->canvas()->shapeManager()->remove(m_draggedShape);
        delete m_draggedShape;
        m_draggedShape = 0;
    }
}

void KoCanvasController::Viewport::paintEvent(QPaintEvent *event) {
    QPainter painter( this );
    painter.setClipRect(event->rect());
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

    painter.end();
}

#include "KoCanvasController.moc"
