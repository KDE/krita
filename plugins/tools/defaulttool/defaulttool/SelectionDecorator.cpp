/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "SelectionDecorator.h"

#include <KoShape.h>
#include <KoSelection.h>
#include <KoResourcePaths.h>
#include "kis_algebra_2d.h"

#include "kis_debug.h"
#include <KisHandlePainterHelper.h>
#include <KoCanvasResourceManager.h>

#define HANDLE_DISTANCE 10

SelectionDecorator::SelectionDecorator(KoCanvasResourceManager *resourceManager)
    : m_hotPosition(KoFlake::Center)
    , m_handleRadius(7)
    , m_lineWidth(2)
{
    m_hotPosition =
        KoFlake::AnchorPosition(
            resourceManager->resource(KoFlake::HotPosition).toInt());
}

void SelectionDecorator::setSelection(KoSelection *selection)
{
    m_selection = selection;
}

void SelectionDecorator::setHandleRadius(int radius)
{
    m_handleRadius = radius;
    m_lineWidth = qMax(1, (int)(radius / 2));
}

void SelectionDecorator::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF handleArea;
    painter.save();

    // save the original painter transformation
    QTransform painterMatrix = painter.worldTransform();

    QPen pen;
    //Use the #00adf5 color with 50% opacity
    pen.setColor(QColor(0, 173, 245, 127));
    pen.setWidth(m_lineWidth);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    bool editable = false;

    QList<KoShape*> selectedShapes = m_selection->selectedShapes();
    if (selectedShapes.isEmpty()) return;

    foreach (KoShape *shape, KoShape::linearizeSubtree(selectedShapes)) {
        painter.setWorldTransform(shape->absoluteTransformation(&converter) * painterMatrix);
        KoShape::applyConversion(painter, converter);

        KisHandlePainterHelper helper(&painter);
        helper.drawRubberLine(shape->outlineRect());

        if (!shape->isGeometryProtected()) {
            editable = true;
        }
    }

    handleArea = m_selection->outlineRect();
    painter.setTransform(m_selection->absoluteTransformation(&converter) * painterMatrix);
    KoShape::applyConversion(painter, converter);

    // draw extra rubber line around all the shapes
    if (selectedShapes.size() > 1) {
        painter.setPen(Qt::blue);

        KisHandlePainterHelper helper(&painter);
        helper.drawRubberLine(handleArea);

    }

    // if we have no editable shape selected there
    // is no need drawing the selection handles
    if (editable) {

        painter.setPen(pen);
        painter.setBrush(pen.color());

        QPolygonF outline = handleArea;

        {
            KisHandlePainterHelper helper(&painter, m_handleRadius);
            helper.drawHandleRect(outline.value(0));
            helper.drawHandleRect(outline.value(1));
            helper.drawHandleRect(outline.value(2));
            helper.drawHandleRect(outline.value(3));
            helper.drawHandleRect(0.5 * (outline.value(0) + outline.value(1)));
            helper.drawHandleRect(0.5 * (outline.value(1) + outline.value(2)));
            helper.drawHandleRect(0.5 * (outline.value(2) + outline.value(3)));
            helper.drawHandleRect(0.5 * (outline.value(3) + outline.value(0)));

            // draw the hot position
            painter.setBrush(Qt::red);
            QPointF hotPos = KoFlake::anchorToPoint(m_hotPosition, handleArea);
            helper.drawHandleRect(hotPos);
        }
    }

    painter.restore();
}

