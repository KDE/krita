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
#include <QPainter>

#include "kis_algebra_2d.h"

#include <KoColorSpaceRegistry.h>

#include "kis_image_config.h"
#include "kis_debug.h"
#include "kis_node.h"
#include "kis_sequential_iterator.h"
#include "kis_random_accessor_ng.h"

#include <KisRenderedDab.h>


namespace KritaUtils
{

    QSize optimalPatchSize()
    {
        KisImageConfig cfg(true);
        return QSize(cfg.updatePatchWidth(),
                     cfg.updatePatchHeight());
    }

    QVector<QRect> splitRectIntoPatches(const QRect &rc, const QSize &patchSize)
    {
        using namespace KisAlgebra2D;


        QVector<QRect> patches;

        const qint32 firstCol = divideFloor(rc.x(), patchSize.width());
        const qint32 firstRow = divideFloor(rc.y(), patchSize.height());

        // TODO: check if -1 is needed here
        const qint32 lastCol = divideFloor(rc.x() + rc.width(), patchSize.width());
        const qint32 lastRow = divideFloor(rc.y() + rc.height(), patchSize.height());

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

    QVector<QRect> splitRegionIntoPatches(const QRegion &region, const QSize &patchSize)
    {
        QVector<QRect> patches;

        Q_FOREACH (const QRect rect, region.rects()) {
            patches << KritaUtils::splitRectIntoPatches(rect, patchSize);
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

                Q_FOREACH (const QPolygonF &triangle, triangles) {
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

        Q_FOREACH (const QPolygonF &poly, inputPolygons) {
            QPainterPath testPath;
            testPath.addPolygon(poly);

            if (resultList.isEmpty()) {
                resultList.append(testPath);
                continue;
            }

            QPainterPath mergedPath = testPath;

            for (auto it = resultList.begin(); it != resultList.end(); /*noop*/) {
                if (it->intersects(testPath)) {
                    mergedPath.addPath(*it);
                    it = resultList.erase(it);
                } else {
                    ++it;
                }
            }

            resultList.append(mergedPath);
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

    KisNodeSP nearestNodeAfterRemoval(KisNodeSP node)
    {
        KisNodeSP newNode = node->nextSibling();

        if (!newNode) {
            newNode = node->prevSibling();
        }

        if (!newNode) {
            newNode = node->parent();
        }

        return newNode;
    }

    void renderExactRect(QPainter *p, const QRect &rc)
    {
        p->drawRect(rc.adjusted(0,0,-1,-1));
    }

    void renderExactRect(QPainter *p, const QRect &rc, const QPen &pen)
    {
        QPen oldPen = p->pen();
        p->setPen(pen);
        renderExactRect(p, rc);
        p->setPen(oldPen);
    }

    QImage convertQImageToGrayA(const QImage &image)
    {
        QImage dstImage(image.size(), QImage::Format_ARGB32);

        // TODO: if someone feel bored, a more optimized version of this would be welcome
        const QSize size = image.size();
        for(int y = 0; y < size.height(); ++y) {
            for(int x = 0; x < size.width(); ++x) {
                const QRgb pixel = image.pixel(x,y);
                const int gray = qGray(pixel);
                dstImage.setPixel(x, y, qRgba(gray, gray, gray, qAlpha(pixel)));
            }
        }

        return dstImage;
    }

    QColor blendColors(const QColor &c1, const QColor &c2, qreal r1)
    {
        const qreal r2 = 1.0 - r1;

        return QColor::fromRgbF(
            c1.redF() * r1 + c2.redF() * r2,
            c1.greenF() * r1 + c2.greenF() * r2,
            c1.blueF() * r1 + c2.blueF() * r2);
    }

    void applyToAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<void(quint8)> func) {
        KisSequentialConstIterator dstIt(dev, rc);
        while (dstIt.nextPixel()) {
            const quint8 *dstPtr = dstIt.rawDataConst();
            func(*dstPtr);
        }
    }

    void filterAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<quint8(quint8)> func) {
        KisSequentialIterator dstIt(dev, rc);
        while (dstIt.nextPixel()) {
            quint8 *dstPtr = dstIt.rawData();
            *dstPtr = func(*dstPtr);
        }
    }

    qreal estimatePortionOfTransparentPixels(KisPaintDeviceSP dev, const QRect &rect, qreal samplePortion) {
        const KoColorSpace *cs = dev->colorSpace();

        const qreal linearPortion = std::sqrt(samplePortion);
        const qreal ratio = qreal(rect.width()) / rect.height();
        const int xStep = qMax(1, qRound(1.0 / linearPortion * ratio));
        const int yStep = qMax(1, qRound(1.0 / linearPortion / ratio));

        int numTransparentPixels = 0;
        int numPixels = 0;

        KisRandomConstAccessorSP it = dev->createRandomConstAccessorNG(rect.x(), rect.y());
        for (int y = rect.y(); y <= rect.bottom(); y += yStep) {
            for (int x = rect.x(); x <= rect.right(); x += xStep) {
                it->moveTo(x, y);
                const quint8 alpha = cs->opacityU8(it->rawDataConst());

                if (alpha != OPACITY_OPAQUE_U8) {
                    numTransparentPixels++;
                }

                numPixels++;
            }
        }

        return qreal(numTransparentPixels) / numPixels;
    }

    void mirrorDab(Qt::Orientation dir, const QPoint &center, KisRenderedDab *dab)
    {
        const QRect rc = dab->realBounds();

        if (dir == Qt::Horizontal) {
            const int mirrorX = -((rc.x() + rc.width()) - center.x()) + center.x();

            dab->device->mirror(true, false);
            dab->offset.rx() = mirrorX;
        } else /* if (dir == Qt::Vertical) */ {
            const int mirrorY = -((rc.y() + rc.height()) - center.y()) + center.y();

            dab->device->mirror(false, true);
            dab->offset.ry() = mirrorY;
        }
    }

    void mirrorRect(Qt::Orientation dir, const QPoint &center, QRect *rc)
    {
        if (dir == Qt::Horizontal) {
            const int mirrorX = -((rc->x() + rc->width()) - center.x()) + center.x();
            rc->moveLeft(mirrorX);
        } else /* if (dir == Qt::Vertical) */ {
            const int mirrorY = -((rc->y() + rc->height()) - center.y()) + center.y();
            rc->moveTop(mirrorY);
        }
    }

    void mirrorPoint(Qt::Orientation dir, const QPoint &center, QPointF *pt)
    {
        if (dir == Qt::Horizontal) {
            pt->rx() = -(pt->x() - qreal(center.x())) + center.x();
        } else /* if (dir == Qt::Vertical) */ {
            pt->ry() = -(pt->y() - qreal(center.y())) + center.y();
        }
    }

    qreal colorDifference(const QColor &c1, const QColor &c2)
    {
        const qreal dr = c1.redF() - c2.redF();
        const qreal dg = c1.greenF() - c2.greenF();
        const qreal db = c1.blueF() - c2.blueF();

        return std::sqrt(2 * pow2(dr) + 4 * pow2(dg) + 3 * pow2(db));
    }

    void dragColor(QColor *color, const QColor &baseColor, qreal threshold)
    {
        while (colorDifference(*color, baseColor) < threshold) {

            QColor newColor = *color;

            if (newColor.lightnessF() > baseColor.lightnessF()) {
                newColor = newColor.lighter(120);
            } else {
                newColor = newColor.darker(120);
            }

            if (newColor == *color) {
                break;
            }

            *color = newColor;
        }
    }


}
