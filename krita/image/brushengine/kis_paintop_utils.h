/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_PAINTOP_UTILS_H
#define __KIS_PAINTOP_UTILS_H

#include "kis_global.h"
#include "kis_paint_information.h"


namespace KisPaintOpUtils {

template <class PaintOp>
bool paintFan(PaintOp &op,
              const KisPaintInformation &pi1,
              const KisPaintInformation &pi2,
              KisDistanceInformation *currentDistance,
              qreal fanCornersStep)
{
    const qreal angleStep = fanCornersStep;
    const qreal initialAngle = currentDistance->lastDrawingAngle();
    const qreal finalAngle = pi2.drawingAngleSafe(*currentDistance);
    const qreal fullDistance = shortestAngularDistance(initialAngle,
                                                       pi2.drawingAngleSafe(*currentDistance));
    qreal lastAngle = initialAngle;

    int i = 0;

    while (shortestAngularDistance(lastAngle, finalAngle) > angleStep) {
        lastAngle = incrementInDirection(lastAngle, angleStep, finalAngle);

        qreal t = angleStep * i++ / fullDistance;

        QPointF pt = pi1.pos() + t * (pi2.pos() - pi1.pos());
        KisPaintInformation pi = KisPaintInformation::mix(pt, t, pi1, pi2);
        pi.overrideDrawingAngle(lastAngle);
        pi.paintAt(op, currentDistance);
    }

    return i;
}


template <class PaintOp>
void paintLine(PaintOp &op,
               const KisPaintInformation &pi1,
               const KisPaintInformation &pi2,
               KisDistanceInformation *currentDistance,
               bool fanCornersEnabled,
               qreal fanCornersStep)
{
    QPointF end = pi2.pos();

    KisPaintInformation pi = pi1;
    QPointF pt = pi1.pos();
    qreal t = 0.0;

    while ((t = currentDistance->getNextPointPosition(pt, end)) >= 0.0) {
        pt = pt + t * (end - pt);
        pi = KisPaintInformation::mix(pt, t, pi, pi2);

        if (fanCornersEnabled &&
            currentDistance->hasLastPaintInformation()) {

            paintFan(op,
                     currentDistance->lastPaintInformation(),
                     pi,
                     currentDistance,
                     fanCornersStep);
        }

        /**
         * A bit complicated part to ensure the registration
         * of the distance information is done in right order
         */
        pi.paintAt(op, currentDistance);
    }
}

}

#endif /* __KIS_PAINTOP_UTILS_H */
