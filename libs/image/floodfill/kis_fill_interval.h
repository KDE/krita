/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FILL_INTERVAL_H
#define __KIS_FILL_INTERVAL_H


#include <boost/operators.hpp>
#include "kis_global.h"


class KisFillInterval : private boost::equality_comparable1<KisFillInterval>
{
public:
    KisFillInterval()
        : start(0),
          end(-1),
          row(-1)
    {
    }

    KisFillInterval(int _start, int _end, int _row)
        : start(_start),
          end(_end),
          row(_row)
    {
    }

    inline void invalidate() {
        end = start - 1;
    }

    inline bool operator==(const KisFillInterval &rhs) const {
        return start == rhs.start && end == rhs.end && row == rhs.row;
    }

    inline int width() const {
        return end - start + 1;
    }

    inline bool isValid() const {
        return end >= start;
    }

    int start;
    int end;
    int row;
};

#include <kis_debug.h>
inline QDebug operator<<(QDebug dbg, const KisFillInterval& i)
{
#ifndef NODEBUG
    dbg.nospace() << "KisFillInterval(" << i.start << ".." << i.end << "; " << i.row << ")";
#endif
    return dbg;
}

#endif /* __KIS_FILL_INTERVAL_H */
