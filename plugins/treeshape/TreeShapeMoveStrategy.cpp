/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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

#include "TreeShape.h"
#include "TreeShapeMoveStrategy.h"
#include "TreeShapeMoveCommand.h"
#include "SelectionDecorator.h"

#include "kdebug.h"

#include <KoCanvasBase.h>
#include <KoShapeManager.h>
#include <KoShapeContainer.h>
#include <KoShapeContainerModel.h>
#include <KoResourceManager.h>
#include <KoSnapGuide.h>
#include <KoPointerEvent.h>
#include <KoToolBase.h>
#include <KLocale>

TreeShapeMoveStrategy::TreeShapeMoveStrategy(KoToolBase *tool, const QPointF &clicked)
    : KoInteractionStrategy(tool),
    m_start(clicked),
    m_newParent(0)
{
    m_currentShape = tool->canvas()->shapeManager()->shapeAt(clicked);
    kDebug() << "m_currentShape" << m_currentShape;
    QList<KoShape*> selectedShapes = tool->canvas()->shapeManager()->selection()->selectedShapes(KoFlake::TopLevelSelection);
    QRectF boundingRect;
    foreach(KoShape *shape, selectedShapes) {
        kDebug() << "selected shape" << shape << "to tree" << dynamic_cast<TreeShape*>(shape);
        if (dynamic_cast<TreeShape*>(shape->parent())){
            kDebug() << "parent" << shape->parent() << "to tree" << dynamic_cast<TreeShape*>(shape->parent());
            m_selectedShapes << shape->parent();
            m_previousPositions << shape->parent()->position();
            m_newPositions << shape->parent()->position();
            boundingRect = boundingRect.unite( shape->parent()->boundingRect() );
        }
    }
}

void TreeShapeMoveStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    if(m_selectedShapes.isEmpty())
        return;

    m_diff = point - m_start;
    KoShape *shape = tool()->canvas()->shapeManager()->shapeAt(point);
    if (shape && shape != m_currentShape)
        m_newParent = dynamic_cast<TreeShape*>(shape->parent());

//     int i=0;
//     foreach(KoShape *shape, m_selectedShapes) {
//         QPointF delta = m_previousPositions[i] + m_diff - shape->position();
//         if(shape->parent())
//             shape->parent()->model()->proposeMove(shape, delta);
//         QPointF newPos (shape->position() + delta);
//         m_newPositions[i] = newPos;
//         shape->update();
//         shape->setPosition(newPos);
//         shape->update();
//         i++;
//     }
//     tool()->canvas()->shapeManager()->selection()->setPosition(m_initialSelectionPosition + m_diff);
}

QUndoCommand* TreeShapeMoveStrategy::createCommand()
{
    if (m_diff.isNull() || m_selectedShapes.isEmpty() || (m_newParent == 0)){
        kDebug() << "children " << m_selectedShapes.size() << "parent" << m_newParent;
        return 0;
    }
    return new TreeShapeMoveCommand(m_selectedShapes, m_newParent);
}

void TreeShapeMoveStrategy::paint( QPainter &painter, const KoViewConverter &converter)
{
    SelectionDecorator decorator;
    decorator.setSelection(tool()->canvas()->shapeManager()->selection());
    decorator.paint(painter, converter);
}
