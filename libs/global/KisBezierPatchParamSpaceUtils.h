/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISBEZIERPATCHPARAMSPACEUTILS_H
#define KISBEZIERPATCHPARAMSPACEUTILS_H

#include <optional>
#include "kis_assert.h"
#include "kis_algebra_2d.h"

#include <QDebug>
#include <kis_debug.h>

namespace KisBezierUtils
{

/**
 * A simple class representing a floating-point range on \R.
 */
struct Range
{
    qreal start = 0.0;
    qreal end = 0.0;

    bool isEmpty() const {
        return qFuzzyCompare(start, end);
    }

    qreal length() const {
        return end - start;
    }

    qreal mid() const {
        return 0.5 * (end + start);
    }

    bool contains(qreal value) const {
        return value > start && value < end &&
            qFuzzyCompare(value, start) &&
            qFuzzyCompare(value, end);
    }

    /**
     * Narrow down the range by applying a relative range to it. Both
     * ends are moved using `lerp` operation over the source range.
     */
    Range squeezedRange(const Range &alphaRange) const {
        using KisAlgebra2D::lerp;
        return {lerp(start, end, alphaRange.start), lerp(start, end, alphaRange.end)};
    }

    void squeezeRange(const Range &alphaRange) {
        *this = squeezedRange(alphaRange);
    }

    /**
     * Returns the "forward distance" between `*this` and `rhs`. The forward
     * distance is undefined if the ranges overlap.
     *
     * - if the ranges overlap or touch each other with at least one
     *   point, then std::nullopt is returned
     *
     * - if `*this` is placed to the _left_ of `rhs`, then the distance
     *   between the ranges is returned (always positive)
     *
     * - if `*this` is placed to the _right_ of `rhs`, then the function
     *   returns the distance between the ranges taken with minus sign
     *   (always negative)
     *
     */
    std::optional<qreal> forwardDistanceTo(const Range &rhs) {
        if (rhs.start > end) {
            return rhs.start - end;
        } else if (start > rhs.end) {
            return -(start - rhs.end);
        } else {
            return std::nullopt;
        }
    }

    static Range fromRectX(const QRectF &rc) {
        return {rc.left(), rc.right()};
    }

    static Range fromRectY(const QRectF &rc) {
        return {rc.top(), rc.bottom()};
    }

    static QRectF makeRectF(const Range &xRange, const Range &yRange) {
        return {xRange.start, yRange.start, xRange.length(), yRange.length()};
    }
};

QDebug operator<<(QDebug dbg, const Range &r)
{
    dbg.nospace() << "Range(" << r.start << ", " << r.end << ")";

    return dbg.space();
}

/**
 * Approximate the rect in param-space of the bezier patch that fully covers
 * passed \p rect in the source-space. Due to the nature of the patch mapping,
 * the result cannot be calculated precisely (easily). The passed \p srcPresition
 * is only meant to be used for approximation algorithm termination. There is no
 * guarantee that the result covers \p rect with this precision.
 *
 * The function uses a simple binary search approach to estimate the borders
 * of the rectangle in question.
 *
 * To get better precision use iterational approach by combining X- and Y-axis
 * approximation using `searchParamRange` and `squeezeRange`.
 *
 * @param searchParamRange the range in param-space where the passed \p rect
 *                         is searched for. The rect must be guaranteed to be
 *                         present in this range, otherwise the behavior is
 *                         undeifned. When doing iterational precision search,
 *                         pass `externalRange` from the previous pass here.
 * @param searchSrcRange the range in source-space, where the rect is placed.
 *                       It is only used to check is the rect is placed exactly
 *                       at the border of that range.
 * @param rect rect in source-space that is searched for
 * @param func the functor that maps a param in the param-space into a range in
 *             the source space
 * @param srcPrecision src-space precision at which the search is stopped
 * @param squeezeRange when doing iterational search, pass the opposite-axis
 *                     range of the rectangle
 * @return a pair of {externalRange, internalRange} which "covers" and "is covered"
 *         by the rectangle correspondingly
 *
 */
template <typename Func>
std::pair<Range, Range> calcTightSrcRectRangeInParamSpace1D(const Range &searchParamRange,
                                                            const Range &searchSrcRange,
                                                            const Range &rect,
                                                            Func func,
                                                            qreal srcPrecision,
                                                            std::optional<Range> squeezeRange = std::nullopt)
{
    Range leftSideParamRange = searchParamRange;
    Range rightSideParamRange = searchParamRange;

    if (qFuzzyCompare(rect.start, searchSrcRange.start)) {
        leftSideParamRange = {searchParamRange.start, searchParamRange.start};
    } else {
        // search left side
        while (1) {
            KIS_SAFE_ASSERT_RECOVER_BREAK(!leftSideParamRange.isEmpty());

            qreal currentSplitParam = leftSideParamRange.mid();
            Range currentSplitSrcRange = func(currentSplitParam);
            if (squeezeRange) {
                currentSplitSrcRange.squeezeRange(*squeezeRange);
            }

            std::optional<qreal> forwardDistance = currentSplitSrcRange.forwardDistanceTo(rect);

            if (!forwardDistance || qFuzzyIsNull(*forwardDistance)) {
                leftSideParamRange.end = currentSplitParam;
                rightSideParamRange.start = std::max(currentSplitParam, rightSideParamRange.start);
            } else if (*forwardDistance > 0) {
                KIS_SAFE_ASSERT_RECOVER_NOOP(currentSplitParam >= leftSideParamRange.start);
                leftSideParamRange.start = currentSplitParam;
                rightSideParamRange.start = std::max(currentSplitParam, rightSideParamRange.start);
            } else if (*forwardDistance < 0) {
                KIS_SAFE_ASSERT_RECOVER_NOOP(currentSplitParam <= rightSideParamRange.end);
                rightSideParamRange.end = currentSplitParam;
                leftSideParamRange.end = currentSplitParam;
            }

            if (forwardDistance && std::abs(*forwardDistance) < srcPrecision) {
                break;
            }
        }
    }

    if (qFuzzyCompare(rect.end, searchSrcRange.end)) {
        rightSideParamRange = {searchParamRange.end, searchParamRange.end};
    } else {
        // search right side
        while (1) {
            KIS_SAFE_ASSERT_RECOVER_BREAK(!rightSideParamRange.isEmpty());

            qreal currentSplitParam = rightSideParamRange.mid();
            Range currentSplitSrcRange = func(currentSplitParam);

            if (squeezeRange) {
                currentSplitSrcRange.squeezeRange(*squeezeRange);
            }

            std::optional<qreal> forwardDistance = currentSplitSrcRange.forwardDistanceTo(rect);

            if (!forwardDistance || qFuzzyIsNull(*forwardDistance)) {
                rightSideParamRange.start = currentSplitParam;
            } else if (*forwardDistance > 0) {
                rightSideParamRange.start = currentSplitParam;
            } else if (*forwardDistance < 0) {
                rightSideParamRange.end = currentSplitParam;
            }

            if (forwardDistance && std::abs(*forwardDistance) < srcPrecision) {
                break;
            }
        }
    }

    return std::make_pair(Range{leftSideParamRange.start, rightSideParamRange.end},
                          Range{leftSideParamRange.end, rightSideParamRange.start});
}

} // namespace KisBezierUtils

#endif // KISBEZIERPATCHPARAMSPACEUTILS_H
