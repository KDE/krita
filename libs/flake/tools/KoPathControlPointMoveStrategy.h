/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHCONTROLPOINTMOVESTRATEGY_H
#define KOPATHCONTROLPOINTMOVESTRATEGY_H

#include <QPointF>
#include "KoInteractionStrategy.h"
#include "KoPathPoint.h"
#include "KoPathPointData.h"

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
    ~KoPathControlPointMoveStrategy() override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    KUndo2Command* createCommand() override;

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

