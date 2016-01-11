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

#define HANDLE_DISTANCE 10

KoFlake::Position SelectionDecorator::m_hotPosition = KoFlake::TopLeftCorner;

SelectionDecorator::SelectionDecorator(KoFlake::SelectionHandle arrows, bool rotationHandles, bool shearHandles)
    : m_rotationHandles(rotationHandles)
    , m_shearHandles(shearHandles)
    , m_arrows(arrows)
    , m_handleRadius(3)
    , m_lineWidth(1)
{
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

void SelectionDecorator::setHotPosition(KoFlake::Position hotPosition)
{
    m_hotPosition = hotPosition;
}

KoFlake::Position SelectionDecorator::hotPosition()
{
    return m_hotPosition;
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
    foreach (KoShape *shape, m_selection->selectedShapes(KoFlake::StrippedSelection)) {
        // apply the shape transformation on top of the old painter transformation
        painter.setWorldTransform(shape->absoluteTransformation(&converter) * painterMatrix);
        // apply the zoom factor
        KoShape::applyConversion(painter, converter);
        // draw the shape bounding rect
        painter.drawRect(QRectF(QPointF(), shape->size()));

        if (!shape->isGeometryProtected()) {
            editable = true;
        }
    }

    if (m_selection->count() > 1) {
        // more than one shape selected, so we need to draw the selection bounding rect
        painter.setPen(Qt::blue);
        // apply the selection transformation on top of the old painter transformation
        painter.setWorldTransform(m_selection->absoluteTransformation(&converter) * painterMatrix);
        // apply the zoom factor
        KoShape::applyConversion(painter, converter);
        // draw the selection bounding rect
        painter.drawRect(QRectF(QPointF(), m_selection->size()));
        // save the selection bounding rect for later drawing the selection handles
        handleArea = QRectF(QPointF(), m_selection->size());
    } else if (m_selection->firstSelectedShape()) {
        // only one shape selected, so we compose the correct painter matrix
        painter.setWorldTransform(m_selection->firstSelectedShape()->absoluteTransformation(&converter) * painterMatrix);
        KoShape::applyConversion(painter, converter);
        // save the only selected shapes bounding rect for later drawing the handles
        handleArea = QRectF(QPointF(), m_selection->firstSelectedShape()->size());
    }

    painterMatrix = painter.worldTransform();
    painter.restore();

    // if we have no editable shape selected there is no need drawing the selection handles
    if (!editable) {
        return;
    }

    painter.save();

    painter.setTransform(QTransform());
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.setPen(pen);
    painter.setBrush(pen.color());

    QPolygonF outline = painterMatrix.map(handleArea);

    // the 8 move rects
    QRectF rect(QPointF(0.5, 0.5), QSizeF(2 * m_handleRadius, 2 * m_handleRadius));
    rect.moveCenter(outline.value(0));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(1));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(2));
    painter.drawRect(rect);
    rect.moveCenter(outline.value(3));
    painter.drawRect(rect);
    rect.moveCenter((outline.value(0) + outline.value(1)) / 2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(1) + outline.value(2)) / 2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(2) + outline.value(3)) / 2);
    painter.drawRect(rect);
    rect.moveCenter((outline.value(3) + outline.value(0)) / 2);
    painter.drawRect(rect);

    // draw the hot position
    painter.setBrush(Qt::red);
    QPointF pos;
    switch (m_hotPosition) {
    case KoFlake::TopLeftCorner: pos = handleArea.topLeft(); break;
    case KoFlake::TopRightCorner: pos = handleArea.topRight(); break;
    case KoFlake::BottomLeftCorner: pos = handleArea.bottomLeft(); break;
    case KoFlake::BottomRightCorner: pos = handleArea.bottomRight(); break;
    case KoFlake::CenteredPosition: pos = handleArea.center(); break;
    }
    rect.moveCenter(painterMatrix.map(pos));
    painter.drawRect(rect);

    painter.restore();
}

