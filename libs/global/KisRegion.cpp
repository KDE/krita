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
#include "KisRegion.h"

#include <QRegion>

namespace detail {

struct HorizontalMergePolicy
{
    static int col(const QRect &rc) {
        return rc.x();
    }
    static int nextCol(const QRect &rc) {
        return rc.x() + rc.width();
    }
    static int rowHeight(const QRect &rc) {
        return rc.height();
    }
    static bool rowIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.y() < rhs.y();
    }
    static bool elementIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.y() < rhs.y() || (lhs.y() == rhs.y() && lhs.x() < rhs.x());
    }
};

struct VerticalMergePolicy
{
    static int col(const QRect &rc) {
        return rc.y();
    }
    static int nextCol(const QRect &rc) {
        return rc.y() + rc.height();
    }
    static int rowHeight(const QRect &rc) {
        return rc.width();
    }
    static bool rowIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.x() < rhs.x();
    }
    static bool elementIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.x() < rhs.x() || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
    }
};

template <typename MergePolicy>
QVector<QRect>::iterator mergeRects(QVector<QRect>::iterator beginIt,
                                    QVector<QRect>::iterator endIt,
                                    MergePolicy policy)
{
    if (beginIt == endIt) return endIt;

    std::sort(beginIt, endIt, MergePolicy::elementIsLess);

    auto resultIt = beginIt;
    auto it = std::next(beginIt);

    while (it != endIt) {
        auto rowEnd = std::upper_bound(it, endIt, *it, MergePolicy::rowIsLess);
        for (auto rowIt = it; rowIt != rowEnd; ++rowIt) {
            if (policy.rowHeight(*resultIt) == policy.rowHeight(*rowIt) &&
                    policy.nextCol(*resultIt) == policy.col(*rowIt)) {
                *resultIt |= *rowIt;
            } else {
                resultIt++;
                *resultIt = *rowIt;
            }
        }

        it = rowEnd;
    }

    return std::next(resultIt);
}
}

QVector<QRect>::iterator KisRegion::mergeSparseRects(QVector<QRect>::iterator beginIt, QVector<QRect>::iterator endIt)
{
    endIt = detail::mergeRects(beginIt, endIt, detail::HorizontalMergePolicy());
    endIt = detail::mergeRects(beginIt, endIt, detail::VerticalMergePolicy());
    return endIt;
}

KisRegion::KisRegion(const QRect &rect)
{
    m_rects << rect;
}

KisRegion::KisRegion(std::initializer_list<QRect> rects)
    : m_rects(rects)
{
}

KisRegion::KisRegion(const QVector<QRect> &rects)
    : m_rects(rects)
{
    mergeAllRects();
}

KisRegion::KisRegion(QVector<QRect> &&rects)
    : m_rects(rects)
{
    mergeAllRects();
}

KisRegion &KisRegion::operator=(const KisRegion &rhs)
{
    m_rects = rhs.m_rects;
    return *this;
}

KisRegion &KisRegion::operator&=(const QRect &rect)
{
    for (auto it = m_rects.begin(); it != m_rects.end(); /* noop */) {
        *it &= rect;
        if (it->isEmpty()) {
            it = m_rects.erase(it);
        } else {
            ++it;
        }
    }
    mergeAllRects();
    return *this;
}

QRect KisRegion::boundingRect() const
{
    return std::accumulate(m_rects.constBegin(), m_rects.constEnd(), QRect(), std::bit_or<QRect>());
}

QVector<QRect> KisRegion::rects() const
{
    return m_rects;
}

int KisRegion::rectCount() const
{
    return m_rects.size();
}

bool KisRegion::isEmpty() const
{
    return boundingRect().isEmpty();
}

QRegion KisRegion::toQRegion() const
{
    // TODO: ustilize QRegion::setRects to make creation of QRegion much
    //       faster. The only reason why we cannot use it "as is", our m_rects
    //       do not satisfy the second setRects()'s precondition: "All rectangles with
    //       a given top coordinate must have the same height". We can implement an
    //       simple algorithm for cropping m_rects, and it will be much faster than
    //       constructing QRegion iterationally.

    return std::accumulate(m_rects.constBegin(), m_rects.constEnd(), QRegion(), std::bit_or<QRegion>());
}

void KisRegion::translate(int dx, int dy)
{
    std::transform(m_rects.begin(), m_rects.end(),
                   m_rects.begin(),
                   [dx, dy] (const QRect &rc) { return rc.translated(dx, dy); });
}

KisRegion KisRegion::translated(int dx, int dy) const
{
    KisRegion region(*this);
    region.translate(dx, dy);
    return region;
}

KisRegion KisRegion::fromQRegion(const QRegion &region)
{
    KisRegion result;
    result.m_rects.clear();
    QRegion::const_iterator begin = region.begin();
    while (begin != region.end()) {
        result.m_rects << *begin;
        begin++;
    }
    return result;
}

void KisRegion::mergeAllRects()
{
    auto endIt = mergeSparseRects(m_rects.begin(), m_rects.end());
    m_rects.erase(endIt, m_rects.end());
}

bool operator==(const KisRegion &lhs, const KisRegion &rhs)
{
    return lhs.m_rects == rhs.m_rects;
}
