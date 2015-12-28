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

#include <QVector>
#include "kritaimage_export.h"
#include "kis_types.h"

namespace KritaUtils
{
    QSize KRITAIMAGE_EXPORT optimalPatchSize();

    QVector<QRect> KRITAIMAGE_EXPORT splitRectIntoPatches(const QRect &rc, const QSize &patchSize);

    QRegion KRITAIMAGE_EXPORT splitTriangles(const QPointF &center,
                                             const QVector<QPointF> &points);
    QRegion KRITAIMAGE_EXPORT splitPath(const QPainterPath &path);

    void KRITAIMAGE_EXPORT initAntsPen(QPen *antsPen, QPen *outlinePen,
                                       int antLength = 4, int antSpace = 4);

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

    template <class T>
        bool compareListsUnordered(const QList<T> &a, const QList<T> &b) {
        if (a.size() != b.size()) return false;

        Q_FOREACH(const T &t, a) {
            if (!b.contains(t)) return false;
        }

        return true;
    }
}

#endif /* __KRITA_UTILS_H */
