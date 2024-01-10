/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHPOINTMOVESTRATEGY_H
#define KOPATHPOINTMOVESTRATEGY_H

#include <QPointF>
#include <QScopedPointer>
#include "KoInteractionStrategy.h"


class KoPathTool;

/**
 * @brief Strategy for moving points of a path shape.
 */
class KoPathPointMoveStrategy : public KoInteractionStrategy
{
public:
    KoPathPointMoveStrategy(KoPathTool *tool, const QPointF &mousePosition, const QPointF &pointPosition);
    ~KoPathPointMoveStrategy() override;
    void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    KUndo2Command *createCommand() override;

private:
    QPointF m_startMousePosition;
    QPointF m_startPointPosition;
    /// the accumulated point move amount
    QPointF m_move;
    /// pointer to the path tool
    KoPathTool *m_tool;
    QScopedPointer<KUndo2Command> m_intermediateCommand;
};

#endif /* KOPATHPOINTMOVESTRATEGY_H */

