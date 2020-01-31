/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
 * into a smalles set of bigger rectangles, the same thing that QRegion
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
     * @brief merge a set of rectanges into a smaller set of bigger rectangles
     *
     * The algorithm does two passes over the rectanges. First it tries to
     * merge all the rectanges horizontally, then vertically. The merge happens
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

public:
    KisRegion() = default;
    KisRegion(const KisRegion &rhs) = default;
    KisRegion(const QRect &rect);
    KisRegion(std::initializer_list<QRect> rects);

    /**
     * @brief creates a region from a set of non-intersecting rectanges
     * @param rects rectangles that should be merged. Rectangles must not intersect.
     */
    KisRegion(const QVector<QRect> &rects);
    KisRegion(QVector<QRect> &&rects);

    KisRegion& operator=(const KisRegion &rhs);
    friend bool operator==(const KisRegion &lhs, const KisRegion &rhs);

    KisRegion& operator&=(const QRect &rect);

    QRect boundingRect() const;
    QVector<QRect> rects() const;
    int rectCount() const;
    bool isEmpty() const;

    QRegion toQRegion() const;

    void translate(int dx, int dy);
    KisRegion translated(int x, int y) const;

    static KisRegion fromQRegion(const QRegion &region);

private:
    void mergeAllRects();

private:
    QVector<QRect> m_rects;
};

KRITAGLOBAL_EXPORT bool operator==(const KisRegion &lhs, const KisRegion &rhs);


#endif // KISREGION_H
