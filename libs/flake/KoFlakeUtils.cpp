/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoFlakeUtils.h"

#include <kis_global.h>
#include <kis_algebra_2d.h>

#include <KoViewConverter.h>
#include <KoShapeManager.h>
#include <KoPathPoint.h>
#include <KoParameterShape.h>


KoPathPoint *KoFlake::findNearestPathEndPoint(const QPointF &position, KoShapeManager *shapeManager, qreal viewGrabThreshold, const KoViewConverter &viewConverter)
{
    const QRectF viewROI = QRectF(-viewGrabThreshold, -viewGrabThreshold, 2 * viewGrabThreshold, 2 * viewGrabThreshold);
    const QRectF roi = viewConverter.viewToDocument(viewROI).translated(position);
    const qreal docDistanceThreshold = 0.5 * KisAlgebra2D::maxDimension(roi);

    const QList<KoShape *> shapes = shapeManager->shapesAt(roi);

    KoPathPoint *nearestPoint = 0;
    qreal minDistance = std::numeric_limits<qreal>::max();

    Q_FOREACH(KoShape *s, shapes) {
        KoPathShape *path = dynamic_cast<KoPathShape*>(s);
        if (!path) continue;

        KoParameterShape *paramShape = dynamic_cast<KoParameterShape*>(s);
        if (paramShape && paramShape->isParametricShape()) continue;

        const uint subpathCount = path->subpathCount();
        for (uint i = 0; i < subpathCount; ++i) {
            if (path->isClosedSubpath(i)) continue;

            auto addIfSmaller = [&] (const KoPathPointIndex &index) {
                KoPathPoint *pt = path->pointByIndex(index);
                const qreal distance = kisDistance(position, path->shapeToDocument(pt->point()));

                if (distance < minDistance && distance < docDistanceThreshold) {
                    nearestPoint = pt;
                    minDistance = distance;
                }
            };

            // check start of subpath
            addIfSmaller(KoPathPointIndex(i, 0));

            // check end of subpath
            addIfSmaller(KoPathPointIndex(i, path->subpathPointCount(i) - 1));
        }
    }

    return nearestPoint;
}
