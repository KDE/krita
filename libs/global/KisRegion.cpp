/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRegion.h"

#include <QRegion>
#include "kis_debug.h"

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

struct VerticalSplitPolicy
{
    static int rowStart(const QRect &rc) {
        return rc.y();
    }
    static int rowEnd(const QRect &rc) {
        return rc.bottom();
    }
    static int rowHeight(const QRect &rc) {
        return rc.height();
    }
    static void setRowEnd(QRect &rc, int rowEnd) {
        return rc.setBottom(rowEnd);
    }
    static bool rowIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.y() < rhs.y();
    }
    static QRect splitRectHi(const QRect &rc, int rowEnd) {
        return QRect(rc.x(), rc.y(),
                     rc.width(), rowEnd - rc.y() + 1);
    }
    static QRect splitRectLo(const QRect &rc, int rowEnd) {
        return QRect(rc.x(), rowEnd + 1,
                     rc.width(), rc.height() - (rowEnd - rc.y() + 1));
    }
};

struct HorizontalSplitPolicy
{
    static int rowStart(const QRect &rc) {
        return rc.x();
    }
    static int rowEnd(const QRect &rc) {
        return rc.right();
    }
    static int rowHeight(const QRect &rc) {
        return rc.width();
    }
    static void setRowEnd(QRect &rc, int rowEnd) {
        return rc.setRight(rowEnd);
    }
    static bool rowIsLess(const QRect &lhs, const QRect &rhs) {
        return lhs.x() < rhs.x();
    }
    static QRect splitRectHi(const QRect &rc, int rowEnd) {
        return QRect(rc.x(), rc.y(),
                     rowEnd - rc.x() + 1, rc.height());
    }
    static QRect splitRectLo(const QRect &rc, int rowEnd) {
        return QRect(rowEnd + 1, rc.y(),
                     rc.width() - (rowEnd - rc.x() + 1), rc.height());
    }
};


struct VoidNoOp {
    void operator()() const { };
    template<typename P1, typename... Params>
    void operator()(P1 p1, Params... parameters) {
        Q_UNUSED(p1);
        operator()(parameters...);
    }
};

struct MergeRectsOp
{
    MergeRectsOp(QVector<QRect> &source, QVector<QRect> &destination)
        : m_source(source),
          m_destination(destination)
    {
    }

    void operator()() {
        m_destination.append(std::accumulate(m_source.begin(), m_source.end(),
                                             QRect(), std::bit_or<QRect>()));
        m_source.clear();
    }

private:
    QVector<QRect> &m_source;
    QVector<QRect> &m_destination;
};

template <typename Policy, typename RowMergeOp, typename OutIt>
void splitRects(QVector<QRect>::iterator beginIt, QVector<QRect>::iterator endIt,
                OutIt resultIt,
                QVector<QRect> tempBuf[2],
                int gridSize,
                 RowMergeOp rowMergeOp)
{
    if (beginIt == endIt) return;

    QVector<QRect> &nextRowExtra = tempBuf[0];
    QVector<QRect> &nextRowExtraTmp = tempBuf[1];

    std::sort(beginIt, endIt, Policy::rowIsLess);
    int rowStart = Policy::rowStart(*beginIt);
    int rowEnd = rowStart + gridSize - 1;

    auto it = beginIt;
    while (1) {
        bool switchToNextRow = false;

        if (it == endIt) {
            if (nextRowExtra.isEmpty()) {
                rowMergeOp();
                break;
            } else {
                switchToNextRow = true;
            }
        } else if (Policy::rowStart(*it) > rowEnd) {
            switchToNextRow = true;
        }

        if (switchToNextRow) {
            rowMergeOp();

            if (!nextRowExtra.isEmpty()) {
                rowStart = Policy::rowStart(nextRowExtra.first());
                rowEnd = rowStart + gridSize - 1;

                for (auto nextIt = nextRowExtra.begin(); nextIt != nextRowExtra.end(); ++nextIt) {
                    if (Policy::rowEnd(*nextIt) > rowEnd) {
                        nextRowExtraTmp.append(Policy::splitRectLo(*nextIt, rowEnd));
                        *resultIt++ = Policy::splitRectHi(*nextIt, rowEnd);
                    } else {
                        *resultIt++ = *nextIt;
                    }
                }
                nextRowExtra.clear();
                std::swap(nextRowExtra, nextRowExtraTmp);

                continue;
            } else {
                rowStart = Policy::rowStart(*it);
                rowEnd = rowStart + gridSize - 1;
            }
        }

        if (Policy::rowEnd(*it) > rowEnd) {
            nextRowExtra.append(Policy::splitRectLo(*it, rowEnd));
            *resultIt++ = Policy::splitRectHi(*it, rowEnd);
        } else {
            *resultIt++ = *it;
        }

        ++it;
    }
}

}

QVector<QRect>::iterator KisRegion::mergeSparseRects(QVector<QRect>::iterator beginIt, QVector<QRect>::iterator endIt)
{
    endIt = detail::mergeRects(beginIt, endIt, detail::HorizontalMergePolicy());
    endIt = detail::mergeRects(beginIt, endIt, detail::VerticalMergePolicy());
    return endIt;
}

void KisRegion::approximateOverlappingRects(QVector<QRect> &rects, int gridSize)
{
    using namespace detail;

    if (rects.isEmpty()) return;

    QVector<QRect> rowsBuf;
    QVector<QRect> intermediate;
    QVector<QRect> tempBuf[2];

    splitRects<VerticalSplitPolicy>(rects.begin(), rects.end(),
                                    std::back_inserter(rowsBuf),
                                    tempBuf, gridSize, VoidNoOp());

    rects.clear();
    KIS_SAFE_ASSERT_RECOVER_NOOP(tempBuf[0].isEmpty());
    KIS_SAFE_ASSERT_RECOVER_NOOP(tempBuf[1].isEmpty());

    auto rowBegin = rowsBuf.begin();
    while (rowBegin != rowsBuf.end()) {
        auto rowEnd = std::upper_bound(rowBegin, rowsBuf.end(),
                                       QRect(rowBegin->x(),
                                             rowBegin->y() + gridSize - 1,
                                             1,1),
                                       VerticalSplitPolicy::rowIsLess);

        splitRects<HorizontalSplitPolicy>(rowBegin, rowEnd,
                                          std::back_inserter(intermediate),
                                          tempBuf, gridSize,
                                          MergeRectsOp(intermediate, rects));
        rowBegin = rowEnd;

        KIS_SAFE_ASSERT_RECOVER_NOOP(intermediate.isEmpty());
        KIS_SAFE_ASSERT_RECOVER_NOOP(tempBuf[0].isEmpty());
        KIS_SAFE_ASSERT_RECOVER_NOOP(tempBuf[1].isEmpty());
    }
}

void KisRegion::makeGridLikeRectsUnique(QVector<QRect> &rects)
{
    std::sort(rects.begin(), rects.end(), detail::HorizontalMergePolicy::elementIsLess);
    auto it = std::unique(rects.begin(), rects.end());
    rects.erase(it, rects.end());
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

KisRegion KisRegion::fromOverlappingRects(const QVector<QRect> &rects, int gridSize)
{
    QVector<QRect> tmp = rects;
    approximateOverlappingRects(tmp, gridSize);
    return KisRegion(tmp);
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
