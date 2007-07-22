/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "Canvas.h"
#include "ShapeSelector.h"
#include "GroupShape.h"
#include "TemplateShape.h"
#include "FolderShape.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoProperties.h>

#include <QMouseEvent>
#include <QToolTip>
#include <QUndoCommand>

#include <kdebug.h>

Canvas::Canvas(ShapeSelector *parent)
: QWidget(parent)
, KoCanvasBase( &m_shapeController )
, m_parent(parent)
, m_emitItemSelected(false)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setAcceptDrops(true);
}

void Canvas::gridSize (double *, double *) const {
}

void Canvas::updateCanvas (const QRectF &rc) {
    QRect rect = rc.toRect();
    rect.adjust(-2, -2, 2, 2); // grow for to anti-aliasing
    update(rect);
}

void  Canvas::addCommand (QUndoCommand *command) {
    command->redo();
    delete command;
}

// event handlers
void Canvas::mousePressEvent(QMouseEvent *event) {
    KoShape *clickedShape = shapeManager()->shapeAt(event->pos());
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes())
        shape->repaint();
    shapeManager()->selection()->deselectAll();
    m_emitItemSelected = false;
    if(clickedShape == 0)
        return;
    m_emitItemSelected = true;
    shapeManager()->selection()->select(clickedShape);
    clickedShape->repaint();
}

void Canvas::tabletEvent(QTabletEvent *event) {
    event->ignore();
    if(event->type() != QEvent::TabletMove)
        return;
    KoShape *clickedShape = shapeManager()->selection()->firstSelectedShape();
    if(clickedShape == 0) {
        event->accept();
        return;
    }
    QPointF distance = clickedShape->position() - event->pos();
    if(qAbs(distance.x()) < 15 && qAbs(distance.y()) < 15)
        event->accept();

    // if not accepted it will fall through and be offered as a mouseMoveEvent
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    KoShape *clickedShape = shapeManager()->selection()->firstSelectedShape();
    if(clickedShape == 0)
        return;
    QPointF distance = clickedShape->position() - event->pos();
    if(qAbs(distance.x()) < 5 && qAbs(distance.y()) < 5)
        return;

    // start drag
    QString mimeType;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    TemplateShape *templateShape = dynamic_cast<TemplateShape*> (clickedShape);
    if(templateShape) {
        dataStream << templateShape->shapeTemplate().id;
        KoProperties *props = templateShape->shapeTemplate().properties;
        if(props)
            dataStream << props->store(); // is a QString
        else
            dataStream << QString();
        mimeType = SHAPETEMPLATE_MIMETYPE;
    }
    else {
        GroupShape *group = dynamic_cast<GroupShape*> (clickedShape);
        if(group) {
            dataStream << group->groupId();
            mimeType = SHAPEID_MIMETYPE;
        }
        else if(dynamic_cast<FolderShape*> (clickedShape)) {
            dataStream << QString();
            mimeType = FOLDERSHAPE_MIMETYPE;
        } else {
            kWarning() << "Unimplemented drag for this type!\n";
            return;
        }
    }
    QPointF offset(event->pos() - clickedShape->absolutePosition(KoFlake::TopLeftCorner));
    dataStream << offset;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType, itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    IconShape *iconShape = dynamic_cast<IconShape*> (clickedShape);
    if(iconShape)
        drag->setPixmap(iconShape->pixmap());
    drag->setHotSpot(offset.toPoint());

    if(drag->start(Qt::CopyAction | Qt::MoveAction) != Qt::MoveAction)
        m_emitItemSelected = false;
}

void  Canvas::dragEnterEvent(QDragEnterEvent *event) {
    if (event->source() == this && (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE) ||
                event->mimeData()->hasFormat(SHAPEID_MIMETYPE) ||
                event->mimeData()->hasFormat(FOLDERSHAPE_MIMETYPE))) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if(m_emitItemSelected)
        m_parent->itemSelected();
}

void  Canvas::dropEvent(QDropEvent *event) {
    QByteArray itemData;
    bool isTemplate = true;
    if (event->mimeData()->hasFormat(SHAPETEMPLATE_MIMETYPE))
        itemData = event->mimeData()->data(SHAPETEMPLATE_MIMETYPE);
    else if(event->mimeData()->hasFormat(SHAPEID_MIMETYPE)) {
        isTemplate = false;
        itemData = event->mimeData()->data(SHAPEID_MIMETYPE);
    }
    else { // FOLDERSHAPE_MIMETYPE
        isTemplate = false;
        itemData = event->mimeData()->data(FOLDERSHAPE_MIMETYPE);
    }
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QString dummy;
    dataStream >> dummy;
    if(isTemplate) {
        // a template additionally has a properties object. Lets get rid of that.
        QString properties;
        dataStream >> properties;
    }

    // and finally, there is a point.
    QPointF offset;
    dataStream >> offset;

    event->setDropAction(Qt::MoveAction);
    event->accept();
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        shape->repaint();
        if(dynamic_cast<FolderShape*>(shape)) { // is a folder
            shape->setPosition(event->pos() - offset);
        }
        else { // is an icon.
            QList<KoShape*> shapes = shapeManager()->shapesAt(QRectF(event->pos() - offset, QSizeF(0.1, 0.1)));
            foreach(KoShape *s, shapes) {
                FolderShape *folder = dynamic_cast<FolderShape*>(s);
                if(folder) {
                    if(folder != shape->parent())
                        shape->setParent(folder);
                    break;
                }
            }
            if(shapes.count())
                shape->setPosition(event->pos() - offset - shape->parent()->position());
            else {
                shape->setParent(0);
                shape->setPosition(event->pos() - offset);
            }
        }
        shape->repaint();
    }
}

void Canvas::paintEvent(QPaintEvent * e) {
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(e->rect());

    QPen pen(Qt::blue); // TODO use the kde-wide 'selected' color.
    pen.setWidth(1);
    foreach(KoShape *shape, shapeManager()->selection()->selectedShapes()) {
        painter.save();
        painter.translate(shape->position().x(), shape->position().y());
        painter.strokePath(shape->outline(), pen);
        painter.restore();
    }
    m_parent->m_shapeManager->paint( painter, *(viewConverter()), false );
    painter.end();
}

bool Canvas::event(QEvent *e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);

        const QPointF pos(helpEvent->x(), helpEvent->y());
        IconShape *is = dynamic_cast<IconShape*> (m_parent->m_shapeManager->shapeAt(pos));
        if(is)
            QToolTip::showText(helpEvent->globalPos(), is->toolTip());
        else
            QToolTip::showText(helpEvent->globalPos(), "");
    }
    return QWidget::event(e);
}

void Canvas::resizeEvent (QResizeEvent *event) {
    emit resized(event->size());
}

// getters
KoShapeManager * Canvas::shapeManager() const {
    return m_parent->m_shapeManager;
}

QWidget *Canvas::canvasWidget () {
    return m_parent;
}

#include <Canvas.moc>
