/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOPATHPOINTRUBBERSELECTSTRATEGY_H
#define KOPATHPOINTRUBBERSELECTSTRATEGY_H

#include "KoShapeRubberSelectStrategy.h"

class KoPathTool;

/**
 * @brief Strategy to rubber select points of a path shape
 */
class KoPathPointRubberSelectStrategy : public KoShapeRubberSelectStrategy
{
public:
    KoPathPointRubberSelectStrategy(KoPathTool *tool, const QPointF &clicked);
    ~KoPathPointRubberSelectStrategy() override {}

    void handleMouseMove(const QPointF &p, Qt::KeyboardModifiers modifiers) override;
    void finishInteraction(Qt::KeyboardModifiers modifiers) override;
    void cancelInteraction() override;

private:
    /// pointer to the path tool
    KoPathTool *m_tool;
    Q_DECLARE_PRIVATE(KoShapeRubberSelectStrategy)
};

#endif /* KOPATHPOINTRUBBERSELECTSTRATEGY_H */
