/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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
#ifndef KOPARAMETERCHANGESTRATEGY_H
#define KOPARAMETERCHANGESTRATEGY_H

#include "flake_export.h"
#include "KoInteractionStrategy.h"
#include <QPointF>

class KoParameterShape;
class KoParameterChangeStrategyPrivate;

/// Strategy for changing control points of parametric shapes
class FLAKE_EXPORT KoParameterChangeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructs a strategy for changing control points of parametric shapes.
     * @param tool the tool the strategy belongs to
     * @param parameterShape the parametric shapes the strategy is working on
     * @param handleId the id of the handle to modify
     */
    KoParameterChangeStrategy(KoToolBase *tool, KoParameterShape *parameterShape, int handleId);
    virtual ~KoParameterChangeStrategy();

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);
    virtual KUndo2Command* createCommand();

protected:
    /// constructor
    KoParameterChangeStrategy(KoParameterChangeStrategyPrivate &);

private:
    Q_DECLARE_PRIVATE(KoParameterChangeStrategy)
};

#endif /* KOPARAMETERCHANGESTRATEGY_H */
