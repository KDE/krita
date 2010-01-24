/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "SelectStrategy.h"
#include "Canvas.h"
#include "TemplateShape.h"
#include "GroupShape.h"
#include "FolderShape.h"
#include "ClipboardProxyShape.h"

#include <KoShape.h>
#include <KoProperties.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>

#include <KDebug>
#include <QMouseEvent>
#include <QDrag>

SelectStrategy::SelectStrategy(Canvas *canvas, KoShape *clickedShape, KoPointerEvent &event)
    : m_canvas(canvas), m_clickedShape(clickedShape)
{
    const bool deselectAll = event.button() == Qt::LeftButton
        || (event.button() == Qt::RightButton && clickedShape);
    if (deselectAll) {
        foreach(KoShape *shape, canvas->shapeManager()->selection()->selectedShapes())
            shape->update();
        canvas->shapeManager()->selection()->deselectAll();
    }
    if (clickedShape) {
        canvas->shapeManager()->selection()->select(clickedShape);
        clickedShape->update();
    }
    m_emitItemSelected = clickedShape;
}

void SelectStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    if (m_clickedShape == 0)
        return;
    QPointF distance = m_clickedShape->position() - mouseLocation;
    QPointF pixelDistance = m_canvas->viewConverter()->documentToView(distance);
    if (qAbs(pixelDistance.x()) < 5 && qAbs(pixelDistance.y()) < 5)
        return;

    // start drag
    QString mimeType;
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    TemplateShape *templateShape = dynamic_cast<TemplateShape*>(m_clickedShape);
    bool addOffset = true;
    if (templateShape) {
        dataStream << templateShape->shapeTemplate().id;
        KoProperties *props = templateShape->shapeTemplate().properties;
        if (props)
            dataStream << props->store("item"); // is an xml-QString
        else
            dataStream << QString();
        mimeType = SHAPETEMPLATE_MIMETYPE;
    } else {
        GroupShape *group = dynamic_cast<GroupShape*>(m_clickedShape);
        if (group) {
            dataStream << group->groupId();
            mimeType = SHAPEID_MIMETYPE;
        }
        else if (dynamic_cast<FolderShape*>(m_clickedShape)) {
            dataStream << QString();
            mimeType = FOLDERSHAPE_MIMETYPE;
        } else {
            ClipboardProxyShape *cbps = dynamic_cast<ClipboardProxyShape*>(m_clickedShape);
            if (cbps) {
                mimeType = OASIS_MIME;
                addOffset = false;
                itemData = cbps->clipboardData();
            } else {
                kWarning(31000) << "Unimplemented drag for this type!";
                return;
            }
        }
    }
    QPointF offset(mouseLocation - m_clickedShape->absolutePosition(KoFlake::TopLeftCorner));
    if (addOffset)
        dataStream << offset;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(mimeType, itemData);

    QDrag *drag = new QDrag(m_canvas);
    drag->setMimeData(mimeData);
    IconShape *iconShape = dynamic_cast<IconShape*>(m_clickedShape);
    if (iconShape)
        drag->setPixmap(iconShape->pixmap());
    drag->setHotSpot(offset.toPoint());

    if (drag->start(Qt::CopyAction | Qt::MoveAction) != Qt::MoveAction)
        m_emitItemSelected = false;
}

void SelectStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    if (m_clickedShape && m_emitItemSelected)
        emit itemSelected();
}

#include <SelectStrategy.moc>

