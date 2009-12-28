/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>

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

#ifndef KOINTERACTIONSTRATEGY_H
#define KOINTERACTIONSTRATEGY_H

#include "flake_export.h"

#include <Qt>

class KoPointerEvent;
class KoViewConverter;
class KoInteractionStrategyPrivate;
class KoTool;
class QUndoCommand;
class QPointF;
class QPainter;

/**
 * Abstract interface to define what actions a KoInteractionTool can do based on
 * the Strategy design pattern.
 * e.g, move, select, transform.

 * KoInteractionStrategy is a Strategy baseclass for the KoInteractionTool and it
 * defines the behavior in case the user clicks or drags the input device.
 * The strategy is created in the createPolicy() function which defines the
 * resulting behavior and initiates a move or a resize, for example.
 * The mouseMove events are forwarded to the handleMouseMove() method and the interaction
 * is either finished with finishInteraction() or cancelInteraction() (never both).
 */
class FLAKE_EXPORT KoInteractionStrategy
{
public:
    /// constructor
    KoInteractionStrategy(KoTool *parent);
    /// Destructor
    virtual ~KoInteractionStrategy();

    /**
     * Reimplement this if the action needs to draw a "blob" on the canvas;
     * that is, a transient decoration like a rubber band.
     */
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    /**
     * Extending classes should implement this method to update the selectedShapes
     * based on the new mouse position.
     * @param mouseLocation the new location in pt
     * @param modifiers OR-ed set of keys pressed.
     */
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) = 0;

    /**
     * Extending classes should implement this method to update the selectedShapes
     * based on the new pointer event. The default implementations does nothing.
     * @param event the new pointer event
     */
    virtual void handleCustomEvent(KoPointerEvent *event);

    /**
     * For interactions that are undo-able this method should be implemented to return such
     * a command.  Implementations should return 0 otherwise.
     * @return a command, or 0.
     */
    virtual QUndoCommand *createCommand() = 0;
    /**
     * This method will undo frames based interactions by calling createCommand()
     * and unexecuting that.
     */
    virtual void cancelInteraction();
    /**
     * Override to make final changes to the data on the end of an interaction.
     */
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers) = 0;

    KoTool *tool() const;

protected:
    /// constructor
    KoInteractionStrategy(KoInteractionStrategyPrivate &);

    KoInteractionStrategyPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoInteractionStrategy)
};

#endif
