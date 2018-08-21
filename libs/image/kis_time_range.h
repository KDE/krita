/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TIME_RANGE_H
#define __KIS_TIME_RANGE_H

#include "kritaimage_export.h"

#include <algorithm>
#include <limits>
#include <QMetaType>
#include <boost/operators.hpp>
#include "kis_types.h"
#include <kis_dom_utils.h>

/**
 * Represents a finite, continuous span of time between two frames.
 * Start and end frames are both included in the span.
 */
class KRITAIMAGE_EXPORT KisTimeSpan : public boost::equality_comparable<KisTimeSpan> {
public:

    inline KisTimeSpan()
        : m_start(-1)
        , m_end(-2)
    {}

    inline KisTimeSpan(int start, int end)
        : m_start(start),
          m_end(end)
    {}

    inline bool isEmpty() const { return m_end < m_start; }
    inline int start() const { return m_start; }
    inline int end() const { return m_end; }

    inline bool isValid() const { return isEmpty(); }

    inline int duration() const {
        return !isEmpty() ? (m_end - m_start + 1) : 0;
    }

    inline bool contains(int time) const {
        return !isEmpty() ? (m_start <= time && time <= m_end) : false;
    }

    inline bool contains(const KisTimeSpan rhs) const {
        return rhs.isEmpty() || (m_start <= rhs.start() && rhs.end() <= m_end);
    }

    inline bool overlaps(const KisTimeSpan &other) const {
        return m_start <= other.m_end && other.m_start <= m_end;
    }

    inline KisTimeSpan truncateLeft(int newEnd) const {
        if (newEnd < m_start) return KisTimeSpan();
        if (m_end <= newEnd) return *this;
        return KisTimeSpan(m_start, newEnd);
    }

    inline KisTimeSpan truncateRight(int newStart) const {
        if (m_end < newStart) return KisTimeSpan();
        if (newStart <= m_start) return *this;
        return KisTimeSpan(newStart, m_end);
    }

    bool operator==(const KisTimeSpan &rhs) const {
        return rhs.m_start == m_start && rhs.m_end == m_end;
    }

    KisTimeSpan operator|(const KisTimeSpan &rhs) const {
        if (isEmpty()) return rhs;
        if (rhs.isEmpty()) return *this;

        int start = qMin(m_start, rhs.m_start);
        int end = qMax(m_end, rhs.m_end);

        return KisTimeSpan(start, end);
    }

    KisTimeSpan operator&(const KisTimeSpan &rhs) const {
        if (isEmpty() || rhs.isEmpty()) return KisTimeSpan();

        int start = qMax(m_start, rhs.m_start);
        int end = qMin(m_end, rhs.m_end);

        if (end < start) return KisTimeSpan();

        return KisTimeSpan(start, end);
    }

private:
    int m_start, m_end;
};

/**
 * Represents an arbitrary set of frames, possibly stretching to infinity.
 *
 */
class KRITAIMAGE_EXPORT KisFrameSet :
        public boost::equality_comparable<KisFrameSet>,
        public boost::andable<KisFrameSet>,
        public boost::orable<KisFrameSet>,
        public boost::subtractable<KisFrameSet>
{
public:
    static KisFrameSet between(int start, int end) { return KisFrameSet(KisTimeSpan(start, end)); }
    static KisFrameSet infiniteFrom(int start) { return KisFrameSet({}, start); }

    KisFrameSet() = default;

    inline explicit KisFrameSet(QVector<KisTimeSpan> spans, int firstFrameOfInfinity = -1)
            : m_spans(spans)
            , m_firstFrameOfInfinity(firstFrameOfInfinity)
    {
        // Normalize

        std::sort(m_spans.begin(), m_spans.end(),
                  [](const KisTimeSpan &a, const KisTimeSpan &b) { return a.start() < b.start(); }
        );

        mergeOverlappingSpans();
    }

    explicit KisFrameSet(const KisTimeSpan span)
        : m_spans({span})
    {}

    inline bool isInfinite() const { return m_firstFrameOfInfinity >= 0; }

    bool isEmpty() const {
        return m_spans.isEmpty() && !isInfinite();
    }

    int start() const {
        if (m_spans.isEmpty()) return m_firstFrameOfInfinity;
        return m_spans.first().start();
    }

    /**
     * List of the finite, continuous spans this set consists of.
     * Note: this list does not contain the tail of infinite sets. See firstFrameOfInfinity().
     */
    inline const QVector<KisTimeSpan> finiteSpans() const { return m_spans; }

    /**
     * If the set is infinite, the frame from which the infinite tail begins. Returns -1 if the set is finite.
     */
    inline int firstFrameOfInfinity() const {
        return m_firstFrameOfInfinity;
    }

    inline bool contains(int frame) const {
        if (0 <= m_firstFrameOfInfinity && m_firstFrameOfInfinity <= frame) return true;

        Q_FOREACH(const KisTimeSpan &span, m_spans) {
                if (span.contains(frame)) return true;
            }

        return false;
    }

    bool operator==(const KisFrameSet &rhs) const {
        return rhs.m_firstFrameOfInfinity == m_firstFrameOfInfinity && rhs.m_spans == m_spans;
    }

    KisFrameSet& operator|=(const KisFrameSet &rhs);
    KisFrameSet& operator&=(const KisFrameSet &rhs);
    KisFrameSet& operator-=(const KisFrameSet &rhs);

private:
    inline void mergeOverlappingSpans()
    {
        int dst = 0;
        for (int src = 1; src < m_spans.length(); src++) {
            if (isInfinite() && m_firstFrameOfInfinity <= m_spans[src].end()) {
                m_firstFrameOfInfinity = qMin(m_spans[src].start(), m_firstFrameOfInfinity);
                break;
            }

            if (m_spans[src].overlaps(m_spans[dst])) {
                m_spans[dst] = m_spans[dst] | m_spans[src];
            } else {
                dst++;
                if (dst != src) {
                    m_spans[dst] = m_spans[src];
                }
            }
        }

        if (dst < m_spans.length() - 1) {
            m_spans.resize(dst - 1);
        }
    }

    QVector<KisTimeSpan> m_spans;
    int m_firstFrameOfInfinity = -1;
};

