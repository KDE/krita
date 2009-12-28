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
#ifndef KOPATHPARAMETERCHANGESTRATEGY_H
#define KOPATHPARAMETERCHANGESTRATEGY_H

#include <QPointF>
#include "KoInteractionStrategy.h"

class KoParameterShape;

/// Strategy for changing control points of parametric shapes
class KoParameterChangeStrategy : public KoInteractionStrategy
{
public:
    /**
     * Constructs a strategy for changing control points of parametric shapes.
     * @param tool the tool the strategy belongs to
     * @param canvas canvas the canvas to paint on
     * @param parameterShape the parametric shapes the strategy is working on
     * @param handleId the id of the handle to modify
     */
    KoParameterChangeStrategy(KoTool *tool, KoParameterShape *parameterShape, int handleId);
    virtual ~KoParameterChangeStrategy();

    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers) {
        Q_UNUSED(modifiers);
    }
    virtual QUndoCommand* createCommand();

private:
    KoParameterShape * const m_parameterShape; ///< the parametric shape we are working on
    const int m_handleId;                      ///< the id of the control point
    const QPointF m_startPoint;                ///< the starting position of the control point
    Qt::KeyboardModifiers m_lastModifierUsed;
    QPointF m_releasePoint;
};

#endif /* KOPATHPARAMETERCHANGESTRATEGY_H */
