/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISREGION_H
#define KISREGION_H

#include "kritaglobal_export.h"
#include <QVector>
#include <QRect>
#include <boost/operators.hpp>

class QRegion;

/**
 * An more efficient (and more limited) replacement for QRegion.
 *
 * Its main purpose it to be able to merge a huge set of rectangles
 * into a smaller set of bigger rectangles, the same thing that QRegion
 * is supposed to do. The main difference (and limitation) is: all the
 * input rects must be non-intersecting. This requirement is perfectly
 * fine for Krita's tiles, which do never intersect.
 */
class KRITAGLOBAL_EXPORT KisRegion :
        public boost::equality_comparable<KisRegion>,
        public boost::andable<KisRegion, QRect>
{
public:
    /**
     * @brief merge a set of rectangles into a smaller set of bigger rectangles
     *
     * The algorithm does two passes over the rectangles. First it tries to
     * merge all the rectangles horizontally, then vertically. The merge happens
     * in-place, that is, all the merged elements will be moved to the front
     * of the original range.
     *
     * The final range is defined by [beginIt, retvalIt)
     *
     * @param beginIt iterator to the beginning of the source range
     * @param endIt iterator to the end of the source range
     * @return iteration pointing past the last element of the merged range
     */
    static QVector<QRect>::iterator mergeSparseRects(QVector<QRect>::iterator beginIt, QVector<QRect>::iterator endIt);


    /**
     * Simplifies \p rects in a way that they don't overlap anymore. The actual
     * resulting area may be larger than original \p rects, but not more than
     * \p gridSize in any dimension.
     */
    static void approximateOverlappingRects(QVector<QRect> &rects, int gridSize);

    static void makeGridLikeRectsUnique(QVector<QRect> &rects);

public:
    KisRegion() = default;
    KisRegion(const KisRegion &rhs) = default;
    KisRegion(const QRect &rect);
    KisRegion(std::initializer_list<QRect> rects);

    /**
     * @brief creates a region from a set of non-intersecting rectangles
     * @param rects rectangles that should be merged. Rectangles must not intersect.
     */
    KisRegion(const QVector<QRect> &rects);
    KisRegion(QVector<QRect> &&rects);

    KisRegion& operator=(const KisRegion &rhs);
    friend KRITAGLOBAL_EXPORT bool operator==(const KisRegion &lhs, const KisRegion &rhs);

    KisRegion& operator&=(const QRect &rect);

    QRect boundingRect() const;
    QVector<QRect> rects() const;
    int rectCount() const;
    bool isEmpty() const;

    QRegion toQRegion() const;

    void translate(int dx, int dy);
    KisRegion translated(int x, int y) const;

    static KisRegion fromQRegion(const QRegion &region);

    /**
     * Approximates a KisRegion from \p rects, which may overlap. The resulting
     * KisRegion may be larger than the original set of rects, but it is guaranteed
     * to cover it completely.
     */
    static KisRegion fromOverlappingRects(const QVector<QRect> &rects, int gridSize);

private:
    void mergeAllRects();

private:
    QVector<QRect> m_rects;
};

KRITAGLOBAL_EXPORT bool operator==(const KisRegion &lhs, const KisRegion &rhs);


#endif // KISREGION_H
