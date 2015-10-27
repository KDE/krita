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

#include <QtCore/qmath.h>

#include <QRect>
#include <QRegion>
#include <QPainterPath>
#include <QPolygonF>
#include <QPen>

#include <KoColorSpaceRegistry.h>

#include "kis_image_config.h"
#include "kis_debug.h"


namespace KritaUtils
{

    QSize optimalPatchSize()
    {
        KisImageConfig cfg;
        return QSize(cfg.updatePatchWidth(),
                     cfg.updatePatchHeight());
    }

    QVector<QRect> splitRectIntoPatches(const QRect &rc, const QSize &patchSize)
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

                if (!patchRect.isEmpty()) {
                    patches.append(patchRect);
                }
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
        totalRect = totalRect.adjusted(-1,-1,1,1);

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

    void KRITAIMAGE_EXPORT initAntsPen(QPen *antsPen, QPen *outlinePen,
                                       int antLength, int antSpace)
    {
        QVector<qreal> antDashPattern;
        antDashPattern << antLength << antSpace;

        *antsPen = QPen(Qt::CustomDashLine);
        antsPen->setDashPattern(antDashPattern);
        antsPen->setCosmetic(true);
        antsPen->setColor(Qt::black);

        *outlinePen = QPen(Qt::SolidLine);
        outlinePen->setCosmetic(true);
        outlinePen->setColor(Qt::white);
    }

    QString KRITAIMAGE_EXPORT prettyFormatReal(qreal value)
    {
        return QString("%1").arg(value, 6, 'f', 1);
    }

    qreal KRITAIMAGE_EXPORT maxDimensionPortion(const QRectF &bounds, qreal portion, qreal minValue)
    {
        qreal maxDimension = qMax(bounds.width(), bounds.height());
        return qMax(portion * maxDimension, minValue);
    }

    bool tryMergePoints(QPainterPath &path,
                        const QPointF &startPoint,
                        const QPointF &endPoint,
                        qreal &distance,
                        qreal distanceThreshold,
                        bool lastSegment)
    {
        qreal length = (endPoint - startPoint).manhattanLength();

        if (lastSegment || length > distanceThreshold) {
            if (lastSegment) {
                qreal wrappedLength =
                    (endPoint - QPointF(path.elementAt(0))).manhattanLength();

                if (length < distanceThreshold ||
                    wrappedLength < distanceThreshold) {

                    return true;
                }
            }

            distance = 0;
            return false;
        }

        distance += length;

        if (distance > distanceThreshold) {
            path.lineTo(endPoint);
            distance = 0;
        }

        return true;
    }

    QPainterPath trySimplifyPath(const QPainterPath &path, qreal lengthThreshold)
    {
        QPainterPath newPath;
        QPointF startPoint;
        qreal distance = 0;

        int count = path.elementCount();
        for (int i = 0; i < count; i++) {
            QPainterPath::Element e = path.elementAt(i);
            QPointF endPoint = QPointF(e.x, e.y);

            switch (e.type) {
            case QPainterPath::MoveToElement:
                newPath.moveTo(endPoint);
                break;
            case QPainterPath::LineToElement:
                if (!tryMergePoints(newPath, startPoint, endPoint,
                                    distance, lengthThreshold, i == count - 1)) {

                    newPath.lineTo(endPoint);
                }
                break;
            case QPainterPath::CurveToElement: {
                Q_ASSERT(i + 2 < count);

                if (!tryMergePoints(newPath, startPoint, endPoint,
                                    distance, lengthThreshold, i == count - 1)) {

                    e = path.elementAt(i + 1);
                    Q_ASSERT(e.type == QPainterPath::CurveToDataElement);
                    QPointF ctrl1 = QPointF(e.x, e.y);
                    e = path.elementAt(i + 2);
                    Q_ASSERT(e.type == QPainterPath::CurveToDataElement);
                    QPointF ctrl2 = QPointF(e.x, e.y);
                    newPath.cubicTo(ctrl1, ctrl2, endPoint);
                }

                i += 2;
            }
            default:
                ;
            }
            startPoint = endPoint;
        }

        return newPath;
    }

    QList<QPainterPath> splitDisjointPaths(const QPainterPath &path)
    {
        QList<QPainterPath> resultList;
        QList<QPolygonF> inputPolygons = path.toSubpathPolygons();

        foreach (const QPolygonF &poly, inputPolygons) {
            QPainterPath testPath;
            testPath.addPolygon(poly);

            if (resultList.isEmpty()) {
                resultList.append(testPath);
                continue;
            }

            QList<QPainterPath>::iterator it = resultList.begin();
            QList<QPainterPath>::iterator end = resultList.end();
            QList<QPainterPath>::iterator savedIt = end;

            bool wasMerged = false;

            while (it != end) {
                bool skipIncrement = false;

                if (it->intersects(testPath)) {
                    if (savedIt == end) {
                        it->addPath(testPath);
                        savedIt = it;
                    } else {
                        savedIt->addPath(*it);
                        it = resultList.erase(it);
                        skipIncrement = true;
                    }

                    wasMerged = true;
                }

                if (!skipIncrement) {
                    ++it;
                }
            }

            if (!wasMerged) {
                resultList.append(testPath);
            }
        }

        return resultList;
    }

    quint8 mergeOpacity(quint8 opacity, quint8 parentOpacity)
    {
        if (parentOpacity != OPACITY_OPAQUE_U8) {
            opacity = (int(opacity) * parentOpacity) / OPACITY_OPAQUE_U8;
        }
        return opacity;
    }

    QBitArray mergeChannelFlags(const QBitArray &childFlags, const QBitArray &parentFlags)
    {
        QBitArray flags = childFlags;

        if (!flags.isEmpty() &&
            !parentFlags.isEmpty() &&
            flags.size() == parentFlags.size()) {

            flags &= parentFlags;

        } else if (!parentFlags.isEmpty()) {
            flags = parentFlags;
        }

        return flags;
    }

    bool compareChannelFlags(QBitArray f1, QBitArray f2)
    {
        if (f1.isNull() && f2.isNull()) return true;

        if (f1.isNull()) {
            f1.fill(true, f2.size());
        }

        if (f2.isNull()) {
            f2.fill(true, f1.size());
        }

        return f1 == f2;
    }

    QString KRITAIMAGE_EXPORT toLocalizedOnOff(bool value) {
        return value ? i18n("on") : i18n("off");
    }

}
