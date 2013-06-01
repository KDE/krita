/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "krita_utils.h"

#include <QRect>
#include <QRegion>
#include <QPainterPath>
#include <QPolygonF>

namespace KritaUtils
{

    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatches(const QRect &rc, const QSize &patchSize)
    {
        QVector<QRect> patches;

        qint32 firstCol = rc.x() / patchSize.width();
        qint32 firstRow = rc.y() / patchSize.height();

        qint32 lastCol = (rc.x() + rc.width()) / patchSize.width();
        qint32 lastRow = (rc.y() + rc.height()) / patchSize.height();

        for(qint32 i = firstRow; i <= lastRow; i++) {
            for(qint32 j = firstCol; j <= lastCol; j++) {
                QRect maxPatchRect(j * patchSize.width(), i * patchSize.height(),
                                   patchSize.width(), patchSize.height());
                QRect patchRect = rc & maxPatchRect;

                patches.append(patchRect);
            }
        }

        return patches;
    }

    bool checkInTriangle(const QRectF &rect,
                         const QPolygonF &triangle)
    {
        return triangle.intersected(rect).boundingRect().isValid();
    }


    QRegion KRITAIMAGE_EXPORT splitTriangles(const QPointF &center,
                                             const QVector<QPointF> &points)
    {

        Q_ASSERT(points.size());
        Q_ASSERT(!(points.size() & 1));

        QVector<QPolygonF> triangles;
        QRect totalRect;

        for (int i = 0; i < points.size(); i += 2) {
            QPolygonF triangle;
            triangle << center;
            triangle << points[i];
            triangle << points[i+1];

            totalRect |= triangle.boundingRect().toAlignedRect();
            triangles << triangle;
        }


        const int step = 64;
        const int right = totalRect.x() + totalRect.width();
        const int bottom = totalRect.y() + totalRect.height();

        QRegion dirtyRegion;

        for (int y = totalRect.y(); y < bottom;) {
            int nextY = qMin((y + step) & ~(step-1), bottom);

            for (int x = totalRect.x(); x < right;) {
                int nextX = qMin((x + step) & ~(step-1), right);

                QRect rect(x, y, nextX - x, nextY - y);

                foreach(const QPolygonF &triangle, triangles) {
                    if(checkInTriangle(rect, triangle)) {
                        dirtyRegion |= rect;
                        break;
                    }
                }

                x = nextX;
            }
            y = nextY;
        }
        return dirtyRegion;
    }

    QRegion KRITAIMAGE_EXPORT splitPath(const QPainterPath &path)
    {
        QRect totalRect = path.boundingRect().toAlignedRect();

        // adjust the rect for antialiasing to work
        totalRect.adjusted(-1,-1,1,1);

        const int step = 64;
        const int right = totalRect.x() + totalRect.width();
        const int bottom = totalRect.y() + totalRect.height();

        QRegion dirtyRegion;


        for (int y = totalRect.y(); y < bottom;) {
            int nextY = qMin((y + step) & ~(step-1), bottom);

            for (int x = totalRect.x(); x < right;) {
                int nextX = qMin((x + step) & ~(step-1), right);

                QRect rect(x, y, nextX - x, nextY - y);

                if(path.intersects(rect)) {
                    dirtyRegion |= rect;
                }

                x = nextX;
            }
            y = nextY;
        }

        return dirtyRegion;
    }

}
