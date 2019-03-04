/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *
 *  adopted from here http://www.snippetcenter.org/en/a-fast-atan2-function-s1868.aspx
 */

#include "kis_fast_math.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <QtGlobal>
#include <QGlobalStatic>

// Algorithm from http://www.snippetcenter.org/en/a-fast-atan2-function-s1868.aspx
const qreal MAX_SECOND_DERIV_IN_RANGE = 0.6495;

/// precision
const qreal MAX_ERROR = 0.0001;

struct KisATanTable {

    KisATanTable() {
        qreal nf = ::sqrt(MAX_SECOND_DERIV_IN_RANGE / (8 * MAX_ERROR));
        NUM_ATAN_ENTRIES = int(nf) + 1;
        // Build table
        qreal y = 10.0;
        qreal x;
        ATanTable = new qreal[NUM_ATAN_ENTRIES + 1];
        ATanTable[0] = 0.0;
        for (quint32 i = 1; i <= NUM_ATAN_ENTRIES; i++) {
            x = (y / i) * NUM_ATAN_ENTRIES;
            ATanTable[i] = (qreal)::atan2(y, x);
        }

    }

    ~KisATanTable() {
        delete [] ATanTable;
    }

    quint32 NUM_ATAN_ENTRIES;
    qreal* ATanTable;
};

Q_GLOBAL_STATIC(KisATanTable, kisATanTable)

/// private functions

inline qreal interp(qreal r, qreal a, qreal b)
{
    return r*(b - a) + a;
}

inline qreal calcAngle(qreal x, qreal y)
{
    static qreal af = kisATanTable->NUM_ATAN_ENTRIES;
    static int ai = kisATanTable->NUM_ATAN_ENTRIES;
    static qreal* ATanTable = kisATanTable->ATanTable;
    qreal di = (y / x) * af;
    int i = (int)(di);
    if (i >= ai) return ::atan2(y, x);
    return interp(di - i, ATanTable[i], ATanTable[i+1]);
}

qreal KisFastMath::atan2(qreal y, qreal x)
{

    if (y == 0.0) { // the line is horizontal
        if (x >= 0.0) { // towards the right
            return(0.0);// the angle is 0
        }
        // toward the left
        return qreal(M_PI);
    } // we now know that y is not 0 check x
    if (x == 0.0) { // the line is vertical
        if (y > 0.0) {
            return M_PI_2;
        }
        return -M_PI_2;
    }
    // from here on we know that niether x nor y is 0
    if (x > 0.0) {
        // we are in quadrant 1 or 4
        if (y > 0.0) {
            // we are in quadrant 1
            // now figure out which side of the 45 degree line
            if (x > y) {
                return(calcAngle(x, y));
            }
            return(M_PI_2 - calcAngle(y, x));
        }
        // we are in quadrant 4
        y = -y;
        // now figure out which side of the 45 degree line
        if (x > y) {
            return(-calcAngle(x, y));
        }
        return(-M_PI_2 + calcAngle(y, x));
    }
    // we are in quadrant 2 or 3
    x = -x;
    // flip x so we can use it as a positive
    if (y > 0.0) {
        // we are in quadrant 2
        // now figure out which side of the 45 degree line
        if (x > y) {
            return(M_PI - calcAngle(x, y));
        } return(M_PI_2 + calcAngle(y, x));
    }
    // we are in quadrant 3
    y = -y;
    // flip y so we can use it as a positive
    // now figure out which side of the 45 degree line
    if (x > y) {
        return(-M_PI + calcAngle(x, y));
    } return(-M_PI_2 - calcAngle(y, x));
}
