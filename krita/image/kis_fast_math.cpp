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
#include <kglobal.h>

// Algorithm from http://www.snippetcenter.org/en/a-fast-atan2-function-s1868.aspx
const double PI = 4 * atan(1.0);
const double MAX_SECOND_DERIV_IN_RANGE = 0.6495;

/// precision
const double MAX_ERROR = 0.0001;

struct KisATanTable {

    KisATanTable() {
        double nf = ::sqrt(MAX_SECOND_DERIV_IN_RANGE / (8 * MAX_ERROR));
        NUM_ATAN_ENTRIES = int(nf) + 1;
        // Build table
        float y = 10.f;
        float x;
        ATanTable = new float[NUM_ATAN_ENTRIES + 1];
        ATanTable[0] = 0.0f;
        for (quint32 i = 1; i <= NUM_ATAN_ENTRIES; i++) {
            x = (y / i) * NUM_ATAN_ENTRIES;
            ATanTable[i] = (float)::atan2(y, x);
        }

    }

    ~KisATanTable() {
        delete [] ATanTable;
    }

    quint32 NUM_ATAN_ENTRIES;
    float* ATanTable;
};

K_GLOBAL_STATIC(KisATanTable, kisATanTable);

/// private functions

inline float interp(float r, float a, float b)
{
    return r*(b - a) + a;
}

inline double calcAngle(float x, float y)
{
    float di = (y / x) * kisATanTable->NUM_ATAN_ENTRIES;
    uint i = int(di);
    if (i >= kisATanTable->NUM_ATAN_ENTRIES) return ::atan2(y, x);
    return interp(di - i, kisATanTable->ATanTable[i], kisATanTable->ATanTable[i+1]);
}

float KisFastMath::atan2(float y, float x)
{

    if (y == 0.f) { // the line is horizontal
        if (x > 0.f) { // towards the right
            return(0.f);// the angle is 0
        }
        // toward the left
        return float(PI);
    } // we now know that y is not 0 check x
    if (x == 0.f) { // the line is vertical
        if (y > 0.f) {
            return(PI / 2.f);
        }
        return(-PI / 2.f);
    }
    // from here on we know that niether x nor y is 0
    if (x > 0.f) {
        // we are in quadrant 1 or 4
        if (y > 0.f) {
            // we are in quadrant 1
            // now figure out which side of the 45 degree line
            if (x > y) {
                return(calcAngle(x, y));
            }
            return((PI / 2.f) - calcAngle(y, x));
        }
        // we are in quadrant 4
        y = -y;
        // now figure out which side of the 45 degree line
        if (x > y) {
            return(-calcAngle(x, y));
        }
        return(-(PI / 2.f) + calcAngle(y, x));
    }
    // we are in quadrant 2 or 3
    x = -x;
    // flip x so we can use it as a positive
    if (y > 0) {
        // we are in quadrant 2
        // now figure out which side of the 45 degree line
        if (x > y) {
            return(PI - calcAngle(x, y));
        } return(PI / 2.f + calcAngle(y, x));
    }
    // we are in quadrant 3
    y = -y;
    // flip y so we can use it as a positve
    // now figure out which side of the 45 degree line
    if (x > y) {
        return(-PI + calcAngle(x, y));
    } return(-(PI / 2.f) - calcAngle(y, x));
}
