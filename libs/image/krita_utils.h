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

#include <QVector>
#include "kritaimage_export.h"
#include "kis_types.h"
#include "krita_container_utils.h"
#include <functional>


namespace KritaUtils
{
    QSize KRITAIMAGE_EXPORT optimalPatchSize();

    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatches(const QRect &rc, const QSize &patchSize);
    QVector<QRect> KRITAIMAGE_EXPORT splitRegionIntoPatches(const QRegion &region, const QSize &patchSize);

    QRegion KRITAIMAGE_EXPORT splitTriangles(const QPointF &center,
                                             const QVector<QPointF> &points);
    QRegion KRITAIMAGE_EXPORT splitPath(const QPainterPath &path);

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
    QColor KRITAIMAGE_EXPORT blendColors(const QColor &c1, const QColor &c2, qreal r1);

    /**
     * \return an approximate difference between \p c1 and \p c2
     *         in a (nonlinear) range [0, 3]
     *
     * The colors are compared using the formula:
     *     difference = sqrt(2 * diff_R^2 + 4 * diff_G^2 + 3 * diff_B^2)
     */
    qreal KRITAIMAGE_EXPORT colorDifference(const QColor &c1, const QColor &c2);

    /**
     * Make the color \p color differ from \p baseColor for at least \p threshold value
     */
    void KRITAIMAGE_EXPORT dragColor(QColor *color, const QColor &baseColor, qreal threshold);

    void KRITAIMAGE_EXPORT applyToAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<void(quint8)> func);
    void KRITAIMAGE_EXPORT filterAlpha8Device(KisPaintDeviceSP dev, const QRect &rc, std::function<quint8(quint8)> func);

    qreal KRITAIMAGE_EXPORT estimatePortionOfTransparentPixels(KisPaintDeviceSP dev, const QRect &rect, qreal samplePortion);

    void KRITAIMAGE_EXPORT mirrorDab(Qt::Orientation dir, const QPoint &center, KisRenderedDab *dab);
    void KRITAIMAGE_EXPORT mirrorRect(Qt::Orientation dir, const QPoint &center, QRect *rc);
    void KRITAIMAGE_EXPORT mirrorPoint(Qt::Orientation dir, const QPoint &center, QPointF *pt);
}

#endif /* __KRITA_UTILS_H */
