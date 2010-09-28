/*
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
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

#include "Canvas.h"
#include "ClipboardProxyShape.h"
#include "DragCanvasStrategy.h"
#include "FolderShape.h"
#include "IconShape.h"
#include "MoveFolderStrategy.h"
#include "ResizeFolderStrategy.h"
#include "RightClickStrategy.h"
#include "SelectStrategy.h"
#include "ShapeSelector.h"

#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeFactoryBase.h> // for the mimetype defines etc

#include <QApplication>
#include <QToolTip>
#include <QUndoCommand>
#include <QPainter>
#include <QMenu>
#include <QTimer>

#include <KUrl>

Canvas::Canvas(ShapeSelector *parent, ItemStore *itemStore)
    : QWidget(parent),
    KoCanvasBase(itemStore->shapeController()),
    m_parent(parent),
    m_currentStrategy(0),
    m_zoomIndex(1),
    m_previousFocusOwner(0)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setAcceptDrops(true);
    setMinimumSize(32, 42); // based on the fact that an IconShape is 22x22
    QTimer::singleShot(0, this, SLOT(loadShapeTypes()));

    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*, QWidget*)),
        this, SLOT(focusChanged(QWidget*, QWidget*)));
}

void Canvas::gridSize(qreal *, qreal *) const
{
}

void Canvas::updateCanvas(const QRectF &rc)
{
    QRectF zoomedRect = rc;
    zoomedRect.moveTopLeft(zoomedRect.topLeft() - m_displayOffset);
    QRectF clipRect = m_converter.documentToView(zoomedRect);
    clipRect.adjust(-2, -2, 2, 2); // grow for anti-aliasing
    update(clipRect.toRect());
}

void  Canvas::addCommand(QUndoCommand *command)
{
    command->redo();
    delete command;
}

void Canvas::zoomIn(const QPointF &center)
{
    m_converter.setZoomIndex(++m_zoomIndex);
    QSizeF docSize = m_converter.viewToDocument(size());
    m_displayOffset = QPointF(center.x() - docSize.width() / 2.0, center.y() - docSize.height() / 2.0);
    update();
}

void Canvas::zoomOut(const QPointF &center)
{
    m_converter.setZoomIndex(--m_zoomIndex);
    QSizeF docSize = m_converter.viewToDocument(size());
    m_displayOffset = QPointF(center.x() - docSize.width() / 2.0, center.y() - docSize.height() / 2.0);
    update();
}

QAction *Canvas::popup(QMenu *menu, const QPointF &docCoordinate)
{
    return menu->exec(mapToGlobal(m_converter.documentToView(docCoordinate - m_displayOffset).toPoint()));
}

void Canvas::moveDocumentOffset(const QPointF &offset)
{
    m_displayOffset -= offset;
    QPointF distance = m_converter.documentToView(offset);
    scroll(qRound(distance.x()), qRound(distance.y()));
}

void Canvas::resetDocumentOffset()
{
    m_displayOffset = QPointF();
    update();
}

// event handlers
void Canvas::mousePressEvent(QMouseEvent *event)
{
    KoPointerEvent pe(event, m_displayOffset + m_converter.viewToDocument(event->pos()));
    m_lastPoint = pe.point;
    KoShape *clickedShape = 0;
    foreach(KoShape *shape, shapeManager()->shapesAt(QRectF(pe.point, QSizeF(1,1)))) {
        FolderShape *folder = dynamic_cast<FolderShape*>(shape);
        if ((event->buttons() & Qt::LeftButton) && itemStore()->mainFolder() == 0 && folder) {
            QPointF localPoint = pe.point - folder->position();
            if (localPoint.x() <= 5 || localPoint.x() >= folder->size().width() - 10
                || localPoint.y() >= folder->size().height() - 5) {
                m_currentStrategy = new ResizeFolderStrategy(this, folder, pe);
                return;
            }
            if (localPoint.y() <= 0) {
                m_currentStrategy = new MoveFolderStrategy(this, folder, pe);
                return;
            }
            continue;
        }
        else if (folder && (event->buttons() & Qt::RightButton)) {
            m_currentStrategy = new RightClickStrategy(this, folder, pe);
            return;
        }
        if (folder == 0) {
            clickedShape = shape;
            break;
        }
    }
    if (event->buttons() == Qt::LeftButton) {
        if (clickedShape) {
            SelectStrategy *ss = new SelectStrategy(this, clickedShape, pe);
            connect (ss, SIGNAL(itemSelected()), m_parent, SLOT(itemSelected()));
            m_currentStrategy = ss;
        } else if (itemStore()->mainFolder() == 0) {
            m_currentStrategy = new DragCanvasStrategy(this, pe);
        }
    } else if (event->buttons() == Qt::RightButton) {
        m_currentStrategy = new RightClickStrategy(this, clickedShape, pe);
    } else {
        event->ignore();
    }

    if (m_currentStrategy)
        setFocusPolicy(Qt::ClickFocus);
}

void Canvas::tabletEvent(QTabletEvent *event)
{
    event->ignore(); // if not accepted it will fall through and be offered as a mouseMoveEvent
    if (event->type() != QEvent::TabletMove)
        return;
    KoShape *clickedShape = shapeManager()->selection()->firstSelectedShape();
    if (clickedShape) {
        QPoint distance = m_converter.documentToView(clickedShape->position()).toPoint() - event->pos();
        if (qAbs(distance.x()) < 15 && qAbs(distance.y()) < 15) // filter out tablet events that don't move enough
            event->accept();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    m_lastPoint = m_displayOffset + m_converter.viewToDocument(event->pos());
    if (m_currentStrategy)
        m_currentStrategy->handleMouseMove(m_lastPoint, event->modifiers());
    else
        event->ignore();
}

void  Canvas::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this && (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE) ||
                event->mimeData()->hasFormat(SHAPEID_MIMETYPE) ||
                event->mimeData()->hasFormat(OASIS_MIME) ||
                event->mimeData()->hasFormat(FOLDERSHAPE_MIMETYPE))) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("text/uri-list")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    m_lastPoint = m_displayOffset + m_converter.viewToDocument(event->pos());
    if (m_currentStrategy == 0) {
        event->ignore();
        return;
    }
    m_currentStrategy->finishInteraction(event->modifiers());
    delete m_currentStrategy;
    m_currentStrategy = 0;
    if (hasFocus() && m_previousFocusOwner)
        m_previousFocusOwner->setFocus();
    setFocusPolicy(Qt::NoFocus);
}

void  Canvas::dropEvent(QDropEvent *event)
{
    QByteArray itemData;
    bool isTemplate = true;
    if (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE)) {
        itemData = event->mimeData()->data(SHAPETEMPLATE_MIMETYPE);
    } else if (event->mimeData()->hasFormat(SHAPEID_MIMETYPE)) {
        isTemplate = false;
        itemData = event->mimeData()->data(SHAPEID_MIMETYPE);
    } else if (event->mimeData()->hasFormat(FOLDERSHAPE_MIMETYPE)) {
        isTemplate = false;
        itemData = event->mimeData()->data(FOLDERSHAPE_MIMETYPE);
    } else if (event->mimeData()->hasFormat(OASIS_MIME)) {
        isTemplate = false;
        itemData = event->mimeData()->data(OASIS_MIME);
    }
    else { // "text/uri-list"
        QRectF hitArea(m_displayOffset + viewConverter()->viewToDocument(event->pos()), QSizeF(1,1));
        FolderShape *folder = 0;
        foreach (KoShape *shape, shapeManager()->shapesAt(hitArea)) {
            folder = dynamic_cast<FolderShape*>(shape);
            if (folder) break;
        }
        QByteArray urls = event->mimeData()->data("text/uri-list");
        foreach (QString file, QString(urls).split('\n')) {
            file = file.trimmed();
            if (file.isEmpty())
                break;
            m_parent->addItems(KUrl(file), folder);
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
        return;
    }
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QString dummy;
    dataStream >> dummy;
    if (isTemplate) {
        // a template additionally has a properties object. Lets get rid of that.
        QString properties;
        dataStream >> properties;
    }

    // and finally, there is a point.
    QPointF offset;
    dataStream >> offset;

    event->setDropAction(Qt::MoveAction);
    event->accept();
    QPointF point = m_displayOffset + m_converter.viewToDocument(event->pos());
    foreach (KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        shape->update();
        if (dynamic_cast<FolderShape*>(shape)) { // is a folder
            shape->setPosition(point - offset);
        }
        else { // is an icon.
            QList<KoShape*> shapes = shapeManager()->shapesAt(QRectF(point - offset, QSizeF(0.1, 0.1)));
            foreach(KoShape *s, shapes) {
                FolderShape *folder = dynamic_cast<FolderShape*>(s);
                if (folder) {
                    if (folder != shape->parent())
                        shape->setParent(folder);
                    break;
                }
            }
            if (shapes.count() && shape->parent()) {
                shape->setPosition(point - offset - shape->parent()->position());
            } else {
                shape->setParent(0);
                shape->setPosition(point - offset);
            }
        }
        shape->update();
    }
}

void Canvas::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setClipRect(e->rect());
    qreal zoomX, zoomY;
    m_converter.zoom(&zoomX, &zoomY);

    painter.save();
    painter.scale(zoomX, zoomY);
    painter.translate(-m_displayOffset);
    QPen pen(Qt::blue); // TODO use the kde-wide 'selected' color.
    pen.setWidth(0); // a cosmetic pen
    foreach (KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        painter.save();
        QPointF pos = shape->position();
        KoShape *parent = shape->parent ();
        while(parent) {
            pos += parent->position();
            parent = parent->parent();
        }
        painter.translate(pos.x(), pos.y());
        painter.strokePath(shape->outline(), pen);
        painter.restore();
    }
    painter.restore();
    QPointF offset = m_converter.documentToView(m_displayOffset);
    painter.translate(-offset);
    painter.setRenderHint(QPainter::Antialiasing);
    shapeManager()->paint( painter, *(viewConverter()), false );
    painter.end();
}

bool Canvas::event(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(e);

        const QPointF pos(helpEvent->x(), helpEvent->y());
        IconShape *is = dynamic_cast<IconShape*>(shapeManager()->shapeAt(pos));
        if (is)
            QToolTip::showText(helpEvent->globalPos(), is->toolTip(), this);
        else
            QToolTip::showText(helpEvent->globalPos(), "", this);
    }
    return QWidget::event(e);
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
    if (m_currentStrategy &&
       (event->key() == Qt::Key_Control
        || event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift
        || event->key() == Qt::Key_Meta)) {
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
        event->accept();
    }
}

void Canvas::keyReleaseEvent(QKeyEvent *event)
{
    if (m_currentStrategy == 0) { // catch all cases where no current strategy is needed
    }
    else if (event->key() == Qt::Key_Escape) {
        m_currentStrategy->cancelInteraction();
        delete m_currentStrategy;
        m_currentStrategy = 0;
        event->accept();
        if (hasFocus() && m_previousFocusOwner)
            m_previousFocusOwner->setFocus();
        setFocusPolicy(Qt::NoFocus);
    }
    else if (event->key() == Qt::Key_Control
            || event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift
            || event->key() == Qt::Key_Meta) {
        m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
    }
}

void Canvas::focusChanged(QWidget *old, QWidget *now)
{
    if (now == this)
        m_previousFocusOwner = old;
}

// getters
KoShapeManager * Canvas::shapeManager() const
{
    return itemStore()->shapeManager();
}

QWidget *Canvas::canvasWidget()
{
    return m_parent;
}

int Canvas::zoomIndex()
{
    return m_zoomIndex;
}

// slots
void Canvas::loadShapeTypes()
{
    QRectF bounds = itemStore()->loadShapeTypes();
    if (itemStore()->mainFolder()) {
        itemStore()->mainFolder()->setPosition(QPointF());
        itemStore()->mainFolder()->setSize(size());
    } else if (!bounds.contains(0, 0)) {
        m_displayOffset = bounds.topLeft();
        update();
    }
}

#include <Canvas.moc>
