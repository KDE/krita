/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KRITA_UTILS_H
#define __KRITA_UTILS_H

class QRect;
class QRectF;
class QSize;
class QPen;
class QPointF;
class QPainterPath;
class QBitArray;
class QPainter;
struct KisRenderedDab;
class KisRegion;

#include <QVector>
#include "kritaimage_export.h"
#include "kis_types.h"
#include "krita_container_utils.h"
#include <functional>


namespace KritaUtils
{
    QSize KRITAIMAGE_EXPORT optimalPatchSize();

    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatches(const QRect &rc, const QSize &patchSize);
    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatchesTight(const QRect &rc, const QSize &patchSize);
    QVector<QRect> KRITAIMAGE_EXPORT splitRegionIntoPatches(const QRegion &region, const QSize &patchSize);
    QVector<QRect> KRITAIMAGE_EXPORT splitRegionIntoPatches(const KisRegion &region, const QSize &patchSize);

    KRITAIMAGE_EXPORT KisRegion splitTriangles(const QPointF &center,
                                             const QVector<QPointF> &points);
    KRITAIMAGE_EXPORT KisRegion splitPath(const QPainterPath &path);

    QString KRITAIMAGE_EXPORT prettyFormatReal(qreal value);

    qreal KRITAIMAGE_EXPORT maxDimensionPortion(const QRectF &bounds, qreal portion, qreal minValue);
    QPainterPath KRITAIMAGE_EXPORT trySimplifyPath(const QPainterPath &path, qreal lengthThreshold);

    /**
     * Split a path \p path into a set of disjoint (non-intersectable)
     * paths if possible.
     *
     * It tries to follow odd-even fill rule, but has a small problem:
     * If you have three selections included into each other twice,
     * then the smallest selection will be included into the final subpath,
     * although it shouldn't according to odd-even-fill rule. It is still
     * to be fixed.
     */
    QList<QPainterPath> KRITAIMAGE_EXPORT splitDisjointPaths(const QPainterPath &path);


    quint8 KRITAIMAGE_EXPORT mergeOpacity(quint8 opacity, quint8 parentOpacity);
    QBitArray KRITAIMAGE_EXPORT mergeChannelFlags(const QBitArray &flags, const QBitArray &parentFlags);

    bool KRITAIMAGE_EXPORT compareChannelFlags(QBitArray f1, QBitArray f2);
    QString KRITAIMAGE_EXPORT toLocalizedOnOff(bool value);

    KisNodeSP KRITAIMAGE_EXPORT nearestNodeAfterRemoval(KisNodeSP node);

    /**
     * When drawing a rect Qt uses quite a weird algorithm. It
     * draws 4 lines:
     *  o at X-es: rect.x() and rect.right() + 1
     *  o at Y-s: rect.y() and rect.bottom() + 1
     *
     *  Which means that bottom and right lines of the rect are painted
     *  outside the virtual rectangle the rect defines. This methods overcome this issue by
     *  painting the adjusted rect.
     */
    void KRITAIMAGE_EXPORT renderExactRect(QPainter *p, const QRect &rc);

    /**
     * \see renderExactRect(QPainter *p, const QRect &rc)
     */
    void KRITAIMAGE_EXPORT renderExactRect(QPainter *p, const QRect &rc, const QPen &pen);

    QImage KRITAIMAGE_EXPORT convertQImageToGrayA(const QImage &image);

