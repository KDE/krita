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

class KRITAIMAGE_EXPORT KisTimeSpan : public boost::equality_comparable<KisTimeSpan>
{
public:
    inline KisTimeSpan()
        : m_start(0),
          m_end(-1)
    {
    }

    inline KisTimeSpan(int start, int duration)
        : m_start(start),
          m_end(start + duration - 1)
    {
    }

    inline KisTimeSpan(int start, int end, bool)
        : m_start(start),
          m_end(end)
    {
    }

    bool operator==(const KisTimeSpan &rhs) const {
        return rhs.m_start == m_start && rhs.m_end == m_end;
    }

    KisTimeSpan& operator|=(const KisTimeSpan &rhs) {
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

    KisTimeSpan& operator&=(const KisTimeSpan &rhs) {
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

    static inline KisTimeSpan fromTime(int start, int end) {
        return KisTimeSpan(start, end, true);
    }

    static inline KisTimeSpan infinite(int start) {
        return KisTimeSpan(start, std::numeric_limits<int>::min(), true);
    }

    static KisTimeSpan calculateIdenticalFramesRecursive(const KisNode *node, int time);
    static KisTimeSpan calculateAffectedFramesRecursive(const KisNode *node, int time);

    static KisTimeSpan calculateNodeIdenticalFrames(const KisNode *node, int time);
    static KisTimeSpan calculateNodeAffectedFrames(const KisNode *node, int time);

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
