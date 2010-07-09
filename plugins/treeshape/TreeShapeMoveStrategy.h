/* This file is part of the KDE project

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

#ifndef TREESHAPEMOVESTRATEGY_H
#define TREESHAPEMOVESTRATEGY_H

#include <KoInteractionStrategy.h>

#include <QPointF>
#include <QList>

class KoCanvasBase;
class KoToolBase;
class KoShape;
class TreeShape;

/**
 * Implements the Move action on an object or selected objects.
 */
class TreeShapeMoveStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that starts to move the objects.
     * @param tool the parent tool which controls this strategy
     * @param canvas the canvas interface which will supply things like a selection object
     * @param clicked the initial point that the user depressed (in pt).
     */
    TreeShapeMoveStrategy(KoToolBase *tool, const QPointF &clicked);
    virtual ~TreeShapeMoveStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    QUndoCommand* createCommand();
    void finishInteraction( Qt::KeyboardModifiers modifiers ) { Q_UNUSED( modifiers ); }
    virtual void paint( QPainter &painter, const KoViewConverter &converter);
private:
    QList<QPointF> m_previousPositions;
    QList<QPointF> m_newPositions;
    QPointF m_start, m_diff, m_initialSelectionPosition, m_initialOffset;
    QList<KoShape*> m_selectedShapes;
    TreeShape *m_newParent;
    KoShape *m_currentShape;
};

#endif
