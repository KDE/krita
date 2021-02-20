/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
