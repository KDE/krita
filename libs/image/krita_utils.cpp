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

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include "kis_algebra_2d.h"

#include <KoColorSpaceRegistry.h>

#include "kis_image_config.h"
#include "kis_debug.h"
#include "kis_node.h"
#include "kis_sequential_iterator.h"
#include "kis_random_accessor_ng.h"


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

    QVector<QRect> splitRegionIntoPatches(const QRegion &region, const QSize &patchSize)
    {
        QVector<QRect> patches;

        Q_FOREACH (const QRect rect, region.rects()) {
            patches << KritaUtils::splitRectIntoPatches(rect, patchSize);
        }

        return patches;
    }

    template <class Rect, class Point>
    QVector<Point> sampleRectWithPoints(const Rect &rect)
    {
        QVector<Point> points;

        Point m1 = 0.5 * (rect.topLeft() + rect.topRight());
        Point m2 = 0.5 * (rect.bottomLeft() + rect.bottomRight());

        points << rect.topLeft();
        points << m1;
        points << rect.topRight();

        points << 0.5 * (rect.topLeft() + rect.bottomLeft());
        points << 0.5 * (m1 + m2);
        points << 0.5 * (rect.topRight() + rect.bottomRight());

        points << rect.bottomLeft();
        points << m2;
        points << rect.bottomRight();

        return points;
    }

    QVector<QPoint> sampleRectWithPoints(const QRect &rect)
    {
        return sampleRectWithPoints<QRect, QPoint>(rect);
    }

    QVector<QPointF> sampleRectWithPoints(const QRectF &rect)
    {
        return sampleRectWithPoints<QRectF, QPointF>(rect);
    }


    template <class Rect, class Point, bool alignPixels>
    Rect approximateRectFromPointsImpl(const QVector<Point> &points)
    {
        using namespace boost::accumulators;
        accumulator_set<qreal, stats<tag::min, tag::max > > accX;
        accumulator_set<qreal, stats<tag::min, tag::max > > accY;

        Q_FOREACH (const Point &pt, points) {
            accX(pt.x());
            accY(pt.y());
        }

        Rect resultRect;

        if (alignPixels) {
            resultRect.setCoords(std::floor(min(accX)), std::floor(min(accY)),
                                 std::ceil(max(accX)), std::ceil(max(accY)));
        } else {
            resultRect.setCoords(min(accX), min(accY),
                                 max(accX), max(accY));
        }

        return resultRect;
    }

    QRect approximateRectFromPoints(const QVector<QPoint> &points)
    {
        return approximateRectFromPointsImpl<QRect, QPoint, true>(points);
    }

    QRectF approximateRectFromPoints(const QVector<QPointF> &points)
    {
        return approximateRectFromPointsImpl<QRectF, QPointF, false>(points);
    }

    QRect approximateRectWithPointTransform(const QRect &rect, std::function<QPointF(QPointF)> func)
    {
        QVector<QPoint> points = KritaUtils::sampleRectWithPoints(rect);

        using namespace boost::accumulators;
        accumulator_set<qreal, stats<tag::min, tag::max > > accX;
        accumulator_set<qreal, stats<tag::min, tag::max > > accY;

        Q_FOREACH (const QPoint &pt, points) {
            QPointF dstPt = func(pt);

            accX(dstPt.x());
            accY(dstPt.y());
        }

        QRect resultRect;
        resultRect.setCoords(std::floor(min(accX)), std::floor(min(accY)),
                             std::ceil(max(accX)), std::ceil(max(accY)));

        return resultRect;
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
        for(int i = 0; i < size.height(); ++i) {
            for(int j = 0; j < size.width(); ++j) {
                const QRgb pixel = image.pixel(i,j);
                const int gray = qGray(pixel);
                dstImage.setPixel(i, j, qRgba(gray, gray, gray, qAlpha(pixel)));
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
        do {
            const quint8 *dstPtr = dstIt.rawDataConst();
            func(*dstPtr);
        } while (dstIt.nextPixel());
    }

    void filterAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<quint8(quint8)> func) {
        KisSequentialIterator dstIt(dev, rc);
        do {
            quint8 *dstPtr = dstIt.rawData();
            *dstPtr = func(*dstPtr);
        } while (dstIt.nextPixel());
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
}
