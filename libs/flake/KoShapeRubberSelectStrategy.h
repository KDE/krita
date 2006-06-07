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

class QPainter;
class QPointF;

class KoPointerEvent;
class KoCanvasBase;
class KoTool;

/**
 * Implement the rubber band selection of flake objects.
 */
class KoShapeRubberSelectStrategy : public KoInteractionStrategy
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
     */
    KoShapeRubberSelectStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked );
    virtual ~KoShapeRubberSelectStrategy();

    void paint( QPainter &painter, KoViewConverter &converter);
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual KCommand* createCommand() { return 0; }
    virtual void finishInteraction();

protected:
    const QRectF selectRect() const;

private:
    QRectF m_selectRect;
};

#endif /* KODEFRUBBERSELECT_H */
