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

#include "kis_algebra_2d.h"

#include <kis_debug.h>

#define SANITY_CHECKS

namespace KisAlgebra2D {

void KRITAIMAGE_EXPORT adjustIfOnPolygonBoundary(const QPolygonF &poly, int polygonDirection, QPointF *pt)
{
    const int numPoints = poly.size();
    for (int i = 0; i < numPoints; i++) {
        int nextI = i + 1;
        if (nextI >= numPoints) {
            nextI = 0;
        }

        const QPointF &p0 = poly[i];
        const QPointF &p1 = poly[nextI];

        QPointF edge = p1 - p0;

        qreal cross = crossProduct(edge, *pt - p0)
            / (0.5 * edge.manhattanLength());

        if (cross < 1.0 &&
            isInRange(pt->x(), p0.x(), p1.x()) &&
            isInRange(pt->y(), p0.y(), p1.y())) {

            QPointF salt = 1.0e-3 * inwardUnitNormal(edge, polygonDirection);
            *pt += salt;

#ifdef SANITY_CHECKS
            KIS_ASSERT_RECOVER_NOOP(kisDistanceToLine(*pt, QLineF(p0, p1)) > 1e-4);
#endif /* SANITY_CHECKS */
        }
    }
}

QPointF KRITAIMAGE_EXPORT transformAsBase(const QPointF &pt, const QPointF &base1, const QPointF &base2) {
    qreal len1 = norm(base1);
    if (len1 < 1e-5) return pt;
    qreal sin1 = base1.y() / len1;
    qreal cos1 = base1.x() / len1;

    qreal len2 = norm(base2);
    if (len2 < 1e-5) return QPointF();
    qreal sin2 = base2.y() / len2;
    qreal cos2 = base2.x() / len2;

    qreal sinD = sin2 * cos1 - cos2 * sin1;
    qreal cosD = cos1 * cos2 + sin1 * sin2;
    qreal scaleD = len2 / len1;

    QPointF result;
    result.rx() = scaleD * (pt.x() * cosD - pt.y() * sinD);
    result.ry() = scaleD * (pt.x() * sinD + pt.y() * cosD);

    return result;
}

qreal KRITAIMAGE_EXPORT angleBetweenVectors(const QPointF &v1, const QPointF &v2)
{
    qreal a1 = std::atan2(v1.y(), v1.x());
    qreal a2 = std::atan2(v2.y(), v2.x());

    return a2 - a1;
}

}