    void KRITAIMAGE_EXPORT applyToAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<void(quint8)> func);
    void KRITAIMAGE_EXPORT filterAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<quint8(quint8)> func);

    qreal KRITAIMAGE_EXPORT estimatePortionOfTransparentPixels(KisPaintDeviceSP dev, const QRect &rect, qreal samplePortion);

    void KRITAIMAGE_EXPORT mirrorDab(Qt::Orientation dir, const QPoint &center, KisRenderedDab *dab, bool skipMirrorPixels = false);
    void KRITAIMAGE_EXPORT mirrorDab(Qt::Orientation dir, const QPointF &center, KisRenderedDab *dab, bool skipMirrorPixels = false);

    void KRITAIMAGE_EXPORT mirrorRect(Qt::Orientation dir, const QPoint &center, QRect *rc);
    void KRITAIMAGE_EXPORT mirrorRect(Qt::Orientation dir, const QPointF &center, QRect *rc);
    void KRITAIMAGE_EXPORT mirrorPoint(Qt::Orientation dir, const QPoint &center, QPointF *pt);
    void KRITAIMAGE_EXPORT mirrorPoint(Qt::Orientation dir, const QPointF &center, QPointF *pt);


    /**
     * Returns a special transformation that converts vector shape coordinates
     * ('pt') into a special coordinate space, where all path boolean operations
     * should happen.
     *
     * The problem is that Qt's path boolean operation do not support curves,
     * therefore all the curves are converted into lines
     * (see QPathSegments::addPath()). The curves are split into lines using
     * absolute size of the curve for the threshold. Therefore, when applying
     * boolean operations we should convert them into 'image pixel' coordinate
     * space first.
     *
     * See https://bugs.kde.org/show_bug.cgi?id=411056
     */
    QTransform KRITAIMAGE_EXPORT pathShapeBooleanSpaceWorkaround(KisImageSP image);

    enum ThresholdMode {
        ThresholdNone = 0,
        ThresholdFloor,
        ThresholdCeil,
        ThresholdMaxOut
    };

    void thresholdOpacity(KisPaintDeviceSP device, const QRect &rect, ThresholdMode mode);
    void thresholdOpacityAlpha8(KisPaintDeviceSP device, const QRect &rect, ThresholdMode mode);

    template <typename Visitor>
    void rasterizeHLine(const QPoint &startPoint, const QPoint &endPoint, Visitor visitor)
    {
        QVector<QPoint> points;
        int startX, endX;
        if (startPoint.x() < endPoint.x()) {
            startX = startPoint.x();
            endX = endPoint.x();
        } else {
            startX = endPoint.x();
            endX = startPoint.x();
        }
        for (int x = startX; x <= endX; ++x) {
            visitor(QPoint(x, startPoint.y()));
        }
    }

    template <typename Visitor>
    void rasterizeVLine(const QPoint &startPoint, const QPoint &endPoint, Visitor visitor)
    {
        QVector<QPoint> points;
        int startY, endY;
        if (startPoint.y() < endPoint.y()) {
            startY = startPoint.y();
            endY = endPoint.y();
        } else {
            startY = endPoint.y();
            endY = startPoint.y();
        }
        for (int y = startY; y <= endY; ++y) {
            visitor(QPoint(startPoint.x(), y));
        }
    }

    template <typename Visitor>
    void rasterizeLineDDA(const QPoint &startPoint, const QPoint &endPoint, Visitor visitor)
    {
        QVector<QPoint> points;

        if (startPoint == endPoint) {
            visitor(startPoint);
            return;
        }
        if (startPoint.y() == endPoint.y()) {
            rasterizeHLine(startPoint, endPoint, visitor);
            return;
        }
        if (startPoint.x() == endPoint.x()) {
            rasterizeVLine(startPoint, endPoint, visitor);
            return;
        }

        const QPoint delta = endPoint - startPoint;
        QPoint currentPosition = startPoint;
        QPointF currentPositionF = startPoint;
        qreal m = static_cast<qreal>(delta.y()) / static_cast<qreal>(delta.x());
        int increment;

        if (std::abs(m) > 1.0) {
            if (delta.y() > 0) {
                m = 1.0 / m;
                increment = 1;
            } else {
                m = -1.0 / m;
                increment = -1;
            }
            while (currentPosition.y() != endPoint.y()) {
                currentPositionF.setX(currentPositionF.x() + m);
                currentPosition = QPoint(static_cast<int>(qRound(currentPositionF.x())),
                                        currentPosition.y() + increment);
                visitor(currentPosition);
            }
        } else {
            if (delta.x() > 0) {
                increment = 1;
            } else {
                increment = -1;
                m = -m;
            }
            while (currentPosition.x() != endPoint.x()) {
                currentPositionF.setY(currentPositionF.y() + m);
                currentPosition = QPoint(currentPosition.x() + increment,
                                        static_cast<int>(qRound(currentPositionF.y())));
                visitor(currentPosition);
            }
        }
    }

    template <typename Visitor>
    void rasterizePolylineDDA(const QVector<QPoint> &polylinePoints, Visitor visitor)
    {
        if (polylinePoints.size() == 0) {
            return;
        }
        if (polylinePoints.size() == 1) {
            visitor(polylinePoints.first());
            return;
        }

        // copy all points from the first segment
        rasterizeLineDDA(polylinePoints[0], polylinePoints[1], visitor);
        // for the rest of the segments, copy all points except the first one
        // (it is the same as the last point in the previous segment)
        for (int i = 1; i < polylinePoints.size() - 1; ++i) {
            int pointIndex = 0;
            rasterizeLineDDA(
                polylinePoints[i], polylinePoints[i + 1],
                [&pointIndex, &visitor](const QPoint &point) -> void
                {
                    if (pointIndex > 0) {
                        visitor(point);
                    }
                    ++pointIndex;
                }
            );
        }
    }

    template <typename Visitor>
    void rasterizePolygonDDA(const QVector<QPoint> &polygonPoints, Visitor visitor)
    {
        // this is a line
        if (polygonPoints.size() < 3) {
            rasterizeLineDDA(polygonPoints.first(), polygonPoints.last(), visitor);
            return;
        }
        // rasterize all segments except the last one
        QPoint lastSegmentStart;
        if (polygonPoints.first() == polygonPoints.last()) {
            rasterizePolylineDDA(polygonPoints.mid(0, polygonPoints.size() - 1), visitor);
            lastSegmentStart = polygonPoints[polygonPoints.size() - 2];
        } else {
            rasterizePolylineDDA(polygonPoints, visitor);
            lastSegmentStart = polygonPoints[polygonPoints.size() - 1];
        }
        // close the polygon
        {
            QVector<QPoint> points;
            auto addPoint = [&points](const QPoint &point) -> void { points.append(point); };
            rasterizeLineDDA(lastSegmentStart, polygonPoints.first(), addPoint);
            for (int i = 1; i < points.size() - 1; ++i) {
                visitor(points[i]);
            }
        }
    }

    // Convenience functions
    QVector<QPoint> KRITAIMAGE_EXPORT rasterizeHLine(const QPoint &startPoint, const QPoint &endPoint);
    QVector<QPoint> KRITAIMAGE_EXPORT rasterizeVLine(const QPoint &startPoint, const QPoint &endPoint);
    QVector<QPoint> KRITAIMAGE_EXPORT rasterizeLineDDA(const QPoint &startPoint, const QPoint &endPoint);
    QVector<QPoint> KRITAIMAGE_EXPORT rasterizePolylineDDA(const QVector<QPoint> &polylinePoints);
    QVector<QPoint> KRITAIMAGE_EXPORT rasterizePolygonDDA(const QVector<QPoint> &polygonPoints);
}

#endif /* __KRITA_UTILS_H */
