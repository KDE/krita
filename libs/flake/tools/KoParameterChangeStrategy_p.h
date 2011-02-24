/* This file is part of the KDE project
 *   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 *   Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * 
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 * 
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 * 
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPARAMETERCHANGESTRATEGYPRIVATE_H
#define KOPARAMETERCHANGESTRATEGYPRIVATE_H

#include "KoInteractionStrategy_p.h"
#include "KoParameterShape.h"

class KoParameterChangeStrategyPrivate : public KoInteractionStrategyPrivate
{
public:
    KoParameterChangeStrategyPrivate(KoToolBase *owner, KoParameterShape *paramShape, int handle)
        : KoInteractionStrategyPrivate(owner), parameterShape(paramShape), handleId(handle)
        , startPoint(paramShape->shapeToDocument(paramShape->handlePosition(handle)))
        , lastModifierUsed(0)
    {
        // initialize release point with start point position to prevent
        // change when just clicking a handle without moving the mouse
        releasePoint = startPoint;
    }
    KoParameterShape * const parameterShape; ///< the parametric shape we are working on
    const int handleId;                      ///< the id of the control point
    const QPointF startPoint;                ///< the starting position of the control point
    QPointF releasePoint;
    Qt::KeyboardModifiers lastModifierUsed;
};

#endif // KOPARAMETERCHANGESTRATEGYPRIVATE_H
