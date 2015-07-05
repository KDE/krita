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

#include <limits>
#include <QMetaType>
#include <boost/operators.hpp>


class KisTimeRange : public boost::equality_comparable<KisTimeRange>
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

    bool operator==(const KisTimeRange &rhs) const {
        return rhs.m_start == m_start && rhs.m_end == m_end;
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

    inline bool isInfinity() const {
        return m_end == std::numeric_limits<int>::min() &&
            m_start == std::numeric_limits<int>::max();
    }

    inline bool isValid() const {
        return m_end >= m_start;
    }

    static inline KisTimeRange fromTime(int start, int end) {
        return KisTimeRange(start, end, true);
    }

    static inline KisTimeRange infinity() {
        return KisTimeRange(std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), true);
    }

private:
    inline KisTimeRange(int start, int end, bool)
        : m_start(start),
          m_end(end)
    {
    }

private:
    int m_start;
    int m_end;
};

Q_DECLARE_METATYPE(KisTimeRange);


#endif /* __KIS_TIME_RANGE_H */
