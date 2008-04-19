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

#include "ShapeMoveStrategy.h"
#include "SelectionDecorator.h"

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoShapeContainer.h>
#include <KoShapeContainerModel.h>
#include <KoCanvasResourceProvider.h>
#include <commands/KoShapeMoveCommand.h>
#include <KoSnapGuide.h>
#include <KoPointerEvent.h>

ShapeMoveStrategy::ShapeMoveStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
: KoInteractionStrategy(tool, canvas)
, m_start(clicked)
{
    QList<KoShape*> selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    QRectF boundingRect;
    foreach(KoShape *shape, selectedShapes) {
        if( ! shape->isEditable() )
            continue;
        m_selectedShapes << shape;
        m_previousPositions << shape->position();
        m_newPositions << shape->position();
        boundingRect = boundingRect.unite( shape->boundingRect() );
    }
    KoSelection * selection = m_canvas->shapeManager()->selection();
    m_initialOffset = selection->absolutePosition( SelectionDecorator::hotPosition() ) - m_start;
    m_initialSelectionPosition = selection->position();
    m_canvas->snapGuide()->setIgnoredShapes( selection->selectedShapes( KoFlake::FullSelection ) );
}

void ShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) 
{
    if(m_selectedShapes.isEmpty())
        return;
    QPointF diff = point - m_start;

    if(modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // keep x or y position unchanged
        if(qAbs(diff.x()) < qAbs(diff.y()))
            diff.setX(0);
        else
            diff.setY(0);
    }
    else
    {
        QPointF positionToSnap = point + m_initialOffset;
        m_canvas->updateCanvas( m_canvas->snapGuide()->boundingRect() );
        QPointF snappedPosition = m_canvas->snapGuide()->snap( positionToSnap, modifiers );
        m_canvas->updateCanvas( m_canvas->snapGuide()->boundingRect() );
        diff = snappedPosition - m_initialOffset - m_start;
    }

    m_diff = diff;

    moveBy( diff );
}

void ShapeMoveStrategy::handleCustomEvent( KoPointerEvent * event )
{
    QPointF diff = m_canvas->viewConverter()->viewToDocument( event->pos() );

    if( event->modifiers() & (Qt::AltModifier | Qt::ControlModifier)) {
        // keep x or y position unchanged
        if(qAbs(diff.x()) < qAbs(diff.y()))
            diff.setX(0);
        else
            diff.setY(0);
    }

    m_diff += 0.1 * diff ;

    moveBy( diff );
}

void ShapeMoveStrategy::moveBy( const QPointF &diff )
{
    Q_ASSERT(m_newPositions.count());

    int i=0;
    foreach(KoShape *shape, m_selectedShapes) {
        QPointF delta = m_previousPositions.at(i) + m_diff - shape->position();
        if(shape->parent())
            shape->parent()->model()->proposeMove(shape, delta);
        m_canvas->clipToDocument(shape, delta);
        QPointF newPos (shape->position() + delta);
        m_newPositions[i] = newPos;
        shape->update();
        shape->setPosition(newPos);
        shape->update();
        i++;
    }
    m_canvas->shapeManager()->selection()->setPosition(m_initialSelectionPosition + m_diff);
}

QUndoCommand* ShapeMoveStrategy::createCommand() {
    m_canvas->snapGuide()->reset();
    if(m_diff.x() == 0 && m_diff.y() == 0)
        return 0;
    return new KoShapeMoveCommand(m_selectedShapes, m_previousPositions, m_newPositions);
}

void ShapeMoveStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator (KoFlake::NoHandle, false, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);
}
