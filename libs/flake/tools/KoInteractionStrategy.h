/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2007, 2009 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOINTERACTIONSTRATEGY_H
#define KOINTERACTIONSTRATEGY_H

#include "kritaflake_export.h"

#include <Qt>

class KoPointerEvent;
class KoViewConverter;
class KoInteractionStrategyPrivate;
class KoToolBase;
class KUndo2Command;
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
class KRITAFLAKE_EXPORT KoInteractionStrategy
{
public:
    /// constructor
    explicit KoInteractionStrategy(KoToolBase *parent);
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
     * For interactions that are undo-able this method should be implemented to return such
     * a command.  Implementations should return 0 otherwise.
     * @return a command, or 0.
     */
    virtual KUndo2Command *createCommand() = 0;
    /**
     * This method will undo frames based interactions by calling createCommand()
     * and unexecuting that.
     */
    virtual void cancelInteraction();
    /**
     * Override to make final changes to the data on the end of an interaction.
     */
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers) = 0;

    KoToolBase *tool() const;

protected:
    /// constructor
    KoInteractionStrategy(KoInteractionStrategyPrivate &);

    KoInteractionStrategyPrivate *d_ptr;

    /// Convenience function to get the global handle radius
    uint handleRadius() const;

    /// Cenvenience function to get the global grab sensitivity
    uint grabSensitivity() const;

private:
    Q_DECLARE_PRIVATE(KoInteractionStrategy)
};

#endif
