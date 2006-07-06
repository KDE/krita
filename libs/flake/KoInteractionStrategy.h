/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KODEF_H
#define KODEF_H

#include <Qt>

#include <KoSelection.h>

class QPainter;
class KoPointerEvent;
class KCommand;
class KoCanvasBase;
class KoInteractionTool;
class KoTool;

/**
 * Abstract interface to define what actions a KoInteractionTool can do based on the Strategy design.
 * e.g, move, select, transform.

 * KoInteractionStrategy is a Strategy for the KoInteractionTool and it defines the behavior in case the user
 * clicks or drags the mouse.
 * The strategy is created in the createPolicy() function which defines the
 * resulting behavior and initiates a move or a resize, for example.
 * The mouseMove events are forwarded to the handleMouseMove() method and the interaction
 * is either finished with finishInteraction() or cancelInteraction() (never both).
 */
class KoInteractionStrategy
{
public:
    /// Destructor
    virtual ~KoInteractionStrategy() {}

    /**
     * Reimplement this if the action needs to draw a "blob" on the canvas;
     * that is, a transient decoration like a rubber band.
     */
    virtual void paint( QPainter &painter, KoViewConverter &converter) {
        Q_UNUSED(painter); Q_UNUSED(converter); };
    /**
     * Extending classes should implement this method to update the selectedShapes
     * based on the new mouse position.
     * @param mouseLocation the new location in pt
     * @param modifiers OR-ed set of keys pressed.
     */
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) = 0;
    /**
     * For interactions that are undo-able this method should be implemented to return such
     * a command.  Implementations should return 0 otherwise.
     * @return a command, or 0.
     */
    virtual KCommand* createCommand() = 0;
    /**
     * This method will undo frames based interactions by calling createCommand()
     * and unexecuting that.
     */
    void cancelInteraction();
    /**
     * Override to make final changes to the data on the end of an interaction.
     */
    virtual void finishInteraction() = 0;

    /**
     * This instanciates a new strategy object and decides which one created based on the params.
     */
    static KoInteractionStrategy* createStrategy(KoPointerEvent *event, KoInteractionTool *parentTool, KoCanvasBase *canvas);

protected:
    /// protected constructor. Use the createStrategy method()
    KoInteractionStrategy(KoTool *parent, KoCanvasBase *canvas);

    /**
     * Apply the grid settings to the argument point.
     * @param point this point will be changed to end up on the grid.
     */
    void applyGrid(QPointF &point);

protected: // members
    KoTool *m_parent; ///< the KoTool instance that controls this strategy.
    KoSelectionSet m_selectedShapes; ///< the objects this strategy will act on.
    KoCanvasBase *m_canvas; ///< the canvas which contains getters for document-data
};

#endif /* KODEF_H */
