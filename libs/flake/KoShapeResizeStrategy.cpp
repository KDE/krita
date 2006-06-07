/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeResizeStrategy.h"
#include "KoShapeManager.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCommand.h"

#include <QDebug>

KoShapeResizeStrategy::KoShapeResizeStrategy( KoTool *tool, KoCanvasBase *canvas,
        const QPointF &clicked, KoFlake::SelectionHandle direction )
: KoInteractionStrategy(tool, canvas)
, m_initialBoundingRect()
{
    KoSelectionSet selectedObjects = canvas->shapeManager()->selection()->selectedObjects(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedObjects) {
        m_selectedObjects << shape;
        m_startPositions << shape->absolutePosition();
        m_startSizes << shape->size();
        m_initialBoundingRect = m_initialBoundingRect.unite( shape->boundingRect() );
    }
    m_start = clicked;

    switch(direction) {
        case KoFlake::TopMiddleHandle:
            m_top = true; m_bottom = false; m_left = false; m_right = false; break;
        case KoFlake::TopRightHandle:
            m_top = true; m_bottom = false; m_left = false; m_right = true; break;
        case KoFlake::RightMiddleHandle:
            m_top = false; m_bottom = false; m_left = false; m_right = true; break;
        case KoFlake::BottomRightHandle:
            m_top = false; m_bottom = true; m_left = false; m_right = true; break;
        case KoFlake::BottomMiddleHandle:
            m_top = false; m_bottom = true; m_left = false; m_right = false; break;
        case KoFlake::BottomLeftHandle:
            m_top = false; m_bottom = true; m_left = true; m_right = false; break;
        case KoFlake::LeftMiddleHandle:
            m_top = false; m_bottom = false; m_left = true; m_right = false; break;
        case KoFlake::TopLeftHandle:
            m_top = true; m_bottom = false; m_left = true; m_right = false; break;
        default:
            ;// throw exception ?  TODO
    }
}

void KoShapeResizeStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    QPointF distance = point - m_start;
    bool keepAspect = modifiers & Qt::AltModifier;
    foreach(KoShape *shape, m_selectedObjects)
        keepAspect = keepAspect || shape->keepAspectRatio();

    if(m_canvas->snapToGrid() && (modifiers & Qt::ShiftModifier) == 0)
        applyGrid(distance);

    double startWidth = m_initialBoundingRect.width();
    double startHeight = m_initialBoundingRect.height();
    QMatrix matrix;
    bool scaleFromCenter = modifiers & Qt::ControlModifier;
    if(scaleFromCenter) // translate to center first
        matrix.translate(startWidth / 2.0, startHeight / 2.0);
    else
        matrix.translate(m_left?startWidth:0, m_top?startHeight:0);

    double zoomX=1, zoomY=1;
    if(keepAspect) {
        double ratio = startWidth / startHeight;
        double width = startWidth - distance.x();
        double height = startHeight - distance.y();
        int toLargestEdge = (m_bottom?1:0) + (m_top?1:0) + // should be false when only one
            (m_left?1:0) + (m_right?1:0);                  // of the direction bools is set
        bool horizontal = m_left || m_right;

        if(toLargestEdge != 1) { // one of the corners.
            if (width < height) // the biggest border is the one in control
                width = height * ratio;
            else
                height = width / ratio;
        } else {
            if (horizontal)
                height = width / ratio;
            else
                width = height * ratio;
        }
        zoomX = startWidth / width;
        zoomY = startHeight / height;
    }
    else {
        if(m_left)
            zoomX = (startWidth - distance.x()) / startWidth;
        else if(m_right)
            zoomX = (startWidth + distance.x()) / startWidth;
        if(m_top)
            zoomY = (startHeight - distance.y()) / startHeight;
        else if(m_bottom)
            zoomY = (startHeight + distance.y()) / startHeight;
    }
    matrix.scale(qMax(0.0, zoomX), qMax(0.0, zoomY));
    if(scaleFromCenter) // and back
        matrix.translate(-startWidth / 2.0, -startHeight / 2.0);
    else
        matrix.translate(m_left?-startWidth:0, m_top?-startHeight:0);

    int i=0;
    foreach(KoShape *shape, m_selectedObjects) {
        QRectF rect(m_startPositions.at(i) - m_initialBoundingRect.topLeft(), m_startSizes.at(i));
        QRectF result = matrix.mapRect(rect);
        result.setWidth(qMax(4.0, result.width()));
        result.setHeight(qMax(4.0, result.height()));
        shape->repaint();
        shape->setAbsolutePosition( result.topLeft() + m_initialBoundingRect.topLeft() );
        shape->resize( result.size() );
        shape->repaint();
        i++;
    }
}

KCommand* KoShapeResizeStrategy::createCommand() {
    KMacroCommand *cmd = new KMacroCommand("Resize");
    QList<QPointF> newPositions;
    QList<QSizeF> newSizes;
    foreach(KoShape *shape, m_selectedObjects) {
        newPositions << shape->position();
        newSizes << shape->size();
    }
    cmd->addCommand(new KoShapeMoveCommand(m_selectedObjects, m_startPositions, newPositions));
    cmd->addCommand(new KoShapeSizeCommand(m_selectedObjects, m_startSizes, newSizes));
    return cmd;
}

void KoShapeResizeStrategy::paint( QPainter &painter, KoViewConverter &converter) {
    SelectionDecorator decorator (m_canvas->shapeManager()->selection()->boundingRect(), KoFlake::NoHandle, false, false);
    decorator.paint(painter, converter);
}
