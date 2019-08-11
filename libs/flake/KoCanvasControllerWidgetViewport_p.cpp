/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoCanvasControllerWidgetViewport_p.h"

#include <limits.h>
#include <stdlib.h>

#include <QPainter>
#include <QDragEnterEvent>
#include <QMimeData>

#include <KoProperties.h>

#include <FlakeDebug.h>

#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeFactoryBase.h" // for the SHAPE mimetypes
#include "KoShapeRegistry.h"
#include "KoShapeController.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoCanvasBase.h"
#include "KoShapeLayer.h"
#include "KoShapePaintingContext.h"
#include "KoToolProxy.h"
#include "KoCanvasControllerWidget.h"
#include "KoViewConverter.h"
#include "KoSvgPaste.h"

// ********** Viewport **********
Viewport::Viewport(KoCanvasControllerWidget *parent)
    : QWidget(parent)
    , m_draggedShape(0)
    , m_canvas(0)
    , m_documentOffset(QPoint(0, 0))
    , m_margin(0)
{
    setAutoFillBackground(true);
    setAcceptDrops(true);
    setMouseTracking(true);
    m_parent = parent;
}

void Viewport::setCanvas(QWidget *canvas)
{
    if (m_canvas) {
        m_canvas->hide();
        delete m_canvas;
    }
    m_canvas = canvas;
    if (!canvas) return;
    m_canvas->setParent(this);
    m_canvas->show();
    if (!m_canvas->minimumSize().isNull()) {
        m_documentSize = m_canvas->minimumSize();
    }
    resetLayout();
}

void Viewport::setDocumentSize(const QSizeF &size)
{
    m_documentSize = size;
    resetLayout();
}

void Viewport::documentOffsetMoved(const QPoint &pt)
{
    m_documentOffset = pt;
    resetLayout();
}

void Viewport::handleDragEnterEvent(QDragEnterEvent *event)
{
    // if not a canvas set then ignore this, makes it possible to assume
    // we have a canvas in all the support methods.
    if (!(m_parent->canvas() && m_parent->canvas()->canvasWidget())) {
        event->ignore();
        return;
    }

    delete m_draggedShape;
    m_draggedShape = 0;

    // only allow dropping when active layer is editable
    KoSelection *selection = m_parent->canvas()->shapeManager()->selection();
    KoShapeLayer *activeLayer = selection->activeLayer();
    if (activeLayer && (!activeLayer->isShapeEditable() || activeLayer->isGeometryProtected())) {
        event->ignore();
        return;
    }

    const QMimeData *data = event->mimeData();

    if (data->hasFormat(SHAPETEMPLATE_MIMETYPE) ||
            data->hasFormat(SHAPEID_MIMETYPE) ||
            data->hasFormat("image/svg+xml"))
    {
        if (data->hasFormat("image/svg+xml")) {
            KoCanvasBase *canvas = m_parent->canvas();
            QSizeF fragmentSize;

            QList<KoShape*> shapes = KoSvgPaste::fetchShapesFromData(data->data("image/svg+xml"),
                                                                     canvas->shapeController()->documentRectInPixels(),
                                                                     canvas->shapeController()->pixelsPerInch(),
                                                                     &fragmentSize);

            if (!shapes.isEmpty()) {
                m_draggedShape = shapes[0];
            }
        }
        else {
            QByteArray itemData;
            bool isTemplate = true;

            if (data->hasFormat(SHAPETEMPLATE_MIMETYPE)) {
                itemData = data->data(SHAPETEMPLATE_MIMETYPE);
            }
            else if (data->hasFormat(SHAPEID_MIMETYPE)) {
                isTemplate = false;
                itemData = data->data(SHAPEID_MIMETYPE);
            }


            QDataStream dataStream(&itemData, QIODevice::ReadOnly);
            QString id;
            dataStream >> id;
            QString properties;
            if (isTemplate)
                dataStream >> properties;

            // and finally, there is a point.
            QPointF offset;
            dataStream >> offset;

            // The rest of this method is mostly a copy paste from the KoCreateShapeStrategy
            // So, lets remove this again when Zagge adds his new class that does this kind of thing. (KoLoadSave)
            KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(id);
            if (! factory) {
                warnFlake << "Application requested a shape that is not registered '" <<
                             id << "', Ignoring";
                event->ignore();
                return;
            }
            if (isTemplate) {
                KoProperties props;
                props.load(properties);
                m_draggedShape = factory->createShape(&props, m_parent->canvas()->shapeController()->resourceManager());
            }
            else {
                m_draggedShape = factory->createDefaultShape(m_parent->canvas()->shapeController()->resourceManager());
            }

            if (m_draggedShape->shapeId().isEmpty()) {
                m_draggedShape->setShapeId(factory->id());
            }
        }

        event->setDropAction(Qt::CopyAction);
        event->accept();

        Q_ASSERT(m_draggedShape);
        if (!m_draggedShape) return;

        // calculate maximum existing shape zIndex

        int pasteZIndex = 0;

        {
            QList<KoShape*> allShapes = m_parent->canvas()->shapeManager()->topLevelShapes();

            if (!allShapes.isEmpty()) {
                std::sort(allShapes.begin(), allShapes.end(), KoShape::compareShapeZIndex);
                pasteZIndex = qMin(int(KoShape::maxZIndex), allShapes.last()->zIndex() + 1);
            }
        }

        m_draggedShape->setZIndex(pasteZIndex);
        m_draggedShape->setAbsolutePosition(correctPosition(event->pos()));

        m_parent->canvas()->shapeManager()->addShape(m_draggedShape);
    } else {
        event->ignore();
    }
}

