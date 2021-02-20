/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

class KRITAIMAGE_EXPORT KisTimeSpan : public boost::equality_comparable<KisTimeSpan>
{
private:
    inline KisTimeSpan(int start, int end)
        : m_start(start),
          m_end(end)
    {
    }

public:
    inline KisTimeSpan()
        : m_start(0),
          m_end(-1)
    {
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

    inline void include(int time) {
        m_start = qMin(time, m_start);
        m_end = qMax(time, m_end);
    }

    inline bool overlaps(const KisTimeSpan& other) const {
        // If either are "invalid", we should probably return false.
        if (!isValid() || !other.isValid()) {
            return false;
        }

        // Handle infinite cases...
        if (other.isInfinite()) {
            return (other.contains(start()) || other.contains(end()));
        } else if (isInfinite()) {
            return (contains(other.start()) || contains(other.end()));
        }

        const int selfMin = qMin(start(), end());
        const int selfMax = qMax(start(), end());
        const int otherMin = qMin(other.start(), other.end());
        const int otherMax = qMax(other.start(), other.end());
        return (selfMax >= otherMin) && (selfMin <= otherMax );
    }

    static inline KisTimeSpan fromTimeToTime(int start, int end) {
        return KisTimeSpan(start, end);
    }

    static inline KisTimeSpan fromTimeWithDuration(int start, int duration) {
        return KisTimeSpan( start, start + duration - 1);
    }

    static inline KisTimeSpan infinite(int start) {
        return KisTimeSpan(start, std::numeric_limits<int>::min());
    }

    static KisTimeSpan calculateIdenticalFramesRecursive(const KisNode *node, int time);
    static KisTimeSpan calculateAffectedFramesRecursive(const KisNode *node, int time);

    static KisTimeSpan calculateNodeIdenticalFrames(const KisNode *node, int time);
    static KisTimeSpan calculateNodeAffectedFrames(const KisNode *node, int time);

    bool operator==(const KisTimeSpan &rhs) const {
        return rhs.m_start == m_start && rhs.m_end == m_end;
    }

    KisTimeSpan operator|(const KisTimeSpan &rhs) const {
        KisTimeSpan result = *this;

        if (!result.isValid()) {
            result.m_start = rhs.start();
        } else if (rhs.isValid()) {
            result.m_start = std::min(result.m_start, rhs.start());
        }

        if (rhs.isInfinite() || result.isInfinite()) {
            result.m_end = std::numeric_limits<int>::min();
        } else if (!isValid()) {
            result.m_end = rhs.m_end;
        } else {
            result.m_end = std::max(m_end, rhs.m_end);
        }

        return result;
    }

    const KisTimeSpan& operator|=(const KisTimeSpan &rhs) {
        KisTimeSpan result = (*this | rhs);
        this->m_start = result.m_start;
        this->m_end = result.m_end;
        return *this;
    }

    KisTimeSpan operator&(const KisTimeSpan &rhs) const {
         KisTimeSpan result = *this;

        if (!isValid()) {
            return result;
        } else if (!rhs.isValid()) {
            result.m_start = rhs.start();
            result.m_end = rhs.m_end;
            return result;
        } else {
            result.m_start = std::max(result.m_start, rhs.start());
        }

        if (isInfinite()) {
            result.m_end = rhs.m_end;
        } else if (!rhs.isInfinite()) {
            result.m_end = std::min(result.m_end, rhs.m_end);
        }

        return result;
    }

    const KisTimeSpan& operator&=(const KisTimeSpan &rhs) {
        KisTimeSpan result = (*this & rhs);
        this->m_start = result.m_start;
        this->m_end = result.m_end;
        return *this;
    }

private:
    int m_start;
    int m_end;
};

namespace KisDomUtils {
    void KRITAIMAGE_EXPORT saveValue(QDomElement *parent, const QString &tag, const KisTimeSpan &range);
    bool KRITAIMAGE_EXPORT loadValue(const QDomElement &parent, const QString &tag, KisTimeSpan *range);
}



Q_DECLARE_METATYPE(KisTimeSpan)

KRITAIMAGE_EXPORT QDebug operator<<(QDebug dbg, const KisTimeSpan &r);


#endif /* __KIS_TIME_RANGE_H */
