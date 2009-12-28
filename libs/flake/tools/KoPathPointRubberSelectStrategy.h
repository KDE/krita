/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
    virtual ~KoPathPointRubberSelectStrategy() {}
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);

private:
    /// pointer to the path tool
    KoPathTool *m_tool;
    Q_DECLARE_PRIVATE(KoShapeRubberSelectStrategy)
};

#endif /* KOPATHPOINTRUBBERSELECTSTRATEGY_H */