void Viewport::handleDropEvent(QDropEvent *event)
{
    if (!m_draggedShape) {
        m_parent->canvas()->toolProxy()->dropEvent(event, correctPosition(event->pos()));
        return;
    }

    repaint(m_draggedShape);
    m_parent->canvas()->shapeManager()->remove(m_draggedShape); // remove it to not interfere with z-index calc.

    m_draggedShape->setPosition(QPointF(0, 0));  // always save position.
    QPointF newPos = correctPosition(event->pos());
    m_parent->canvas()->clipToDocument(m_draggedShape, newPos); // ensure the shape is dropped inside the document.
    m_draggedShape->setAbsolutePosition(newPos);


    KUndo2Command * cmd = m_parent->canvas()->shapeController()->addShape(m_draggedShape, 0);

    if (cmd) {
        m_parent->canvas()->addCommand(cmd);
        KoSelection *selection = m_parent->canvas()->shapeManager()->selection();

        // repaint selection before selecting newly create shape
        Q_FOREACH (KoShape * shape, selection->selectedShapes()) {
            shape->update();
        }

        selection->deselectAll();
        selection->select(m_draggedShape);
    } else {

        delete m_draggedShape;
    }

    m_draggedShape = 0;
}

QPointF Viewport::correctPosition(const QPoint &point) const
{
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    QPoint correctedPos(point.x() - canvasWidget->x(), point.y() - canvasWidget->y());
    correctedPos += m_documentOffset;
    return m_parent->canvas()->viewToDocument(correctedPos);
}

void Viewport::handleDragMoveEvent(QDragMoveEvent *event)
{
    if (!m_draggedShape) {
        m_parent->canvas()->toolProxy()->dragMoveEvent(event, correctPosition(event->pos()));
        return;
    }

    m_draggedShape->update();
    repaint(m_draggedShape);
    m_draggedShape->setAbsolutePosition(correctPosition(event->pos()));
    m_draggedShape->update();
    repaint(m_draggedShape);
}

void Viewport::repaint(KoShape *shape)
{
    QRect rect = m_parent->canvas()->viewConverter()->documentToView(shape->boundingRect()).toRect();
    QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
    Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
    rect.moveLeft(rect.left() + canvasWidget->x() - m_documentOffset.x());
    rect.moveTop(rect.top() + canvasWidget->y() - m_documentOffset.y());
    rect.adjust(-2, -2, 2, 2); // adjust for antialias
    update(rect);
}

void Viewport::handleDragLeaveEvent(QDragLeaveEvent *event)
{
    if (m_draggedShape) {
        repaint(m_draggedShape);
        m_parent->canvas()->shapeManager()->remove(m_draggedShape);
        delete m_draggedShape;
        m_draggedShape = 0;
    } else {
        m_parent->canvas()->toolProxy()->dragLeaveEvent(event);
    }
}

void Viewport::handlePaintEvent(QPainter &painter, QPaintEvent *event)
{
    Q_UNUSED(event);
    if (m_draggedShape) {
        const KoViewConverter *vc = m_parent->canvas()->viewConverter();

        painter.save();
        QWidget *canvasWidget = m_parent->canvas()->canvasWidget();
        Q_ASSERT(canvasWidget); // since we should not allow drag if there is not.
        painter.translate(canvasWidget->x() - m_documentOffset.x(),
                          canvasWidget->y() - m_documentOffset.y());
        QPointF offset = vc->documentToView(m_draggedShape->position());
        painter.setOpacity(0.6);
        painter.translate(offset.x(), offset.y());
        painter.setRenderHint(QPainter::Antialiasing);
        KoShapePaintingContext paintContext; //FIXME
        m_draggedShape->paint(painter, *vc, paintContext);
        painter.restore();
    }
}

void Viewport::resetLayout()
{
    // Determine the area we have to show
    QRect viewRect(m_documentOffset, size());

    const int viewH = viewRect.height();
    const int viewW = viewRect.width();

    if (m_canvas) {
        QRect geom = QRect(0, 0, viewW, viewH);
        if (m_canvas->geometry() != geom) {
            m_canvas->setGeometry(geom);
            m_canvas->update();
        }
    }
    emit sizeChanged();
#if 0
    debugFlake <<"View port geom:" << geometry();
    if (m_canvas)
        debugFlake <<"Canvas widget geom:" << m_canvas->geometry();
#endif
}
