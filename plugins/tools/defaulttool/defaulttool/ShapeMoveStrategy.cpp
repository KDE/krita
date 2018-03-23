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
#include <KoCanvasResourceManager.h>
#include <commands/KoShapeMoveCommand.h>
#include <KoSnapGuide.h>
#include <KoPointerEvent.h>
#include <KoToolBase.h>
#include <KoSelection.h>
#include <klocalizedstring.h>
#include <kis_global.h>

#include "kis_debug.h"

ShapeMoveStrategy::ShapeMoveStrategy(KoToolBase *tool, KoSelection *selection, const QPointF &clicked)
    : KoInteractionStrategy(tool)
    , m_start(clicked)
    , m_canvas(tool->canvas())
{
    QList<KoShape *> selectedShapes = selection->selectedEditableShapes();

    QRectF boundingRect;
    Q_FOREACH (KoShape *shape, selectedShapes) {
        m_selectedShapes << shape;
        m_previousPositions << shape->absolutePosition(KoFlake::Center);
        m_newPositions << shape->absolutePosition(KoFlake::Center);
        boundingRect = boundingRect.united(shape->boundingRect());
    }

    KoFlake::AnchorPosition anchor =
            KoFlake::AnchorPosition(
                m_canvas->resourceManager()->resource(KoFlake::HotPosition).toInt());

    m_initialOffset = selection->absolutePosition(anchor) - m_start;
    m_canvas->snapGuide()->setIgnoredShapes(KoShape::linearizeSubtree(m_selectedShapes));

    tool->setStatusText(i18n("Press Shift to hold x- or y-position."));
}

void ShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    if (m_selectedShapes.isEmpty()) {
        return;
    }
    QPointF diff = point - m_start;

    if (modifiers & Qt::ShiftModifier) {
        // Limit change to one direction only
        diff = snapToClosestAxis(diff);
    } else {
        QPointF positionToSnap = point + m_initialOffset;
        tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
        QPointF snappedPosition = tool()->canvas()->snapGuide()->snap(positionToSnap, modifiers);
        tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
        diff = snappedPosition - m_initialOffset - m_start;
    }

    moveSelection(diff);
    m_finalMove = diff;
}

void ShapeMoveStrategy::moveSelection(const QPointF &diff)
{
    Q_ASSERT(m_newPositions.count());

    int i = 0;
    Q_FOREACH (KoShape *shape, m_selectedShapes) {
        QPointF delta = m_previousPositions.at(i) + diff - shape->absolutePosition(KoFlake::Center);
        if (shape->parent()) {
            shape->parent()->model()->proposeMove(shape, delta);
        }
        tool()->canvas()->clipToDocument(shape, delta);
        QPointF newPos(shape->absolutePosition(KoFlake::Center) + delta);
        m_newPositions[i] = newPos;

        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setAbsolutePosition(newPos, KoFlake::Center);
        shape->updateAbsolute(oldDirtyRect | oldDirtyRect.translated(delta));
        i++;
    }
}

KUndo2Command *ShapeMoveStrategy::createCommand()
{
    tool()->canvas()->snapGuide()->reset();
    if (m_finalMove.isNull()) {
        return 0;
    }
    return new KoShapeMoveCommand(m_selectedShapes, m_previousPositions, m_newPositions);
}

void ShapeMoveStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    tool()->canvas()->updateCanvas(tool()->canvas()->snapGuide()->boundingRect());
}

void ShapeMoveStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}
