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

#ifndef KODEFRUBBERSELECT_H
#define KODEFRUBBERSELECT_H

#include "KoInteractionStrategy.h"

#include <QRectF>

#include "flake_export.h"

class KoToolBase;
class KoShapeRubberSelectStrategyPrivate;

/**
 * Implement the rubber band selection of flake objects.
 */
class FLAKE_EXPORT KoShapeRubberSelectStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that initiates the rubber select.
     * A rubber select is basically rectangle area that the user drags out
     * from @p clicked to a point later provided in the handleMouseMove() continuously
     * showing a semi-transarant 'rubber-mat' over the objects it is about to select.
     * @param tool the parent tool which controls this strategy
     * @param canvas The canvas that owns the tool for this strategy.
     * @param clicked the initial point that the user depressed (in pt).
     * @param useSnapToGrid use the snap-to-grid settings while doing the rubberstamp.
     */
    KoShapeRubberSelectStrategy(KoToolBase *tool, const QPointF &clicked, bool useSnapToGrid = false);

    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QUndoCommand *createCommand();
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);

protected:
    /// constructor
    KoShapeRubberSelectStrategy(KoShapeRubberSelectStrategyPrivate &);

private:
    Q_DECLARE_PRIVATE(KoShapeRubberSelectStrategy)
};

#endif /* KODEFRUBBERSELECT_H */
