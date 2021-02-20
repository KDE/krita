/* This file is part of the KDE project
 *   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 * 
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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
