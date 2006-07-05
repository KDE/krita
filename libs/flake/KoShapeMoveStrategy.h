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

#ifndef KODEFMOVE_H
#define KODEFMOVE_H

#include "KoInteractionStrategy.h"
#include "KoCommand.h"

#include <QPointF>

#include <koffice_export.h>

class KoCanvasBase;
class KoTool;

/**
 * Implements the Move action on an object or selected objects.
 */
class FLAKE_EXPORT KoShapeMoveStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that starts to move the objects.
     * @param tool the parent tool which controls this strategy
     * @param canvas the canvas interface which will supply things like a selection object
     * @param clicked the initial point that the user depressed (in pt).
     */
    KoShapeMoveStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked );
    virtual ~KoShapeMoveStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    KCommand* createCommand();
    void finishInteraction() { }

private:
    QList<QPointF> m_previousPositions;
    QList<QPointF> m_newPositions;
    QPointF m_initialTopLeft, m_start, m_diff, m_initialSelectionPosition;
};

#endif /* KODEFMOVE_H */
