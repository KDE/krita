/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
