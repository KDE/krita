/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOSHAPEROTATESTRATEGY_H
#define KOSHAPEROTATESTRATEGY_H

#include "KoInteractionStrategy.h"

#include <QPointF>

class KoCanvasBase;
class KoTool;

/**
 * A strategy for the KoInteractionTool.
 * This strategy is invoked when the user starts a rotate of a selection of objects,
 * the stategy will then rotate the objects interactively and provide a command afterwards.
 */
class KoShapeRotateStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructor that starts to rotate the objects.
     * @param tool the parent tool which controls this strategy
     * @param canvas the canvas interface which will supply things like a selection object
     * @param clicked the initial point that the user depressed (in pt).
     */
    KoShapeRotateStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked );
    virtual ~KoShapeRotateStrategy() {}

    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    KCommand* createCommand();
    void finishInteraction() { }
    virtual void paint( QPainter &painter, KoViewConverter &converter);

private:
    QRectF m_initialBoundingRect;
    QPointF m_start;
    QList<QPointF> m_startPositions;
    QList<QPointF> m_startAbsolutePositions;
    QList<double> m_initialAngles;
};

#endif /* KOSHAPEROTATESTRATEGY_H */

