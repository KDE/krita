/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KO_PATH_CONNECTION_POINT_STRATEGY
#define KO_PATH_CONNECTION_POINT_STRATEGY

#include "flake_export.h"
#include "KoParameterChangeStrategy.h"

class KoCanvasBase;
class KoConnectionShape;
class QPointF;
class KoPathConnectionPointStrategyPrivate;

/**
 * @brief Strategy for moving end points of a connection shape.
 */
class FLAKE_EXPORT KoPathConnectionPointStrategy : public KoParameterChangeStrategy
{
public:
    KoPathConnectionPointStrategy(KoToolBase *tool, KoConnectionShape *parameterShape, int handleId);
    virtual ~KoPathConnectionPointStrategy();
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);
    virtual KUndo2Command* createCommand();

private:
    Q_DECLARE_PRIVATE(KoPathConnectionPointStrategy)
};


#endif
