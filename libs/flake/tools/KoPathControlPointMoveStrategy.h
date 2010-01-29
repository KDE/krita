/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPATHCONTROLPOINTMOVESTRATEGY_H
#define KOPATHCONTROLPOINTMOVESTRATEGY_H

#include <QPointF>
#include "KoInteractionStrategy.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"

class KoCanvasBase;
class KoPathTool;

/**
 * /internal
 * @brief Strategy for moving points of a path shape.
 */
class KoPathControlPointMoveStrategy : public KoInteractionStrategy
{
public:
    KoPathControlPointMoveStrategy(KoPathTool *tool, const KoPathPointData &point,
                                   KoPathPoint::PointType type, const QPointF &pos);
    virtual ~KoPathControlPointMoveStrategy();
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);
    virtual QUndoCommand* createCommand();

private:
    /// the last mouse position
    QPointF m_lastPosition;
    /// the accumulated point move amount
    QPointF m_move;

    KoPathTool *m_tool;
    KoPathPointData m_pointData;
    KoPathPoint::PointType m_pointType;
};

#endif /* KOPATHCONTROLPOINTMOVESTRATEGY_H */

