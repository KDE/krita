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

#ifndef KOSHAPERUBBERSELECTSTRATEGY_H
#define KOSHAPERUBBERSELECTSTRATEGY_H

#include "KoInteractionStrategy.h"

#include <QRectF>

#include "kritaflake_export.h"

class KoToolBase;
class KoShapeRubberSelectStrategyPrivate;

/**
 * This is a base class for interactions based on dragging a rectangular area on the canvas,
 * such as selection, zooming or shape creation.
 *
 * When the user selects stuff in left-to-right way, selection is in "covering"
 * (or "containing") mode, when in "left-to-right" in "crossing" mode
 */
class KRITAFLAKE_EXPORT KoShapeRubberSelectStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that initiates the rubber select.
     * A rubber select is basically rectangle area that the user drags out
     * from @p clicked to a point later provided in the handleMouseMove() continuously
     * showing a semi-transarant 'rubber-mat' over the objects it is about to select.
     * @param tool the parent tool which controls this strategy
     * @param clicked the initial point that the user depressed (in pt).
     * @param useSnapToGrid use the snap-to-grid settings while doing the rubberstamp.
     */
    KoShapeRubberSelectStrategy(KoToolBase *tool, const QPointF &clicked, bool useSnapToGrid = false);

    void paint(QPainter &painter, const KoViewConverter &converter) override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;

protected:
    /// constructor
    KoShapeRubberSelectStrategy(KoShapeRubberSelectStrategyPrivate &);

    QRectF selectedRectangle() const;

    enum SelectionMode {
        CrossingSelection,
        CoveringSelection
    };

    virtual SelectionMode currentMode() const;

private:
    Q_DECLARE_PRIVATE(KoShapeRubberSelectStrategy)
};

#endif /* KOSHAPERUBBERSELECTSTRATEGY_H */
