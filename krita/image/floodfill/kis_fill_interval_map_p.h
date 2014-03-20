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

#ifndef __KIS_FILL_INTERVAL_MAP_P_H
#define __KIS_FILL_INTERVAL_MAP_P_H

struct KRITAIMAGE_EXPORT KisFillIntervalMap::Private {
    typedef QMap<int, KisFillInterval> LineIntervalMap;
    typedef QHash<int, LineIntervalMap> GlobalMap;

    struct IteratorRange {
        IteratorRange()
        {
        }

        IteratorRange(LineIntervalMap::iterator _beginIt,
                      LineIntervalMap::iterator _endIt,
                      GlobalMap::iterator _rowMapIt)
            : beginIt(_beginIt),
              endIt(_endIt),
              rowMapIt(_rowMapIt)
        {
        }

        LineIntervalMap::iterator beginIt;
        LineIntervalMap::iterator endIt;
        GlobalMap::iterator rowMapIt;
    };

    GlobalMap map;

    IteratorRange findFirstIntersectingInterval(const KisFillInterval &interval);
};


#endif /* __KIS_FILL_INTERVAL_MAP_P_H */