class KRITAIMAGE_EXPORT KisTimeRange : public boost::equality_comparable<KisTimeRange>
{
public:
    inline KisTimeRange()
        : m_start(0),
          m_end(-1)
    {
    }

    inline KisTimeRange(int start, int duration)
        : m_start(start),
          m_end(start + duration - 1)
    {
    }

    inline KisTimeRange(int start, int end, bool)
        : m_start(start),
          m_end(end)
    {
    }

    bool operator==(const KisTimeRange &rhs) const {
        return rhs.m_start == m_start && rhs.m_end == m_end;
    }

    KisTimeRange& operator|=(const KisTimeRange &rhs) {
        if (!isValid()) {
            m_start = rhs.start();
        } else if (rhs.isValid()) {
            m_start = std::min(m_start, rhs.start());
        }

        if (rhs.isInfinite() || isInfinite()) {
            m_end = std::numeric_limits<int>::min();
        } else if (!isValid()) {
            m_end = rhs.m_end;
        } else {
            m_end = std::max(m_end, rhs.m_end);
        }

        return *this;
    }

    KisTimeRange& operator&=(const KisTimeRange &rhs) {
        if (!isValid()) {
            return *this;
        } else if (!rhs.isValid()) {
            m_start = rhs.start();
            m_end = rhs.m_end;
            return *this;
        } else {
            m_start = std::max(m_start, rhs.start());
        }

        if (isInfinite()) {
            m_end = rhs.m_end;
        } else if (!rhs.isInfinite()) {
            m_end = std::min(m_end, rhs.m_end);
        }

        return *this;
    }

    inline int start() const {
        return m_start;
    }

    inline int end() const {
        return m_end;
    }

    inline int duration() const {
        return m_end >= m_start ? m_end - m_start + 1 : 0;
    }

    inline bool isInfinite() const {
        return m_end == std::numeric_limits<int>::min();
    }

    inline bool isValid() const {
        return (m_end >= m_start) || (m_end == std::numeric_limits<int>::min() && m_start >= 0);
    }

    inline bool contains(int time) const {
        if (m_end == std::numeric_limits<int>::min()) {
            return m_start <= time;
        }

        return m_start <= time && time <= m_end;
    }

    static inline KisTimeRange fromTime(int start, int end) {
        return KisTimeRange(start, end, true);
    }

    static inline KisTimeRange infinite(int start) {
        return KisTimeRange(start, std::numeric_limits<int>::min(), true);
    }

    static void calculateTimeRangeRecursive(const KisNode *node, int time, KisTimeRange &range, bool exclusive);

    static KisTimeRange calculateNodeIdenticalFrames(const KisNode *node, int time);
    static KisTimeRange calculateNodeAffectedFrames(const KisNode *node, int time);

private:
    int m_start;
    int m_end;
};

namespace KisDomUtils {
    void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const KisTimeRange &range);
    bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, KisTimeRange *range);

    void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const KisTimeSpan &range);
    bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, KisTimeSpan *range);
}



Q_DECLARE_METATYPE(KisTimeRange);
Q_DECLARE_METATYPE(KisTimeSpan);
Q_DECLARE_METATYPE(KisFrameSet);

KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisTimeRange &r);
KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisTimeSpan &r);
KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisFrameSet &r);


#endif /* __KIS_TIME_RANGE_H */
